#ifndef BANG_BASHIT_HPP
#define BANG_BASHIT_HPP

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <sys/wait.h>

using namespace std;

/**
 * @brief Runs a command string through /bin/sh and returns its stdout.
 *
 * Blocks until the command finishes. stderr is NOT captured; it is
 * inherited and goes straight to the terminal. Append "2>&1" to the
 * command if you want it merged into the result.
 *
 * @param cmd Shell command to run (pipes, &&, redirects all work).
 * @return Everything the command wrote to stdout.
 * @throws runtime_error if the process could not be started.
 */
string runCommand(const string &cmd);

/**
 * @brief Same as above, but also reports the exit status.
 *
 * @param cmd    Shell command to run.
 * @param status Set to the command's exit code (or 128+signal if it was
 *               killed). Only meaningful if the function returns.
 * @return Everything the command wrote to stdout.
 */
string runCommand(const string &cmd, int &status);

inline string runCommand(const string &cmd) {
    int status = 0;
    string out = runCommand(cmd, status);
    return out;
}

inline string runCommand(const string &cmd, int &status) {
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw runtime_error("popen() failed for: " + cmd);

    string out;
    array<char, 4096> buffer{};
    size_t n;
    while ((n = fread(buffer.data(), 1, buffer.size(), pipe)) > 0)
        out.append(buffer.data(), n);

    int raw = pclose(pipe);
    if (raw == -1)
        throw runtime_error("pclose() failed for: " + cmd);

    if (WIFEXITED(raw))
        status = WEXITSTATUS(raw);
    else if (WIFSIGNALED(raw))
        status = 128 + WTERMSIG(raw);
    else
        status = -1;

    return out;
}

#endif
