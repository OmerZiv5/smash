#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cstdio>
#include "commands.h"


#define MAX_LINE_SIZE 80
#define MAXARGS 20

char* L_Fg_Cmd;
List* jobs = nullptr;
char lineSize[MAX_LINE_SIZE];

/*************PASTE THE SIGNAL HANDLERS HERE******************************/
void handler(int signum){
    if(jobs->fg_busy == false){
        // There is no external process in foreground
        return;
    }
    if(signum == 2){
        // Control-C
        std::cout << "smash: caught ctrl-C" << std::endl;
        int kill_res = kill(jobs->fg_job.pid, SIGKILL);
        if(kill_res != 0){
            perror("smash error: kill failed\n");
        }
        else{
            std::cout << "smash: process " << jobs->fg_job.pid << " was killed" << std::endl;
            jobs->fg_busy = false;
        }
    }
    else if(signum == 20){
        // Control-Z
        std::cout << "smash: caught ctrl-Z" << std::endl;
        int kill_res = kill(jobs->fg_job.pid, SIGSTOP);
        if(kill_res != 0){
            perror("smash error: kill failed\n");
        }
        else{
            // Process was stopped manually
            jobs->fg_job.mode = STOPPED;
            jobs->Add_Job(jobs->fg_job);
            std::cout << "smash: process " << jobs->fg_job.pid << " was stopped" << std::endl;
            jobs->fg_busy = false;
        }
    }
    else{
        // Unfamiliar signal
        return;
    }
}




//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{
    char cmdString[MAX_LINE_SIZE];

    //signal declaretions
    struct sigaction act_c;
    struct sigaction act_z;
    act_c.sa_handler = &handler;
    act_z.sa_handler = &handler;
    // Setting up the control-C mask
    sigfillset(&act_c.sa_mask);
    sigdedelset(&act_c.sa_mask, SIGINT);
    // Setting up the control-Z mask
    sigfillset(&act_z.sa_mask);
    sigdedelset(&act_z.sa_mask, 20);
    //NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
    // Change the response for control + c
    if(sigaction(2, &act_c, nullptr) == -1){
        perror("smash error: sigaction failed\n");
        return -1;
    }
    // Change the response for control + z
    if(sigaction(20, &act_z, nullptr) == -1){
        perror("smash error: sigaction failed\n");
        return -1;
    }

    /************************************/
    //NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
    //set your signal handlers here
    /* add your code here */

    /************************************/

    /************************************/
    // Init globals

    L_Fg_Cmd =new char[MAX_LINE_SIZE+1];
    if (L_Fg_Cmd == nullptr)
        exit (-1);
    L_Fg_Cmd[0] = '\0';

    List job_list;
    jobs = &job_list;


    while (1)
    {
        std::cout << "smash > ";
        fgets(lineSize, MAX_LINE_SIZE, stdin);
        strcpy(cmdString, lineSize);
        cmdString[strlen(lineSize)-1]='\0';
        jobs->Clean_List();
        // execute commands
        ExeCmd(jobs, lineSize, cmdString);

        /* initialize for next line read*/
        lineSize[0]='\0';
        cmdString[0]='\0';
    }
    return 0;
}



