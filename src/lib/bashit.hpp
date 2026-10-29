#ifndef BANG_BASHIT_HPP
#define BANG_BASHIT_HPP

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

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

/**
 * @brief Same as above, with control over the child's stderr.
 *
 * stdout is always captured and returned, never printed. stderr is
 * inherited by default, which means it goes straight to the terminal.
 * Set @p silenceStderr to true to drop it instead.
 *
 * @param cmd           Shell command to run.
 * @param status        Set to the command's exit code.
 * @param silenceStderr When true, the command's stderr is discarded.
 * @return Everything the command wrote to stdout.
 */
string runCommand(const string &cmd, int &status, bool silenceStderr);

/**
 * @brief Runs a script file, letting it talk to the terminal.
 *
 * Unlike runCommand(), this does NOT capture anything. The script inherits
 * stdin, stdout and stderr, so its output appears immediately, it can read
 * from the terminal, and it can be interactive. Arguments are passed through
 * without a shell, so no quoting or escaping is needed.
 *
 * If the file is executable it is exec'd directly, so its shebang decides the
 * interpreter. If it is not executable it is run with /bin/sh.
 *
 * @param path  Path to the script.
 * @param args  Arguments to pass to it.
 * @param quiet When true, the script's stdout and stderr go to /dev/null.
 *              stdin is still inherited.
 * @return The script's exit code (128+signal if it was killed, -1 otherwise).
 * @throws runtime_error if fork() or waitpid() fails.
 */
int runScript(const string &path, const vector<string> &args, bool quiet = false);

inline string runCommand(const string &cmd) {
    int status = 0;
    string out = runCommand(cmd, status);
    return out;
}

inline string runCommand(const string &cmd, int &status) {
    return runCommand(cmd, status, false);
}

inline string runCommand(const string &cmd, int &status, bool silenceStderr) {
    string full = silenceStderr ? cmd + " 2>/dev/null" : cmd;

    FILE *pipe = popen(full.c_str(), "r");
    if (!pipe)
        throw runtime_error("popen() failed for: " + full);

    string out;
    array<char, 4096> buffer{};
    size_t n;
    while ((n = fread(buffer.data(), 1, buffer.size(), pipe)) > 0)
        out.append(buffer.data(), n);

    int raw = pclose(pipe);
    if (raw == -1)
        throw runtime_error("pclose() failed for: " + full);

    if (WIFEXITED(raw))
        status = WEXITSTATUS(raw);
    else if (WIFSIGNALED(raw))
        status = 128 + WTERMSIG(raw);
    else
        status = -1;

    return out;
}

inline int runScript(const string &path, const vector<string> &args, bool quiet) {
    vector<string> argStore;

    // Executable scripts are exec'd directly, so the kernel handles the
    // shebang. Otherwise read it ourselves so "#!/usr/bin/env bash" still
    // means bash, not /bin/sh.
    if (access(path.c_str(), X_OK) == 0) {
        argStore.push_back(path);
    } else {
        string shebang;
        if (FILE *f = fopen(path.c_str(), "r")) {
            char line[512] = {};
            if (fgets(line, sizeof line, f))
                shebang = line;
            fclose(f);
        }

        vector<string> interpreter;
        if (shebang.compare(0, 2, "#!") == 0) {
            string word;
            for (size_t i = 2; i <= shebang.size(); i++) {
                char c = i < shebang.size() ? shebang[i] : ' ';
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    if (!word.empty()) {
                        interpreter.push_back(word);
                        word.clear();
                    }
                } else {
                    word += c;
                }
            }
        }

        if (interpreter.empty())
            argStore.push_back("/bin/sh");
        else
            argStore.insert(argStore.end(), interpreter.begin(), interpreter.end());
        argStore.push_back(path);
    }
    argStore.insert(argStore.end(), args.begin(), args.end());

    vector<char *> argv;
    argv.reserve(argStore.size() + 1);
    for (auto &arg : argStore)
        argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1)
        throw runtime_error("fork() failed for: " + path);

    if (pid == 0) {
        if (quiet) {
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull != -1) {
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
                if (devnull > STDERR_FILENO)
                    close(devnull);
            }
        }
        execv(argv[0], argv.data());
        fprintf(stderr, "bang: cannot execute %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    int raw = 0;
    while (waitpid(pid, &raw, 0) == -1) {
        if (errno != EINTR)
            throw runtime_error("waitpid() failed for: " + path);
    }

    if (WIFEXITED(raw))
        return WEXITSTATUS(raw);
    if (WIFSIGNALED(raw))
        return 128 + WTERMSIG(raw);
    return -1;
}

#endif
