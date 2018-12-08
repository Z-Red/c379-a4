/* 
 * CMPUT 379 - Assignment 4
 * Araien (Zach) Redfern
 * 
 * Utilizes pthreads to simulate the concurrent execution of a set of tasks.
 * The system has a number of resource types, and each resource type has a number of available units.
 * All resource units in the system are non-sharable non-preemptable resources.
 */

#include <sys/resource.h>
#include <pthread.h>
#include <unistd.h>

#include <thread>
#include <mutex>

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Tokenizer.h"
#include "Resource.h"
#include "Task.h"

using namespace std;


mutex mtx;
int NITER;
int monitorTime;
bool monitorPrinting = false;


vector<Task> taskList;
vector<Resource> resList;
chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

std::chrono::milliseconds calculateRunTime() {
    chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();
    std::chrono::milliseconds runTime = chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    return runTime;
}

void taskOutput(Task t, int numIter) {
    std::chrono::milliseconds runTime = calculateRunTime();
    printf("task %s (tid= %lu, iter= %d, time= %ld msec)\n", t.getName().c_str(), (unsigned long)t.getTID(), numIter, runTime.count());
}


void monitorOutput(std::vector<Task> taskList) {

    printf("\nmonitor: [WAIT]");
    for (size_t i = 0; i < taskList.size(); ++i) {
        if (taskList[i].getStatus() == Task::WAIT) {
            printf(" %s", taskList[i].getName().c_str());
        }
    }

    printf("\n\t[RUN]");
    for (size_t i = 0; i < taskList.size(); ++i) {
        if (taskList[i].getStatus() == Task::RUN) {
            printf(" %s", taskList[i].getName().c_str());
        }
    }

    printf("\n\t[IDLE]");
    for (size_t i = 0; i < taskList.size(); ++i) {
        if (taskList[i].getStatus() == Task::IDLE) {
            printf(" %s", taskList[i].getName().c_str());
        }
    }

    printf("\n");
    return;
}

void systemTaskOutput(vector<Task> t, vector<Resource> r) {

    printf("System Tasks\n");
    for (unsigned long i = 0; i < t.size(); ++i) {
        string status = Task::statusToString(t[i].getStatus());
        printf("[%lu] %s (%s, runTime= %d msec, idleTime= %d msec\n",
            i, t[i].getName().c_str(), status.c_str(), t[i].getRunTime(), t[i].getIdleTime());
        printf("\t(tid= %lu)\n", (unsigned long)t[i].getTID());

        for (size_t j = 0; j < r.size(); ++j) {
            string resName = r[j].getName();
            if (t[j].inResMap(resName)) {
                printf("\t%s: (needed= %d, held= %d)\n",
                        r[j].getName().c_str(), t[j].getResMap()[resName], r[j].getNumHeld());
            }
        }
        printf("\t(RUN: %d times, WAIT: %d msec)\n\n", t[i].getNumRuns(), t[i].getWaitTime());

    }
}

void systemResourceOutput(vector<Resource> r) {

    printf("System Resources\n");
    for (size_t i = 0; i < r.size(); ++i) {
        printf("\t%s: (maxAvail= %d, held= %d)\n",
            r[i].getName().c_str(), r[i].getNumAvailable(), r[i].getNumHeld());
    }
    return;
}

// The monitor thread -- output information every monitor time milliseconds.
void* monitorFunc(void* arg) {

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(monitorTime));

        monitorPrinting = true;
        monitorOutput(taskList);
        monitorPrinting = false;
    }

    return ((void*)0);
}

// The task thread -- aquire and hold resources for a period of time.
void* taskFunc(void* arg) {

    // Determine task index
    int idx;
    for (unsigned long i = 0; i < taskList.size(); ++i) {
        if (taskList[i].getTID() == pthread_self()) {
            idx = (int)i;
            break;
        }
    }

    int numIter = 0;
    while(numIter < NITER) {

        chrono::steady_clock::time_point waitStartTime = chrono::steady_clock::now();

        // Attempt to acquire resources
        mtx.lock();

        while(monitorPrinting);
        taskList[idx].setStatus(taskList[idx].WAIT);

        bool gotResources = true;
        unordered_map<string, int> resMap = taskList[idx].getResMap();
        auto begin = resMap.begin();
        auto end = resMap.end();

        for (auto it = resMap.begin(); it != resMap.end(); ++it) {

            string resName = it->first;
            int numUnits = it->second;

            // For each required resource, make sure that resource has the correct number of units available
            for(unsigned long i = 0; i < resList.size(); ++i) {

                if (resList[i].getName() == resName && resList[i].getNumAvailable() < numUnits) {
                    gotResources = false;
                    break;
                }
            }

            // If at any point we couldn't get some of the resources, escape early
            if (!gotResources) {
                break;
            }
        }

        // Only take resources if all were available
        if (gotResources) {

            for (auto it = begin; it != end; ++it) {
                for(unsigned long i = 0; i < resList.size(); ++i) {
                    
                    string resName = (*it).first;
                    int numUnits = (*it).second;
                    if (resList[i].getName() == resName) {
                        resList[i].incrementNumHeld(numUnits);
                    }
                }
            }

            numIter++;
            taskOutput(taskList[idx], numIter);
        } 

        mtx.unlock();

        // If we couldn't get resources, just wait a little longer
        if (!gotResources) {
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        // Calculate time waited trying to collect the necessary resources
        chrono::steady_clock::time_point waitEndTime = chrono::steady_clock::now();
        std::chrono::milliseconds totalWaitTime = chrono::duration_cast<std::chrono::milliseconds>(waitEndTime - waitStartTime);
        taskList[idx].incrementWaitTime(totalWaitTime.count());

        // If we did not get the resources, we just skip this part and try to get some resources again.
        if (gotResources) {

            // Hold the resources
            while(monitorPrinting);
            taskList[idx].setStatus(taskList[idx].RUN);
            this_thread::sleep_for(chrono::milliseconds(taskList[idx].getBusyTime()));
            taskList[idx].incrementRunTime(taskList[idx].getBusyTime());
            taskList[idx].incremenetNumRuns();

            // Release Held resources
            for (auto it = begin; it != end; ++it) {
                for(unsigned long i = 0; i < resList.size(); ++i) {
                    
                    string resName = (*it).first;
                    int numUnits = (*it).second;
                    if (resList[i].getName() == resName) {
                        resList[i].decrementNumHeld(numUnits);
                    }
                }
            }

            // Enter idle period
            while(monitorPrinting);
            taskList[idx].setStatus(taskList[idx].IDLE);
            this_thread::sleep_for(std::chrono::milliseconds(taskList[idx].getIdleTime()));
        }
    }

    return ((void*)0);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("expected four arguments... exiting...\n");
        return 1;
    }

    // Place a 10 minute time limit on cpu resource
    struct rlimit cpuTimeLimit;
    cpuTimeLimit.rlim_cur = 600;
    cpuTimeLimit.rlim_max = 600;
    if (setrlimit(RLIMIT_CPU, & cpuTimeLimit) != 0) {
        cout << "error setting resource limit (cpu time)\n";
    }

    const string inputFileName = string(argv[1]); 
    monitorTime = stoi(argv[2]); 
    NITER = stoi(argv[3]); 

    // Parse input file
    string inputLine;
    ifstream inputFile(inputFileName);
    Tokenizer tokenizer = Tokenizer();
    while(getline(inputFile, inputLine)) {

        if (inputLine.empty())
            continue;

        vector<string> tokens = tokenizer.getTokens(inputLine, ' ');
        if (tokens[0] == "#") {
            continue;
        }

        if (tokens[0] == "resources") {

            // Determine each resource name and its number of available units
            for (size_t i = 1; i < tokens.size(); ++i) {
                vector<string> resData = tokenizer.getTokens(tokens[i], ':');
                resList.push_back(Resource(resData[0], stoi(resData[1])));
            }

        } else {

            string taskName = tokens[1];
            int busyTime = stoi(tokens[2]);
            int idleTime = stoi(tokens[3]);
            unordered_map<string, int> resMap;
            
            // Get resource data for this particular task
            for (size_t i = 4; i < tokens.size(); ++i) {
                vector<string> resData = tokenizer.getTokens(tokens[i], ':');
                string resName = resData[0];
                int unitsRequired = stoi(resData[1]); // Num units required for this tasks to execute
                resMap[resName] = unitsRequired;
            }

            taskList.push_back(Task(taskName, resMap, busyTime, idleTime));
        }
    }

    // Create a single monitor thread
    pthread_t monitorThr;
    if (pthread_create(&monitorThr, NULL, monitorFunc, NULL) != 0) { 
        perror("error creating thread...\n");
        exit(EXIT_FAILURE);
    }

    // Create one thread for each task
    vector<pthread_t> threadList;
    for (unsigned long i = 0; i < taskList.size(); ++i) {
        pthread_t taskThr;
        if (pthread_create(&taskThr, NULL, taskFunc, NULL) != 0) { 
            perror("error creating thread...\n");
            exit(EXIT_FAILURE);
        }
        taskList[i].setTID(taskThr);
        threadList.push_back(taskThr);
    }

    // Wait for threads to finish
    for (size_t i = 0; i < threadList.size(); ++i) {

        if(pthread_join(threadList[i], NULL) != 0) {
            perror("error waiting for thread to return...\n");
            exit(EXIT_FAILURE);
        }

    }

    sleep(1);

    // Print exit information
    systemTaskOutput(taskList, resList);
    systemResourceOutput(resList);
    printf("Running time= %ld msec\n", calculateRunTime().count());
    return 0;
}

