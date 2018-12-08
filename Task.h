/* 
 * CMPUT 379 - Assignment 4
 * Araien (Zach) Redfern
 * 
 * Models a task.
 */ 

#pragma once

#include <string>
#include <unordered_map>

using namespace std;

class Task {
public:

    Task(string n, unordered_map<string, int> map, int bt, int it) :
        name(n), resMap(map), idleTime(it), busyTime(bt) {
        status = WAIT;
        waitTime = 0;
        runTime = 0;
        numRuns = 0;
        tid = 0;
    };

    enum Status {WAIT, RUN, IDLE};

    string getName() { return name; }
    pthread_t getTID() { return tid; }
    void setTID(pthread_t t) { tid = t; }

    unordered_map<string, int> getResMap() { return resMap; }
    bool inResMap(string n) { 
        if (resMap.find(n) != resMap.end())
            return true;
        else 
            return false;   
    }

    int getRunTime() { return runTime; }
    int getIdleTime() { return idleTime; }
    int getBusyTime() { return busyTime; }
    int getWaitTime() { return waitTime; }

    void incrementRunTime(int t) { runTime += t; }
    void incrementWaitTime(int t) { waitTime += t; }

    int getNumRuns() { return numRuns; }
    void incremenetNumRuns() { numRuns++; }

    Status getStatus() { return status; }
    void setStatus(Status s) { status = s; }

    static string statusToString(Status status) {
        if (status == WAIT) {
            return "WAIT";
        } else if (status == RUN) {
            return "RUN";
        } else {
            return "IDLE";
        }
    }

private:

    string name;
    pthread_t tid;
    unordered_map<string, int> resMap;

    int runTime;
    int idleTime;
    int busyTime;
    int waitTime;

    int numRuns;
    Status status;
};
