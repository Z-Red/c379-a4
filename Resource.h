/* 
 * CMPUT 379 - Assignment 4
 * Araien (Zach) Redfern
 * 
 * Models a resources.
 */

#pragma once

#include <string>

using namespace std;

class Resource {
public:

    Resource(string n, int u) : name(n), numUnits(u), numAvailable(u) {
        numHeld = 0;
    };

    string getName() { return name; }

    int getTotalUnits() { return numUnits; }
    int getNumAvailable() { return numAvailable; }
    int getNumHeld() { return numHeld; }

    void incrementNumHeld(int num) {
        numAvailable -= num;
        numHeld += num;
    }

    void decrementNumHeld(int num) {
        numAvailable += num;
        numHeld -= num;
    }
    
private:

    string name;
    int numUnits;
    int numAvailable;
    int numHeld;
};