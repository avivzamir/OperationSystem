#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include<list>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <ctime>
#include <unistd.h>
#include <signal.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

enum class Status {
    BUILT_IN, EXTERNAL
};


class Command {
// TODO: Add your data members
protected:
    const char *cmdline;
    std::string my_cmdLine;
    char *args[COMMAND_MAX_ARGS];
    int numOfArgs;
    bool isStopped;
    pid_t pid;
    bool isBackground;
    Status status;

public:
    Command(const char *cmd_line);

    virtual ~Command() {}

    virtual void execute() = 0;

    void setStatus(const Status &newStatus);

    void stop();

    void cont();

    Status &getStatus();

    pid_t getPid();

    bool getIsStopped();

    char **getArgs();

    int getNumOfArgs();

    const char *getCmdLine() const;

    std::string getMyCmdLine();

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {

public:

    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;

    void setPid(pid_t newPid);
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    QuitCommand(const char *cmd_line);

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {
public:


    class JobEntry {
        // TODO: Add your data members
    private:
        int m_jobId;
        pid_t m_jobPid;
        std::string m_cmdLine;
        bool m_isStopped;
        std::time_t m_jobInsertTime;
        int numOfArgs;//new






    public:
        JobEntry(pid_t pid, std::string cmdLine, time_t jobInsertTime, bool isStopped, int numArgs);

        JobEntry(pid_t pid, std::string cmdLine, time_t jobInsertTime, bool isStopped, int numArgs, int jobId);

        void printJob();

        int getJobID();

        pid_t getJobPid();

        Command *getCmd();

        bool isStopped();

        void setStopped(bool stopped);

        std::string getCmdLine();

        bool getIsStopped();

        int getNumOfArgs();


    };


private:
    std::vector<JobsList::JobEntry *> jobsList;
    int m_currMaxId = 0;

public:
    JobsList() = default;

    ~JobsList() = default;


    void addJob(pid_t cmdPid, std::string lineCmd,int argsNum, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    int getMaxId();

    void incMaxId();

    std::vector<JobsList::JobEntry *> &getJobVec();
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members

public:
    JobsCommand(const char *cmd_line);

    virtual ~JobsCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ChmodCommand(const char *cmd_line);

    virtual ~ChmodCommand() {}

    void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    GetFileTypeCommand(const char *cmd_line);

    virtual ~GetFileTypeCommand() {}

    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    SetcoreCommand(const char *cmd_line);

    virtual ~SetcoreCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line);

    virtual ~KillCommand() {}

    void execute() override;
};


class SmallShell {
private:
    pid_t m_pid;
    pid_t m_cmdPid;
    int currCmdArgs;
    const char* cmdLine;
    std::string currCommandLine;
    JobsList *m_jobsList;
    std::string m_prevDir;
    std::string m_prompt;
    Command *currCmd;
    int currJobId;
    bool m_finished;

    SmallShell() : m_pid(getpid()), m_prompt("smash>"), m_prevDir(""), currCmd(nullptr), currJobId(-1),
                   m_jobsList(new JobsList()), m_finished(false) {}

public:

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    Command *CreateCommand(const char *cmd_line);

    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
    void setPrompt(std::string &prompt);

    pid_t getPid() const;

    std::string getPrevDir();

    void setPrevDir(std::string Dir);

    JobsList *getJobsList();

    int getCurrJobId();

    void setCurrjobId(int jobId);

    std::string getPrompt();

    bool isFinished();

    void setFinished();

    void setRunningPid(pid_t pid);

    pid_t getRunPid();

    Command *getCurrCmd();

    void setCurrCmd(std::string);

    void setCurrCmdArgs(int num);

    int getCurrNumArgs();

    std::string getCmdLine();

    void setCmdlLine(std::string cmdline);

    void nullifyCmdStats();
};

#endif //SMASH_COMMAND_H_
