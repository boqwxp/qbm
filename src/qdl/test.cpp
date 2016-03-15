#include <iostream>

#include "Lib.hpp"
#include "Root.hpp"
#include "QdlParser.hpp"

int main() {
  try {
    Lib  lib;
    QdlParser(std::cin, lib);
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
}
