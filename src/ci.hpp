#ifndef CI_HPP
#define CI_HPP

#include <cstdlib>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "lib/bashit.hpp"

/**
 * @brief Runs CI/CD Workflow in its entirety.
 */
inline void ci() {
  struct Step {
    const char *name;
    const char *path;
  };
  const Step steps[] = {
      {"setup", "./.bang/ci/setup.sh"},
      {"ci", "./.bang/ci/ci.sh"},
      {"deployment", "./.bang/ci/cd.sh"},
  };

  for (const auto &step : steps) {
    if (access(step.path, F_OK) != 0) {
      cout << "bang: " << step.path << " does not exist, skipping "
           << step.name << " step..." << endl;
      continue;
    }
    cout << "bang: running " << step.name << " script..." << endl;
    if (runScript(step.path, {}) != 0) {
      cout << "bang: error: " << step.name << " script failed. exiting..."
           << endl;
      exit(1);
    }
  }
}

#endif
