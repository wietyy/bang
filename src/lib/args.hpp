#ifndef BANG_ARGS_HPP
#define BANG_ARGS_HPP

#include <vector>
#include <string>
using namespace std;

/**
 * @brief Parses arguments. Simple as dat.
 *
 * @param argc argc
 * @param argv argv
 * @param flags Reference to vec<string> that will be filled with flags (- and -- parameters)
 * @param scriptArgs Same as above but for script arguments being passed
 * @param scriptName Reference to string that will hold the script name
 */
inline void parse(int argc, char** argv, vector<string> &flags, vector<string> &scriptArgs, string &scriptName) {
    bool gotScriptName = false;
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (!arg.empty() && arg.at(0) == '-') {
            flags.push_back(arg);
        } else {
            if (!gotScriptName) {
                scriptName = arg;
                gotScriptName = true;
            } else {
                scriptArgs.push_back(arg);
            }
        }
    }
}

#endif
