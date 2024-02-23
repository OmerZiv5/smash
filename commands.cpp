//		commands.cpp
//********************************************
#include "commands.h"
//********************************************
char prev_wd[MAX_LINE_SIZE] = {};
//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************

bool is_number(const std::string& str){
    for (char c : str) {
        if (!std::isdigit(c)) {
            return FALSE;
        }
    }
    return TRUE;
}

std::string get_file_path(std::string file_name){
    std::string file_path = "";
    char curr_dir[MAX_LINE_SIZE] = {};
    getcwd(curr_dir, MAX_LINE_SIZE);
    // Checking validity of the directory
    if(curr_dir[0] == '\0'){
        perror("smash error: getcwd failed\n");
    }
    else{
        std::string curr_dir_str(curr_dir);
        file_path = curr_dir_str + "/" + file_name;
    }
    return file_path;
}

int ExeCmd(List* jobs, char* lineSize, char* cmdString)
{
    char* cmd;
    char* args[MAX_ARG];
    const char* delimiters = " \t\n";
    int i = 0, num_arg = 0;
    bool illegal_cmd = FALSE; // illegal command
    cmd = strtok(lineSize, delimiters);
    if (cmd == nullptr)
        return 0;
    args[0] = cmd;
    for (i=1; i<MAX_ARG; i++)
    {
        args[i] = strtok(nullptr, delimiters);
        if (args[i] != nullptr)
            num_arg++;

    }
/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
    if (!strcmp(cmd, "cd") )
    {
        int ret_val;
        char curr_wd[MAX_LINE_SIZE] = {};
        //Making sure the number of arguments is correct
        if(num_arg != 1){
            perror("smash error: cd: too many arguments\n");
        }
        else if(!strcmp(args[1], "-")){
            // There is no previous working directory
            if(prev_wd[0] == '\0'){
                perror("smash error: cd: OLDPWD not set\n");
            }
            else{
                getcwd(curr_wd, MAX_LINE_SIZE);
                // Checking validity of the directory
                if(curr_wd[0] == '\0'){
                    perror("smash error: getcwd failed\n");
                }
                else {
                    ret_val = chdir(prev_wd);
                    // Changing working directory failed
                    if (ret_val != 0) {
                        perror("smash error: chdir failed\n");
                    }
                    else {
                        // Setting the prev_wd to the directory we switched from
                        for(int i = 0; i < MAX_LINE_SIZE; i++){
                            prev_wd[i] = '\0';
                        }
                        strcpy(prev_wd, curr_wd);
                    }
                }
            }
        }
        // Argument is not "-" (it's a path)
        else{
            getcwd(curr_wd, MAX_LINE_SIZE);
            // Checking validity of the directory
            if(curr_wd[0] == '\0'){
                perror("smash error: getcwd failed\n");
            }
            else {
                ret_val = chdir(args[1]);
                if(ret_val != 0){
                    perror("smash error: chdir failed\n");
                }
                else {
                    // Setting the prev_wd to the directory we switched from
                    for(int i = 0; i < MAX_LINE_SIZE; i++){
                        prev_wd[i] = '\0';
                    }
                    strcpy(prev_wd, curr_wd);
                }
            }
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "pwd"))
    {
        char curr_wd[MAX_LINE_SIZE] = {};
        getcwd(curr_wd, MAX_LINE_SIZE);
        // Checking validity of the directory
        if(curr_wd[0] == '\0'){
            perror("smash error: getcwd failed\n");
        }
        else{
            std::cout << curr_wd << std::endl;
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "jobs"))
    {
        jobs->Print_List();
    }
        /*************************************************/
    else if (!strcmp(cmd, "showpid"))
    {
        int pid = -1;
        pid = getpid();
        if(pid != -1){
            std::cout << "smash pid is " << pid << std::endl;
        }
        else{
            perror("smash error: getpid failed\n");
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "fg"))
    {
        // Command with no arguments
        if(num_arg == 0){
            if(jobs->job_counter == 0){
                perror("smash error: fg: jobs list is empty\n");
            }
            // List is not empty
            else{
                std::cout << jobs->jobs_list.back().command << " : " << jobs->jobs_list.back().pid << std::endl;
                int sig_res = kill(jobs->jobs_list.back().pid, SIGCONT);
                if(sig_res != 0){
                    perror("smash error: kill failed\n");
                }
                // SIGCONT success
                else{
                    jobs->jobs_list.back().mode = BACKGROUND;
                    jobs->fg_job = jobs->jobs_list.back();
                    jobs->jobs_list.pop_back();
                    jobs->job_counter--;
                    jobs->fg_busy = true;
                    int job_status;
                    int wait_res = waitpid(jobs->fg_job.pid, &job_status, WUNTRACED);
                    if(wait_res == -1){
                        // The fg_job ended abnormally
                        perror("smash error: waitpid failed\n");
                        jobs->fg_busy = false;
                    }
                }
            }
        }
        else if(num_arg == 1) {
            if (!is_number(args[1])) {
                // Wrong syntax
                perror("smash error: fg: invalid arguments\n");
            }
            else {
                std::vector<Job>::iterator it = jobs->Search_Job(std::atoi(args[1]));
                // Job does not exist in the list
                if(it == jobs->jobs_list.end()){
                    std::cerr << "smash error: fg: job-id " << args[1] << " does not exist" << std::endl;
                }
                // Job found in the list
                else{
                    std::cout << it->command << " : " << it->pid << std::endl;
                    int sig_res = kill(it->pid, SIGCONT);
                    if(sig_res != 0){
                        perror("smash error: kill failed\n");
                    }
                    // SIGCONT success
                    else{
                        it->mode = BACKGROUND;
                        jobs->fg_job = *it;
                        jobs->jobs_list.erase(it);
                        jobs->job_counter--;
                        jobs->fg_busy = true;
                        int job_status;
                        int wait_res = waitpid(jobs->fg_job.pid, &job_status, WUNTRACED);
                        if(wait_res == -1){
                            // The fg_job ended abnormally
                            perror("smash error: waitpid failed\n");
                            jobs->fg_busy = false;
                        }
                    }
                }
            }
        }
        else{
            // Incorrect number of arguments
            perror("smash error: fg: invalid arguments\n");
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "bg"))
    {
        // Command with no arguments
        if(num_arg == 0){
            std::vector<Job>::reverse_iterator it = jobs->jobs_list.rbegin();
            for(it = jobs->jobs_list.rbegin(); it != jobs->jobs_list.rend(); it++){
                if(it->mode == STOPPED){
                    break;
                }
            }
            if(it == jobs->jobs_list.rend()){
                // List is empty or there is no stopped job
                perror("smash error: bg: there are no stopped jobs to resume\n");
            }
            else{
                // We found a stopped job in the list
                std::cout << it->command << " : " << it->pid << std::endl;
                int sig_res = kill(it->pid, SIGCONT);
                if (sig_res != 0) {
                    perror("smash error: kill failed\n");
                }
                else{
                    it->mode = BACKGROUND;
                }
            }
        }
        else if(num_arg == 1){
            if (!is_number(args[1])) {
                // Wrong syntax
                perror("smash error: bg: invalid arguments\n");
            }
            else{
                std::vector<Job>::iterator it = jobs->Search_Job(std::atoi(args[1]));
                // Job does not exist in the list
                if(it == jobs->jobs_list.end()){
                    std::cerr << "smash error: bg: job-id " << args[1] << " does not exist" << std::endl;
                }
                else{
                    if(it->mode == BACKGROUND){
                        std::cerr << "smash error: bg: job-id " << it->job_id << " is already running in the background" << std::endl;
                    }
                    else{
                        // We found the correct stopped job in the list
                        std::cout << it->command << " : " << it->pid << std::endl;
                        int sig_res = kill(it->pid, SIGCONT);
                        if (sig_res != 0) {
                            perror("smash error: kill failed\n");
                        }
                        else{
                            it->mode = BACKGROUND;
                        }
                    }
                }
            }
        }
        else{
            // Incorrect amount of arguments
            std::cerr << "smash error: bg: invalid arguments" << std::endl;
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "quit"))
    {
        // Only if the command is: "qqqqqquit kill" then we kill the processes, else we preform quit ragil.
        if(num_arg == 1 && !strcmp(args[1], "kill")){
            for(Job job : jobs->jobs_list) {
                time_t begin_time;
                time(&begin_time);
                int kill_res = kill(job.pid, SIGTERM);
                if(kill_res == 0){
                    // Sending SIGTERM
                    std::cout << "[" << job.job_id<< "] " << job.command << "-Sending SIGTERM... ";
                    while(1){
                        time_t end_time;
                        time(&end_time);
                        time_t kill_time = difftime(begin_time, end_time);
                        bool process_exists;
                        if(job.mode == STOPPED){
                            process_exists = true;
                        }
                        int not_exists = kill(job.pid, 0);
                        if(!not_exists){
                            process_exists =  true;
                        }
                        process_exists = false;
                        if(process_exists){
                            if(kill_time > 5){
                                int sigkill_res = kill(job.pid, SIGKILL);
                                if(sigkill_res != 0){
                                    // Unable to send SIGKILL
                                    std::cout << std::endl;
                                    perror("smash error: kill failed\n");
                                }
                                else{
                                    std::cout << "(5 sec passed) Sending SIGKILL... Done." << std::endl;
                                }
                                break;
                            }
                        }
                        else{
                            // Job is already terminated
                            std::cout << "Done." << std::endl;
                            break;
                        }
                    }
                }
                else{
                    // Unable to send SIGTERM
                    perror("smash error: kill failed\n");
                }
            }
            exit(0);
        }
        else{
            // The command given is just "quit"
            exit(0);
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "kill"))
    {
        if(num_arg != 2){
            std::cerr << "smash error: kill: invalid arguments" << std::endl;
        }
        else{
            // 2 arguments received
            std::string signum = args[1];
            std::string job_id = args[2];
            if(signum[0] != '-'){
                std::cerr << "smash error: kill: invalid arguments" << std::endl;
            }
            else{
                // First character is '-'
                signum = signum.substr(1);
                if(!is_number(signum) || !is_number(job_id)){
                    std::cerr << "smash error: kill: invalid arguments" << std::endl;
                }
                else{
                    // All arguments are valid
                    std::vector<Job>::iterator it = jobs->Search_Job(std::stoi(job_id));
                    if(it == jobs->jobs_list.end()){
                        // List is empty or job doesn't exist
                        std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << std::endl;
                    }
                    else{
                        // Relevant job found
                        int kill_res = kill(it->pid ,stoi(signum));
                        if(kill_res != 0){
                            perror("smash error: kill failed\n");
                        }
                        else{
                            std::cout << "signal number " << signum << " was sent to pid " << it->pid << std::endl;
                        }
                    }
                }
            }
            jobs->Clean_List();
        }
    }
        /*************************************************/
    else if (!strcmp(cmd, "diff"))
    {
        if(num_arg != 2){
            std::cerr << "smash error: diff: invalid arguments" << std::endl;
        }
        else{
            // Valid amount of arguments
            // Get the file paths
            std::string file_name1(args[1]);
            std::string file_name2(args[2]);
            char file_path1[MAX_LINE_SIZE];
            strcpy(file_path1, get_file_path(file_name1).c_str());
            char file_path2[MAX_LINE_SIZE];
            strcpy(file_path2, get_file_path(file_name2).c_str());

            int flags = 0;
            int fp1 = open(file_path1, flags, "r");
            int fp2 = open(file_path2, flags, "r");

            if(fp1 == -1 && fp2 == -1){
                // both opens did not succeed
                std::cerr << "smash error: open failed" << std::endl;
                return -1;
            }
            else if(fp1 == -1){
                std::cerr << "smash error: open failed" << std::endl;
                if(close(fp2) == -1){
                    std::cerr << "smash error: close failed" << std::endl;
                    return -1;
                }
            }
            else if(fp2 == -1){
                std::cerr << "smash error: open failed" << std::endl;
                if(close(fp1) == -1){
                    std::cerr << "smash error: close failed" << std::endl;
                    return -1;
                }
            }
            else{
                char buf1;
                char buf2;
                // fopen succeeded
                while(1){
                    int read1 = read(fp1, &buf1, 1);
                    int read2 = read(fp2, &buf2, 1);
                    if(read1 == -1 || read2 == -1){
                        perror("smash error: read failed\n");
                        break;
                    }
                    // read not failed
                    if(read1 != read2 || buf1 != buf2){
                        std::cout << "1" << std::endl;
                        break;
                    }
                    if(read1 == 0 && read2 == 0){
                        // both reached EOF
                        std::cout << "0" << std::endl;
                        break;
                    }
                }
                int res_close1 = close(fp1);
                int res_close2 = close(fp2);
                if(res_close1 == -1 || res_close2 == -1){
                    perror("smash error: close failed\n");
                    return -1;
                }
            }
        }
    }
        /*************************************************/
    else // external command
    {
        ExeExternal(args, cmdString, num_arg, jobs);
        return 0;
    }
    if (illegal_cmd == TRUE)
    {
        std::cout << "smash error: > \"" << cmdString << "\"" << std::endl;
        return 1;
    }
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmdString, int num_arg, List* jobs)
{
    std::string child_command = cmdString;
    int pID = fork();
    if(pID == -1) {
        // Case of an error in the fork
        perror("smash error: fork failed\n");
        return;
    }
    else if(pID == 0) {
        // Child Process
        setpgrp();
        char curr_wd[MAX_LINE_SIZE] = {};
        getcwd(curr_wd, MAX_LINE_SIZE);
        // Checking validity of the directory
        if (curr_wd[0] == '\0'){
            perror("smash error: getcwd failed\n");
        } else {
            std::string prog_name = args[0];
            std::string curr_wd_str(curr_wd);
            std::string path_name = curr_wd_str + "/" + prog_name;
            const char *full_path_name = path_name.c_str();
            execv(full_path_name, args);
            perror("smash error: execv failed\n");
            exit(-1);
        }
        return;
    }
    else {
        // Dad process
        // Creating a child job
        Job child;
        child.pid = pID;
        child.command = child_command;

        if (std::strcmp(args[num_arg - 1], "&")) {
            // Child running in foreground
            jobs->fg_job = child;
            jobs->fg_busy = true;
            int child_status;
            int wait_res = waitpid(child.pid, &child_status, WUNTRACED);
            if (wait_res == -1) {
                // Child process ended abnormally
                perror("smash error: waitpid failed\n");
                jobs->fg_busy = false;
            }
        } else {
            // Child running in background
            jobs->Add_Job(child);
            int child_status;
            if(waitpid(child.pid, &child_status, WNOHANG) == -1){
                // Child process ended abnormally
                perror("smash error: waitpid failed\n");
            }
        }
        return;
    }
}

