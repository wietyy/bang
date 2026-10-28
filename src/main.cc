#include <iostream>
#include <vector>

#include "help.hpp"
#include "lib/args.hpp"
using namespace std;

const string version = "v0.1.0";


int main(int argc, char *argv[]) {
  vector<string> flags;
  vector<string> scriptArgs;
  string scriptName;
  parse(argc, argv, flags, scriptArgs, scriptName);
  for (auto flag : flags) {
    if (flag == "--help") {
      help(version);
      exit(0);
    } else if (flag == "--ci") {
      // TODO: ci()
    }
  }
  if (scriptName.empty()) {
    cout << argv[0] << ": no script specified\n";
    cout << "Run " << argv[0] << " --help for help\n";
    exit(1);
  }
}
