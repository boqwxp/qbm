#include <iostream>
#include <string>
#include <unordered_map>

#include "Lib.hpp"
#include "Root.hpp"
#include "QdlParser.hpp"

int main(int const  argc, char const *const  argv[]) {

  // Extract defines passed via the command line
  std::unordered_map<std::string, std::string>  defines;
  for(int  i = 1; i < argc; i++) {
    char const *const  arg = argv[i];

    char     *name;
    unsigned  end = 0;
    sscanf(arg, " %m[A-Za-z_0-9] = %n", &name, &end);
    defines.emplace(name, end? arg+end : "");
    free(name);
  }

  // Parse and solve input from stdin
  try {
    Lib  lib;
    QdlParser(std::cin, std::move(defines), lib);
    std::unique_ptr<Root>  root(lib.compile("top"));
    //root->dumpClauses(std::cout);
    std::cerr << std::endl << "Solving ... ";

    Result const  res = root->solve();
    std::cout << res << std::endl;
    if(res)  root->printConfig(std::cout);
  }
  catch(char const *const  msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }
  catch(std::string const& msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }

} // main()
