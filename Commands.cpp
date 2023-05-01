#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <utime.h>
#include <fcntl.h>


using namespace std;


const std::string WHITESPACE = " \n\r\t\f\v";


#if 0



#define FUNC_ENTRY()  \



cout << __PRETTY_FUNCTION__ << " --> " << endl;







#define FUNC_EXIT()  \



cout << __PRETTY_FUNCTION__ << " <-- " << endl;



#else


#define FUNC_ENTRY()


#define FUNC_EXIT()


#endif


string _ltrim(const std::string &s) {


    size_t start = s.find_first_not_of(WHITESPACE);


    return (start == std::string::npos) ? "" : s.substr(start);


}


string _rtrim(const std::string &s) {


    size_t end = s.find_last_not_of(WHITESPACE);


    return (end == std::string::npos) ? "" : s.substr(0, end + 1);


}


string _trim(const std::string &s) {


    return _rtrim(_ltrim(s));


}


int _parseCommandLine(const char *cmd_line, char **args) {


    FUNC_ENTRY()



    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {

        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;







    FUNC_EXIT()


}


bool _isBackgroundComamnd(const char *cmd_line) {


    const string str(cmd_line);


    return str[str.find_last_not_of(WHITESPACE)] == '&';


}


void _removeBackgroundSign(char *cmd_line) {


    const string str(cmd_line);



    // find last character other than spaces



    unsigned int idx = str.find_last_not_of(WHITESPACE);



    // if all characters are spaces then return



    if (idx == string::npos) {


        return;


    }



    // if the command line does not end with & then return



    if (cmd_line[idx] != '&') {


        return;


    }



    // replace the & (background sign) with space and then remove all tailing spaces.



    cmd_line[idx] = ' ';



    // truncate the command line string up to the last non-space character



    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;


}


void JobsList::addJob(pid_t cmdPid, std::string lineCmd, int argsNum, bool isStopped) {
    removeFinishedJobs();
    SmallShell &smashy = SmallShell::getInstance();
    if (smashy.getCurrJobId() == -1) {
        JobsList::JobEntry *job = new JobsList::JobEntry(cmdPid, lineCmd, std::time(0), isStopped, argsNum
        );
        smashy.getJobsList()->getJobVec().push_back(job);
        return;
    } else {
        JobsList::JobEntry *job = new JobsList::JobEntry(cmdPid, lineCmd, std::time(0), isStopped,
                                                         argsNum, smashy.getCurrJobId());
        smashy.setCurrjobId(-1);
        smashy.getJobsList()->getJobVec().push_back(job);
        return;
    }

}

JobsList::JobEntry::JobEntry(pid_t pid, std::string cmdLine, std::time_t jobInsertTime, bool isStopped, int numArgs)
        : m_jobId(SmallShell::getInstance().getJobsList()->getMaxId() + 1), m_jobPid(pid), m_cmdLine(cmdLine),
          m_isStopped(isStopped), m_jobInsertTime(
                jobInsertTime), numOfArgs(numArgs) {
    SmallShell::getInstance().getJobsList()->incMaxId();
}

JobsList::JobEntry::JobEntry(pid_t pid, std::string cmdLine, std::time_t jobInsertTime, bool isStopped, int numArgs,
                             int jobId)
        : m_jobId(
        jobId), m_jobPid(pid), m_cmdLine(cmdLine), numOfArgs(numArgs), m_isStopped(isStopped), m_jobInsertTime(
        jobInsertTime) {}

void JobsList::printJobsList() {
    this->removeFinishedJobs();
    for (JobEntry *job: jobsList) {
        job->printJob();
    }
}

int JobsList::JobEntry::getJobID() {
    return m_jobId;
}

pid_t JobsList::JobEntry::getJobPid() {
    return m_jobPid;
}

std::string JobsList::JobEntry::getCmdLine() {
    return m_cmdLine;
}

bool JobsList::JobEntry::getIsStopped() {
    return m_isStopped;
}

int JobsList::JobEntry::getNumOfArgs() {
    return this->numOfArgs;
}

void JobsList::removeJobById(int jobId) {
    int currmax = 0;
    for (std::vector<JobsList::JobEntry *>::iterator job = jobsList.begin(); job != jobsList.end(); ++job) {

        if ((*job)->getJobID() == jobId) {
            delete *job;
            //cout << jobsList.size() << endl;
            jobsList.erase(job);
            --job;
            //cout << jobsList.size() << endl;
            continue;
        }

        (*job)->getJobID() > currmax ? currmax = (*job)->getJobID() : currmax = currmax;
    }
    currmax < m_currMaxId ? m_currMaxId = currmax : m_currMaxId = m_currMaxId;
}

void JobsList::JobEntry::printJob() {
    std::time_t now = std::time(nullptr);
    std::cout << "[" << m_jobId << "] " << this->getCmdLine() << ": " << m_jobPid << " "
              << std::difftime(now, m_jobInsertTime) << " secs";
    if (m_isStopped) {
        std::cout << " " << "(stopped)";
    }
    std::cout << endl;
}

bool JobsList::JobEntry::isStopped() {
    return m_isStopped;
}

void JobsList::JobEntry::setStopped(bool stopped) {
    this->m_isStopped = stopped;
}

void JobsList::removeFinishedJobs() {
    if (jobsList.size() == 0) {
        return;
    }
    for (std::vector<JobsList::JobEntry *>::iterator job = jobsList.begin(); job != jobsList.end(); ++job) {

        pid_t result = waitpid((*job)->getJobPid(), nullptr, WNOHANG);
        if (result > 0) {
            removeJobById((*job)->getJobID());


        } else if (result == -1) {
            std::perror("smash error: waitpid failed");
        }
    }
}

void JobsList::killAllJobs() {
    this->removeFinishedJobs();
    for (std::vector<JobsList::JobEntry *>::iterator job = this->getJobVec().begin();
         job != this->getJobVec().end(); ++job) {
        cout << (*job)->getJobPid() << ": " << (*job)->getCmdLine() << endl;
        kill((*job)->getJobPid(), SIGKILL);
        delete *job;
    }

}

std::vector<JobsList::JobEntry *> &JobsList::getJobVec() {
    return jobsList;
}

// TODO: Add your implementation for classes in Commands.h 



Command::Command(const char *cmd_line) : cmdline(cmd_line), isStopped(false), pid(getpid()),
                                         isBackground(_isBackgroundComamnd(cmd_line)), status(Status::BUILT_IN) {
    std::string temp = cmd_line;
    this->my_cmdLine = temp;
    numOfArgs = _parseCommandLine(cmd_line, args);
    // cout<<endl<<endl<< "Here we do parsing (inside Command cstr)" << endl<<"args is: "<<args<<endl<<endl;
    //   for(int i=0; i< numOfArgs; i++){
    //	cout<<"args["<<i<<"] = " << args[i]<<endl;}
}


void Command::setStatus(const Status &newStatus) {


    status = newStatus;


}

const char *Command::getCmdLine() const {
    return this->cmdline;
}

int Command::getNumOfArgs() {
    return numOfArgs;
}


void Command::stop() {



    //validity check///////////////////////////////////////



    isStopped = true;// מישהו אחר יעביר אותו לרשימה של המחכים, וגםיקרא לפונקציה הזו בשביל לסמן זאת



}


void Command::cont() {



    //validity



    isStopped = false;


}


Status &Command::getStatus() {


    return status;


}


pid_t Command::getPid() {


    return pid;


}


bool Command::getIsStopped() {


    return isStopped;


}

char **Command::getArgs() {
    return args;
}

std::string Command::getMyCmdLine() {
    return this->my_cmdLine;
}


ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {
    status = Status::EXTERNAL;
}

void ExternalCommand::setPid(pid_t newPid) {
    this->pid = newPid;
}

void ExternalCommand::execute() {

    pid_t son_pid = fork();
    if (son_pid < 0) {
        std::perror("smash error: fork failed");
        return;
    }
    bool isComplex = false;
    if (son_pid == 0)//child
    {
        this->setPid(getpid());
        int i = 0;
        while (cmdline[i]) {
            if (cmdline[i] == '*' || cmdline[i] == '?') {
                isComplex = true;
            }
            ++i;
        }
        char cmdLExe[COMMAND_ARGS_MAX_LENGTH] = {0};
        strcpy(cmdLExe, cmdline);
        _removeBackgroundSign(cmdLExe);
        char *argsexec[COMMAND_ARGS_MAX_LENGTH] = {0};
        _parseCommandLine(cmdLExe, argsexec);
        if (!isComplex) {
            int worked = execvp(argsexec[0], argsexec);
            if (worked < 0) {
                std::perror("smash error: execv failed");
                return;
                exit(EXIT_FAILURE);
            }

        } else {
            char *const argsForExec[4] = {(char *) "/bin/bash", (char *) "-c", cmdLExe, nullptr};
            int worked = execv(argsForExec[0], argsForExec);
            if (worked < 0) {

                std::perror("smash error: execv failed");
                return;
                exit(EXIT_FAILURE);
            }
        }

    } else {//father
        SmallShell &smashy = SmallShell::getInstance();
        this->setPid(son_pid);

        if (isBackground) {
            smashy.getJobsList()->addJob(this->getPid(),this->cmdline,this->getNumOfArgs(), this->getIsStopped());
        } else {
            smashy.setCurrjobId(-1);
            int worked = waitpid(son_pid, nullptr, WUNTRACED);
            if (worked < 0) {
                std::perror("smash error: waitpid failed");
                return;
            }
            smashy.setRunningPid(son_pid);
        }
    }
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}


PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) {}


void PipeCommand::execute() {}



// 1







RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line) {}


void RedirectionCommand::execute() {}



//2



void ChangeDirCommand::execute() {

    if (numOfArgs > 2) {
        std::perror("smash error: cd: too many arguments");
    }
    char buff[PATH_MAX + 1];
    std::string toSetAsPrev = getcwd(buff, PATH_MAX + 1);


    if (!strcmp(args[1], "-")) {                           // if it was "cd -"
        //cout <<endl<< "Entering 'cd -'"<<endl<<endl;
        if (SmallShell::getInstance().getPrevDir() == "") {
            std::perror("smash error: cd: OLDPWD not set");
            return;
        } else { // if prevDir was indeed defined
            char *toMoveTo = new char(strlen((SmallShell::getInstance().getPrevDir()).c_str()) + 1);
            strcpy(toMoveTo, (SmallShell::getInstance().getPrevDir()).c_str());

            if (chdir(toMoveTo) == -1) {
                std::perror("smash error: chdir failed");
                return;
            }
            delete[] toMoveTo;
        }
    } else if (!strcmp(args[1], "..")) {
        if (chdir("..") == -1) {
            std::perror("smash error: chdir failed");
            return;
        }
    } else {
        if (chdir(args[1]) == -1) {
            std::perror("smash error: chdir failed");
            return;
        }
    }
    // cout<<"||||||||||||||||||||||||||||||||||||" << endl;
    // cout<< "temp now is for setting prevDir: " << toSetAsPrev << endl;
    //cout << "||||||||||||||||||||||||||||||||||||" << endl;
    SmallShell::getInstance().setPrevDir(toSetAsPrev);

}//3















GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void GetCurrDirCommand::execute() {
    char buff[PATH_MAX];
    size_t size = PATH_MAX;
    bool success = false;
    success = getcwd(buff, size);
    if (success) {
        std::cout << buff << std::endl;
    } else {
        perror("smash error: cmd get cwd failed");
    }
}



//4







ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void ShowPidCommand::execute() {


    SmallShell &smash = SmallShell::getInstance();


    std::cout << "smash pid is " << smash.getPid() << endl;


}



//5



QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void QuitCommand::execute() {
    SmallShell &smashy = SmallShell::getInstance();

    if (numOfArgs >= 2 && args[1] == "kill") {
        smashy.getJobsList()->removeFinishedJobs();
        cout << "smash: sending SIGKILL signal to " << smashy.getJobsList()->getJobVec().size() << " jobs:" << endl;
        smashy.getJobsList()->killAllJobs();
        kill(smashy.getPid(), SIGKILL);
    } else if (numOfArgs == 1 || (numOfArgs > 1 && args[1] != "kill")) {
        cout << "GGG" << endl;
        for (std::vector<JobsList::JobEntry *>::iterator job = smashy.getJobsList()->getJobVec().begin();
             job != smashy.getJobsList()->getJobVec().end(); ++job) {
            delete (*job);
            smashy.setFinished();
        }
    }

}//6







JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void JobsCommand::execute() {
    if (SmallShell::getInstance().getJobsList() != nullptr)
        SmallShell::getInstance().getJobsList()->printJobsList();
}


ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    SmallShell &smashy = SmallShell::getInstance();
}


void ForegroundCommand::execute() {
    SmallShell &smashy = SmallShell::getInstance();


    if (smashy.getJobsList()->getJobVec().empty()) {
        std::perror("smash error: fg: jobs list is empty");
        return;
    }
    pid_t jobPid;
    JobsList::JobEntry *currJob = nullptr;
    int st = 0;


    if (numOfArgs > 2) {
        std::perror("smash error: fg: invalid arguments");
        return;


    } else if (numOfArgs == 1) {
        for (std::vector<JobsList::JobEntry *>::iterator job = smashy.getJobsList()->getJobVec().begin();
             job != smashy.getJobsList()->getJobVec().end(); ++job) {
            if ((*job)->getJobID() == smashy.getJobsList()->getMaxId()) {
                currJob = (*job);
                jobPid = (*job)->getJobPid();
            }
        }
    } else if (numOfArgs == 2) {
        int temp = std::stoull(args[1]);
        currJob = smashy.getJobsList()->getJobById(temp);
        if (!currJob) {
            std::string jobName = args[1];
            std::string errPrint = "smash error: fd: job-id " + jobName + " does not exist";
            std::cerr << errPrint << endl;
            std::perror("");
            return;
        }
        jobPid = currJob->getJobPid();
    }

    //now we have the desired job to move to foreground

    if (currJob->getIsStopped()) {
        st = kill(jobPid, SIGCONT);
        std::perror("kill failed");
        return;
    }

    std::cout << currJob->getCmdLine() << " : " << jobPid << endl;
    int prevIdJob = smashy.getCurrJobId();
    smashy.setCurrjobId(currJob->getJobID());

    st = waitpid(jobPid, nullptr, WUNTRACED);


    if (smashy.getJobsList()->getJobById(currJob->getJobID())) {
        if (currJob->getIsStopped()) {
            smashy.getJobsList()->removeJobById(currJob->getJobID());
        }
    }
    if (st != jobPid) {

        smashy.setCurrjobId(prevIdJob);
        std::perror("smash error: waitpid failed");
        return;
    }
    smashy.setRunningPid(jobPid);
    smashy.setCmdlLine(currJob->getCmdLine());
    smashy.setCurrCmdArgs(currJob->getNumOfArgs());
}//8























BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void BackgroundCommand::execute() {
    SmallShell &smashy = SmallShell::getInstance();
    int maxStoppedId = -1;
    JobsList::JobEntry *jobToCon;
    if (this->numOfArgs > 2) {
        std::perror("smash error: bg: invalid arguments");
    } else if (this->numOfArgs == 1) {
        for (std::vector<JobsList::JobEntry *>::iterator job = smashy.getJobsList()->getJobVec().begin();
             job != smashy.getJobsList()->getJobVec().end(); ++job) {
            if ((*job)->isStopped() && (*job)->getJobID() > maxStoppedId) {
                maxStoppedId = (*job)->getJobID();
                jobToCon = (*job);
            }
        }
        if (maxStoppedId == -1) {
            std::perror("smash error: bg: there is no stopped jobs to resume");
            return;
        }
    } else {
        int temp = std::stoull(args[1]);
        for (std::vector<JobsList::JobEntry *>::iterator job = smashy.getJobsList()->getJobVec().begin();
             job != smashy.getJobsList()->getJobVec().end(); ++job) {
            if ((*job)->getJobID() == temp) {
                if (!(*job)->isStopped()) {
                    cout << "smash error: bg: job-id" << (*job)->getJobID() << "is already running in the background";
                    std::perror("");
                    return;
                }
                maxStoppedId = (*job)->getJobID();
                jobToCon = (*job);
            }
        }
        if (maxStoppedId == -1) {
            std::string jobName = args[1];
            std::string errPrint = "smash error: fd: job-id " + jobName + " does not exist";
            std::cerr << errPrint << endl;
            std::perror("");
            return;
        }
    }
    int result = kill(jobToCon->getJobPid(), SIGCONT);
    if (result < 0) {
        std::perror("smash error: bg: kill failed");
        return;
    }
    std::string jobName = args[1];
    std::string jobRealName = "";
    int len = jobName.size();
    if (jobName[len - 1] == '&') {
        for (int i = 0; i < len - 1; ++i) {
            jobRealName += jobName[i];
        }
        cout << jobRealName << ":" << maxStoppedId << endl;

    } else {
        cout << jobName << ":" << maxStoppedId << endl;
    }
    jobToCon->setStopped(false);
}//9

























ChmodCommand::ChmodCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void ChmodCommand::execute() {

    std::string newPrompt = "";
    if (numOfArgs == 2) {
        int i = 0;
        while (args[1][i]) {
            newPrompt += args[1][i];
            ++i;
        }
        SmallShell::getInstance().setPrompt(newPrompt);
    } else if (numOfArgs == 1) {
        std::string empStr = "";
        SmallShell::getInstance().setPrompt(empStr);
    } else {
        std::perror("too many arguments");
    }
}//10















// GetFileTypeCommand::GetFileTypeCommand(const char* cmd_line):BuiltInCommand(cmd_line){







// }



// void GetFileTypeCommand::execute(){}//11



















// SetcoreCommand::SetcoreCommand(const char* cmd_line):BuiltInCommand(cmd_line){







// }



// void SetcoreCommand::execute(){}//12























KillCommand::KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}


void KillCommand::execute() {
    SmallShell &smashy = SmallShell::getInstance();
    int signum, jobId;
    if (numOfArgs != 3) {
        std::perror("smash error: kill: invalid arguments");
        return;
    }
    signum = std::stoi(args[1]) * (-1);
    jobId = std::stoull(args[2]);
    if (signum <= 0 || signum > 64) {
        std::perror("smash error: kill: invalid arguments");
        return;
    }
    JobsList::JobEntry *currJob = smashy.getJobsList()->getJobById(jobId);
    if (currJob == nullptr) {
        cout << "smash error: kill: job-id " << jobId << " does not exist";
        std::perror("");
        return;
    }
    pid_t currJobPid = (*currJob).getJobPid();
    int worked = 0;

    worked = kill(currJobPid, signum);
    cout << "worked: " << worked << endl;
    if (worked < 0) {
        std::perror("smash error: kill failed");
        return;
    }
    if (!worked && signum == 9) {
        smashy.getJobsList()->removeJobById(currJob->getJobID());
    }

    if (signum == SIGSTOP) {
        currJob->setStopped(true);
    } else if (signum == SIGCONT) {
        currJob->setStopped(false);
    }
    cout << "signal number " << signum << " was sent to pid " << currJobPid << endl;


}


SmallShell::~SmallShell() {
// TODO: add your implementation
    delete this->m_jobsList;
}


/**



* Creates and returns a pointer to Command class which matches the given command line (cmd_line)



*/



Command *SmallShell::CreateCommand(const char *cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChmodCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    } else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    }



//addmore

    else {
        return new ExternalCommand(cmd_line);
    }


    return nullptr;


}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *newCommand = SmallShell::CreateCommand(cmd_line);

    if (newCommand == nullptr) {
        return;
    }
    if (newCommand->getStatus() == Status::BUILT_IN) {

        newCommand->execute();
        delete newCommand;
    } else {// external_____________________________________________
        //add to vector jobs list in background or as stopped

        newCommand->execute();
        delete newCommand;


        // make sure all parameters of job are filled correctly in addJob
    }
    //maybe different for special?


}


int JobsList::getMaxId() {
    return this->m_currMaxId;
}

void JobsList::incMaxId() {
    m_currMaxId++;
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    for (std::vector<JobsList::JobEntry *>::iterator job = this->getJobVec().begin();
         job != this->getJobVec().end(); ++job) {
        if ((*job)->getJobID() == jobId) {
            return (*job);
        }
    }
    return nullptr;
}

pid_t SmallShell::getPid() const {
    return m_pid;
}

std::string SmallShell::getPrevDir() {
    return m_prevDir;
}

void SmallShell::setPrevDir(std::string dir) {
    //cout<< endl << endl << "Now setting pevDir inside SmallShell, and it is:" << endl << dir<< endl << endl;
    this->m_prevDir = dir;
}

void SmallShell::setPrompt(std::string &newPrompt) {
    if (newPrompt.empty()) {
        m_prompt = "smash>";
    } else {
        std::string toretPrompt = newPrompt + '>';
        m_prompt = toretPrompt;
    }
}

int SmallShell::getCurrJobId() {
    return currJobId;
}

void SmallShell::setCurrjobId(int jobId) {
    currJobId = jobId;
}

std::string SmallShell::getPrompt() {
    return m_prompt;
}

bool SmallShell::isFinished() {
    return m_finished;
}

void SmallShell::setFinished() {
    m_finished = true;
}

void SmallShell::setRunningPid(pid_t pid) {
    m_cmdPid = pid;
}
//void SmallShell::executeCommand(const char *cmd_line) {

pid_t SmallShell::getRunPid() {
    return m_cmdPid;
}

Command *SmallShell::getCurrCmd() {
    return this->currCmd;
}

void SmallShell::setCurrCmdArgs(int num){
    this->currCmdArgs = num;
}

int SmallShell::getCurrNumArgs(){
    return this->currCmdArgs;
}

std::string SmallShell::getCmdLine(){
    return this->currCommandLine;
}

void SmallShell::setCmdlLine(std::string cmdline){
    this->currCommandLine = cmdline;
}

void SmallShell::nullifyCmdStats() {
    m_cmdPid = -1;
    currCmdArgs = -1;
    currCommandLine = "";
    currJobId = -1;
}


// TODO: Add your implementation here



// for example:



// Command* cmd = CreateCommand(cmd_line);



// cmd->execute();



// Please note that you must fork smash process for some commands (e.g., external commands....)



//}

JobsList *SmallShell::getJobsList() {
    return m_jobsList;
}








