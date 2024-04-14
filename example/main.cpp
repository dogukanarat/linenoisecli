#include "linenoisecli/linenoisecli.hpp"
#include <thread>
#include <iostream>
#include <signal.h>

#define COLOR_RED         "\x1b[31m"
#define COLOR_GREEN       "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_BLUE        "\x1b[34m"
#define COLOR_RESET       "\x1b[0m"

bool gIsExitRequested = false;

void signalHandler(int signal)
{
    std::cout << "\r" << "Signal " << signal << " received. Exiting..." << std::endl;
    gIsExitRequested = true;
}

int main(int argc, char** argv)
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);

    linenoisecli::cli::getInstance().setPrompt("example> ");
    linenoisecli::cli::getInstance().setHistoryFile("example_history.txt");
    linenoisecli::cli::getInstance().run(argc, argv);

    linenoisecli::cli::getInstance().registerCommand(
        "exampleCmd1 <arg1> <arg2> <arg3>", 
        [](linenoisecli::cli::ArgumentMap& args) -> int32_t
        {
            std::cout << COLOR_GREEN << "exampleCmd1 called with " << args.size() << " arguments." << COLOR_RESET << std::endl;
            for (const auto& arg : args)
            {
                std::cout << COLOR_GREEN << "  " << arg.first << " = " << arg.second << COLOR_RESET << std::endl;
            }
            return 0;
        });

    linenoisecli::cli::getInstance().registerCommand(
        "exampleCmd2 <arg1> <arg2> <arg3> [<arg4>] [<arg5>]", 
        [](linenoisecli::cli::ArgumentMap& args) -> int32_t
        {
            std::cout << COLOR_BLUE << "exampleCmd2 called with " << args.size() << " arguments." << COLOR_RESET << std::endl;
            for (const auto& arg : args)
            {
                std::cout << COLOR_BLUE << "  " << arg.first << " = " << arg.second << COLOR_RESET << std::endl;
            }
            return 0;
        });

    linenoisecli::cli::getInstance().registerCommand(
        "exampleCmd3 [<arg1>]", 
        [](linenoisecli::cli::ArgumentMap& args) -> int32_t
        {
            std::cout << COLOR_YELLOW << "exampleCmd3 called with " << args.size() << " arguments." << COLOR_RESET << std::endl;
            for (const auto& arg : args)
            {
                std::cout << COLOR_YELLOW << "  " << arg.first << " = " << arg.second << COLOR_RESET << std::endl;
            }
            return 0;
        });

    while (!gIsExitRequested && !linenoisecli::cli::getInstance().isExitRequested())
    {
        // sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    linenoisecli::cli::getInstance().destroy();

    return 0;
}