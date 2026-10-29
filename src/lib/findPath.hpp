#ifndef FINDPATH_HPP
#define FINDPATH_HPP

#include <string>

#include "bashit.hpp"

void findPath(string scriptName, string &path);

inline void findPath(string scriptName, string &path) {
    int output;
    string out = runCommand("find .bang/" + scriptName + ".sh", output, true);
    if (output == 0) {
        path = out.substr(0, out.length()-1);
    } else {
        out = runCommand("find ~/.config/bang/" + scriptName + ".sh", output, true);
        if (output == 0) {
            path = out.substr(0, out.length()-1);
        }
    }
}

#endif