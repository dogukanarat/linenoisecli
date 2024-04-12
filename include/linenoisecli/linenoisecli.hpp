#ifndef INCLUDED_LINENOISECLI_HPP
#define INCLUDED_LINENOISECLI_HPP

#include <thread>
#include <map>
#include <functional>
#include <mutex>

namespace linenoisecli 
{
    class cli 
    {
    public:
        typedef std::map<std::string, std::string> ArgumentMap;
        typedef std::function<int32_t(ArgumentMap&)> CommandFunction;

        static cli& getInstance();
        void setPrompt(const std::string& prompt);
        void setHistoryFile(const std::string& historyFile);
        int32_t registerCommand(const std::string& command, CommandFunction function);
        std::vector<std::string> getCommandList();
        bool isExitRequested() const;
        void run(int argc, char** argv);
        void destroy();


    private:
        typedef struct
        {
            std::string commandFormat;
            CommandFunction function;
            std::vector<std::string> argMust;
            std::vector<std::string> argOptional;
        } CommandInfo;

        cli();
        virtual ~cli();
        cli(const cli&) = delete;
        cli& operator=(const cli&) = delete;
        
        int32_t processCommand(const std::string& command);
        int32_t registerDefaultCommands();

        std::thread mThread;
        bool mIsExitRequested = false;
        std::string mPrompt = "cli> ";
        std::string mHistoryFile = "history.txt";
        std::map<std::string, CommandInfo> mCommandMap;
        std::mutex mCommandMapMutex;
    };

} /* namespace linenoisecli */

#endif /* INCLUDED_LINENOISECLI_HPP */