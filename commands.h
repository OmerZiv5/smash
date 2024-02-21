#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <unistd.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>

#define MAX_LINE_SIZE 80
#define MAX_ARG 20
#define STOPPED 0
#define BACKGROUND 1


class Job{
public:
    int pid;
    int job_id;
    std::string command;
    time_t arrive_time;
    int mode;   // Stopped/background

    Job() {
        pid = 0;
        job_id = 0;
        command = "";
        arrive_time = 0;
        mode = BACKGROUND;
    }

    ~Job(){}

    time_t Seconds_Elapsed(){
        time_t curr_time;
        time(&curr_time);
        time_t seconds_elapsed = difftime(curr_time, this->arrive_time);
        return seconds_elapsed;
    }

    void Print_Job() {
        time_t diff_time = this->Seconds_Elapsed();
        if(this->mode == STOPPED){
            std::cout << "[" << this->job_id << "] " << this->command << " : " << this->pid << " " << diff_time << " secs (stopped)" << std::endl;
        }
        else{
            std::cout << "[" << this->job_id << "] " << this->command << " : " << this->pid << " " << diff_time << " secs" << std::endl;
        }
    }
};



class List {
public:
    std::vector<Job> jobs_list;
    int job_counter;
    Job fg_job;
    bool fg_busy;

    // Constructor
    List() : jobs_list(), job_counter(0), fg_job(), fg_busy(false) {}

    // Destructor
    ~List() {}

    static bool Compare_Job_Ids(const Job& job1, const Job& job2){
        return job1.job_id < job2.job_id;
    }

    void Sort_List(){
        std::sort(jobs_list.begin(), jobs_list.end(), Compare_Job_Ids);
    }

    void Add_Job(Job new_job){
        Clean_List(); // Removing jobs that ended in the background
        time(&new_job.arrive_time);
        // Jobs list is empty
        if(this->job_counter == 0){
            // New job is a new process
            if(new_job.job_id == 0) {
                new_job.job_id = 1;
                this->job_counter++;
            }
            // New job already has job id
            else{
                this->job_counter++;
            }
            jobs_list.push_back(new_job);
            return;
        }
        // Jobs list is not empty
        else {
            // New job has no job id
            if (new_job.job_id == 0) {
                new_job.job_id = (jobs_list.end() - 1)->job_id + 1;
                jobs_list.push_back(new_job);
                this->job_counter++;
                return;
            }
            // New job has a job id
            else{
                jobs_list.push_back(new_job);
                this->Sort_List();
                this->job_counter++;
                return;
            }
        }
    }

    std::vector<Job>::iterator Search_Job(int job_id){
        std::vector<Job>::iterator it = jobs_list.begin();
        for(int i = 0; i < job_counter; i++){
            if(it->job_id == job_id){
                return it;
            }
            it++;
        }
        // In case we didn't find the wanted job in the list
        return jobs_list.end();
    }

    void Clean_List(){
        // Checks which jobs are terminated and takes them out of the list
        for(std::vector<Job>::iterator it = jobs_list.begin(); it != jobs_list.end(); ){
            bool process_exists;
            if(it->mode == STOPPED){
                process_exists = true;
            }
            int not_exists = kill(it->pid, 0);
            if(!not_exists){
                process_exists =  true;
            }
            else{
                process_exists = false;
            }
            if(!process_exists){
                int status;
                waitpid(it->pid, &status, WUNTRACED);
                jobs_list.erase(it);
                job_counter--;
            }
            else{
                it++;
            }
        }
    }

    void Print_List(){
        Clean_List();
        std::vector<Job>::iterator it = jobs_list.begin();
        for(int i = 0; i < job_counter; i++){
            (it + i)->Print_Job();
        }
    }
};

enum Bool { FALSE , TRUE };
int ExeCmd(List* jobs, char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString, int num_arg, List* jobs);

#endif // _COMMANDS_H



