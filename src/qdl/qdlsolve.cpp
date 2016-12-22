/*****************************************************************************
 * This file is part of the QBM (Quantified Binary Matching) program.
 *
 * Copyright (C) 2016
 *      Thomas B. Preusser <thomas.preusser@utexas.edu>
 *****************************************************************************
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "Lib.hpp"
#include "Root.hpp"
#include "QdlParser.hpp"

namespace {
  void usage(std::ostream &out, char const *const  prog) {
    out << '\n'<< prog <<
      " [-tTOP[<PAR0,PAR1,...>]] [-DNAME[=VALUE] ...] [-pFILE]\n\n"
      "Parse a configurable circuit description from stdin and compute an implementing\n"
      "configuration of the included user target function if it exists.\n\n"
      " TOP\tname of the top-level module defining the circuit, default: top\n"
      " PARi\tnumeric generic parameters passed to the top-level module, default: none\n"
      " NAME\tmacro definition with optional VALUE for expansion before parsing\n"
      " FILE\tprint qdimacs formulation to FILE rather than solving the problem\n"
	<< std::endl;
  }
}


int main(int const  argc, char const *const  argv[]) {

  // Default Instantiation Parameters
  std::unordered_map<std::string, std::string>  defines;    // parser defines
  std::string       top("top"); // top-level name
  std::vector<int>  generics;   // top-level params
  char const       *qdimacs = 0;


  // Extract parameters passed via the command line
  for(int  i = 1; i < argc;) {
    char const *arg = argv[i++];

    if(arg[0] == '-') {
      char const  opt = arg[1];
      if((opt == '?') || (opt == 'h')) {
	usage(std::cout, *argv);
	return  0;
      }

      // Options with additional parameters
      if(opt != '\0') {
	if(arg[2] != '\0')  arg += 2;
	else {
	  if(i < argc)  arg = argv[i++];
	  else {
	    std::cerr << "Missing parameter after '-" << opt << "'.";
	    return  1;
	  }
	}

	char     *name;
	unsigned  end = 0;
	switch(opt) {
	  // User-defined top-level module with optional generics
	case 't':
	  sscanf(arg, " %m[A-Za-z_0-9] < %n", &name, &end);
	  top = name;
	  free(name);

	  while(end) {
	    int   param;
	    char  sep;
	    arg += end;
	    end  = 0;
	    if((sscanf(arg, "%d %c %n", &param, &sep, &end) < 2) ||
	       ((sep != ',') && (sep != '>')))  goto  err;
	    generics.emplace_back(param);
	    if(sep == '>')  end = 0;
	  }
	  continue;

	  // User-defined macro definitions
	case 'D':
	  sscanf(arg, " %m[A-Za-z_0-9] = %n", &name, &end);
	  defines.emplace(name, end? arg+end : "");
	  free(name);
	  continue;

	  // Print qdimacs formulation to file
	case 'p':
	  qdimacs = arg;
	  continue;
	}
      }
    }
  err:
    std::cerr << "Cannot parse parameter: \"" << arg << '"' << std::endl;
    return  1;
  }

  // Parse and solve input from stdin
  try {
    Lib  lib;
    QdlParser(std::cin, std::move(defines), lib);
    Root  root(lib.resolveComponent(top), generics);
    //root.dumpClauses(std::cerr);

    if(qdimacs) {
      // Dump the posed problem to specified file
      std::cerr << std::endl << "Dumping problem to file '" << qdimacs << '\'' << std::endl;;

      std::ofstream  out(qdimacs);
      root.dumpQDimacs(out);
    }
    else {
      // Solve the posed problem
      std::cerr << std::endl << "Solving ... ";

      Result const  res = root.solve();
      std::cout << res << std::endl;
      if(res)  root.printConfig(std::cout);
    }
  }
  catch(char const *const  msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }
  catch(std::string const& msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }

} // main()
