#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "linenoise.h"

#include "../include/linenoisecli/linenoisecli.hpp"

#define CONSOLE_COLOR_RED "\x1b[31m"
#define CONSOLE_COLOR_GREEN "\x1b[32m"
#define CONSOLE_COLOR_YELLOW "\x1b[33m"
#define CONSOLE_COLOR_BLUE "\x1b[34m"
#define CONSOLE_COLOR_MAGENTA "\x1b[35m"
#define CONSOLE_COLOR_CYAN "\x1b[36m"
#define CONSOLE_COLOR_RESET "\x1b[0m"

#define ERRNO(x) (-(x))

using namespace linenoisecli;

void completion(const char *buf, linenoiseCompletions *lc)
{
    std::vector<std::string> commands = cli::getInstance().getCommandList();
    for (const auto &command : commands)
    {
        if (strncmp(command.c_str(), buf, strlen(buf)) == 0)
        {
            linenoiseAddCompletion(lc, command.c_str());
        }
    }
}

char *hints(const char *buf, int *color, int *bold)
{
    std::vector<std::string> commands = cli::getInstance().getCommandList();
    for (const auto &command : commands)
    {
        size_t len = strlen(buf);
        if (strncmp(command.c_str(), buf, strlen(buf)) == 0)
        {
            *color = 35;
            *bold = 0;
            return strdup(command.c_str() + len);
        }
    }
    return NULL;
}

static std::vector<std::string> strsplit(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

static bool strcontains(const std::string &str, char c)
{
    return str.find(c) != std::string::npos;
}

static std::string strclear(const std::string &str, std::vector<std::string> word)
{
    std::string result = str;
    for (const auto &w : word)
    {
        size_t pos = result.find(w);
        if (pos != std::string::npos)
        {
            result.erase(pos, w.size());
        }
    }
    return result;
}

cli::cli()
{
    registerDefaultCommands();
}

cli::~cli()
{
}

cli &cli::getInstance()
{
    static cli instance;
    return instance;
}

int32_t cli::registerDefaultCommands()
{
    int32_t status = 0;

    std::string testCommand = "testCommand <mustArg1> <mustArg2> [<optionalArg>] [<optionalArg2>]";

    registerCommand(testCommand, [this](ArgumentMap &arguments) -> int32_t {
        int32_t status = 0;

        std::cout << "Test command" << std::endl;

        for (auto& argument : arguments)
        {
            std::cout << "Argument: " << argument.first << " = " << argument.second << std::endl;
        }

        return status; 
    });

    registerCommand("help", [this](ArgumentMap &arguments) -> int32_t {
        int32_t status = 0;
        std::vector<std::string> helpCommands;

        std::cout << "Available commands:" << std::endl;

        for (auto& command : mCommandMap)
        {
            helpCommands.push_back(command.second.commandFormat);
        }

        std::sort(helpCommands.begin(), helpCommands.end());

        for (auto& command : helpCommands)
        {
            std::cout << command << std::endl;
        }

        return status; 
    });

    return status;
}

void cli::run(int argc, char **argv)
{
    char *prgname = argv[0];
    int async = 0;

    /* Parse options, with --multiline we enable multi line editing. */
    while (argc > 1)
    {
        argc--;
        argv++;
        if (!strcmp(*argv, "--multiline"))
        {
            linenoiseSetMultiLine(1);
            std::cout << CONSOLE_COLOR_BLUE << "Multi-line mode enabled." << CONSOLE_COLOR_RESET << std::endl;
        }
        else if (!strcmp(*argv, "--keycodes"))
        {
            linenoisePrintKeyCodes();
            exit(0);
        }
        else if (!strcmp(*argv, "--async"))
        {
            async = 1;
        }
        else
        {
            std::cout << CONSOLE_COLOR_RED << "Unrecognized option: " << *argv << CONSOLE_COLOR_RESET << std::endl;
            exit(1);
        }
    }

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad(mHistoryFile.c_str()); /* Load the history at startup */

    mThread = std::thread([this, async]() {
        char *line;

        while (!mIsExitRequested)
        {
            /* Asynchronous mode using the multiplexing API: wait for
             * data on stdin, and simulate async data coming from some source
             * using the select(2) timeout. */
            struct linenoiseState ls;
            char buf[1024];
            linenoiseEditStart(&ls, -1, -1, buf,sizeof(buf), mPrompt.c_str());

            for (;;)
            {
                fd_set readfds;
                struct timeval tv;
                int retval;

                FD_ZERO(&readfds);
                FD_SET(ls.ifd, &readfds);
                tv.tv_sec = 1; // 1 sec timeout
                tv.tv_usec = 0;

                retval = select(ls.ifd + 1, &readfds, NULL, NULL, &tv);
                if (retval == -1)
                {
                    perror("select()");
                    exit(1);
                }
                else if (retval)
                {
                    line = linenoiseEditFeed(&ls);
                    /* A NULL return means: line editing is continuing.
                     * Otherwise the user hit enter or stopped editing
                     * (CTRL+C/D). */
                    if (line != linenoiseEditMore)
                    {
                        break;
                    }
                }
                else
                {
                    // // Timeout occurred
                    // static int counter = 0;
                    // linenoiseHide(&ls);
                    // printf("Async output %d.\n", counter++);
                    // linenoiseShow(&ls);
                }
            }
            linenoiseEditStop(&ls);
            if (line == NULL)
            {
                break;
            }

            /* Do something with the string. */
            if (line[0] != '\0' && line[0] != '/')
            {
                std::string command = line;
                processCommand(command);

                linenoiseHistoryAdd(line);                  /* Add to the history. */
                linenoiseHistorySave(mHistoryFile.c_str()); /* Save the history on disk. */
            }
            else if (!strncmp(line, "/historylen", 11))
            {
                /* The "/historylen" command will change the history len. */
                int len = atoi(line + 11);
                linenoiseHistorySetMaxLen(len);
            }
            else if (!strncmp(line, "/mask", 5))
            {
                linenoiseMaskModeEnable();
            }
            else if (!strncmp(line, "/unmask", 7))
            {
                linenoiseMaskModeDisable();
            }
            else if (line[0] == '/')
            {
                std::cout << CONSOLE_COLOR_RED << "Unrecognized command: " << line << CONSOLE_COLOR_RESET << std::endl;
            }
            free(line);
        }

        // std::cout << CONSOLE_COLOR_CYAN << "Exiting..." << CONSOLE_COLOR_RESET << std::endl;
        mIsExitRequested = true; });
}

void cli::setPrompt(const std::string &prompt)
{
    mPrompt = prompt;
}

void cli::setHistoryFile(const std::string &historyFile)
{
    mHistoryFile = historyFile;
}

int32_t cli::registerCommand(const std::string &command, CommandFunction function)
{
    std::unique_lock<std::mutex> lock(mCommandMapMutex);
    int32_t status = 0;
    std::string commandName;
    auto commandFormatSplitted = strsplit(command, ' ');
    CommandInfo newCommandInfo;

    for (;;)
    {
        if (commandFormatSplitted.empty())
        {
            std::cerr << "Command is empty" << std::endl;
            status = ERRNO(EINVAL);
            break;
        }

        if (commandFormatSplitted.size() < 1)
        {
            std::cerr << "Command is invalid" << std::endl;
            status = ERRNO(EINVAL);
            break;
        }

        commandName = commandFormatSplitted[0];
        commandFormatSplitted.erase(commandFormatSplitted.begin());

#if 0
        std::cout << "Registering command: " << commandFormatSplitted[0] << std::endl;
#endif 
        newCommandInfo.commandFormat = command;
        newCommandInfo.function = function;

        // check format
        for (const std::string &arg : commandFormatSplitted)
        {
            // <mustArg>
            if ('<' == arg[0] && '>' == arg[arg.size() - 1])
            {
                std::string argName = arg.substr(1, arg.size() - 2);
#if 0
                std::cout << "Must Arg: " << argName << std::endl;
#endif
                newCommandInfo.argMust.push_back(argName);
                continue;
            }

            // [<optionalArg>]
            if ('[' == arg[0] && '<' == arg[1] && '>' == arg[arg.size() - 2] && ']' == arg[arg.size() - 1])
            {
                std::string argName = arg.substr(2, arg.size() - 4);
#if 0 
                std::cout << "Optional Arg: " << argName << std::endl;
#endif 
                newCommandInfo.argOptional.push_back(argName);
                continue;
            }

            status = ERRNO(EINVAL);
        }

        if (status != 0)
        {
            break;
        }

        mCommandMap[commandName] = std::move(newCommandInfo);

        break;
    }

    return status;
}

std::vector<std::string> cli::getCommandList()
{
    std::unique_lock<std::mutex> lock(mCommandMapMutex);
    std::vector<std::string> commandList;
    for (const auto &command : mCommandMap)
    {
        commandList.push_back(command.first);
    }
    return commandList;
}

bool cli::isExitRequested() const
{
    return mIsExitRequested;
}

void cli::destroy()
{
    mIsExitRequested = true;
    if (mThread.joinable())
    {
        mThread.join();
    }
}

int32_t cli::processCommand(const std::string &command)
{
    std::unique_lock<std::mutex> lock(mCommandMapMutex);
    int32_t status = 0;
    std::vector<std::string> userCmdSplitted = strsplit(command, ' ');
    std::vector<std::string> userCmdMustArgs;
    std::vector<std::string> userCmdOptionalArgs;
    std::string commandName;
    ArgumentMap arguments;

    for (;;)
    {
        if (userCmdSplitted.empty())
        {
            std::cerr << CONSOLE_COLOR_RED << "Command is empty" << CONSOLE_COLOR_RESET << std::endl;
            status = ERRNO(EINVAL);
            break;
        }

        if (userCmdSplitted.size() < 1)
        {
            std::cerr << CONSOLE_COLOR_RED << "Command is invalid" << CONSOLE_COLOR_RESET << std::endl;
            status = ERRNO(EINVAL);
            break;
        }

        commandName = userCmdSplitted[0];
        userCmdSplitted.erase(userCmdSplitted.begin());

        for (const auto &arg : userCmdSplitted)
        {
            if (strcontains(arg, '='))
            {
                userCmdOptionalArgs.push_back(arg);
            }
            else
            {
                userCmdMustArgs.push_back(arg);
            }
        }

#if 0
        std::cout << "Command: " << commandName << std::endl;

        for (const auto &arg : userCmdMustArgs)
        {
            std::cout << "Must Arg: " << arg << std::endl;
        }

        for (const auto &arg : userCmdOptionalArgs)
        {
            std::cout << "Optional Arg: " << arg << std::endl;
        }
#endif

        const auto &foundCmdInfo = mCommandMap.find(commandName);
        const auto &foundCmdFormat = foundCmdInfo->second.commandFormat;

        // check command
        if (foundCmdInfo == mCommandMap.end())
        {
            std::cerr << CONSOLE_COLOR_RED  << "Command not found" << CONSOLE_COLOR_RESET << std::endl;
            status = ERRNO(ENOENT);
            break;
        }

        // check arg count
        if (userCmdMustArgs.size() != foundCmdInfo->second.argMust.size())
        {
            std::cerr << CONSOLE_COLOR_RED  << "Usage: " << foundCmdFormat << CONSOLE_COLOR_RESET << std::endl;
            status = ERRNO(EINVAL);
            break;
        }

        for (size_t i = 0; i < userCmdMustArgs.size(); ++i)
        {
            arguments[foundCmdInfo->second.argMust[i]] = userCmdMustArgs[i];
        }

        for (const auto &arg : userCmdOptionalArgs)
        {
            auto argSplitted = strsplit(arg, '=');
            if (argSplitted.size() != 2)
            {
                std::cerr << CONSOLE_COLOR_RED << "Invalid argument format" << CONSOLE_COLOR_RESET << std::endl;
                status = ERRNO(EINVAL);
                break;
            }   
            if (std::find(foundCmdInfo->second.argOptional.begin(), foundCmdInfo->second.argOptional.end(), argSplitted[0]) == foundCmdInfo->second.argOptional.end())
            {
                std::cerr << CONSOLE_COLOR_RED << "Invalid argument name: " << argSplitted[0] << CONSOLE_COLOR_RESET << std::endl;
                status = ERRNO(EINVAL);
                break;
            }
            arguments[argSplitted[0]] = argSplitted[1];
        }

        if (status != 0)
        {
            break;
        }

#if 0
        for (auto& arg : arguments)
        {
            std::cout << "Argument: " << arg.first << " = " << arg.second << std::endl;
        }
#endif

        foundCmdInfo->second.function(arguments);

        break;
    }

    return status;
}