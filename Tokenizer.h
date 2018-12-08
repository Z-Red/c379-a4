/*
 * CMPUT 379 - Assignment 3
 * Araien Zach Redfern
 * 
 * Utiltiy class for parsing tokens from inputs.
 * 
 */ 
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

class Tokenizer {
public:

    // Get the tokens in a string separated by a hyphen
    vector<string> getTokens(string s, char delim) {

        string token;
        istringstream iss(s);
        vector<string> tokens;
        while(getline(iss, token, delim)) {
            tokens.push_back(token);
        }

        // Remove empty spaces
        vector<string> returnList;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i] != "")
                returnList.push_back(tokens[i]);
        }

        return returnList;
    }

};