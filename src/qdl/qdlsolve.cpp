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

#include "Lib.hpp"
#include "Root.hpp"
#include "QdlParser.hpp"

int main(int const  argc, char const *const  argv[]) {

  // Default Instantiation Parameters
  std::unordered_map<std::string, std::string>  defines;    // parser defines
  std::string       top("top"); // top-level name
  std::vector<int>  generics;   // top-level params


  // Extract parameters passed via the command line
  for(int  i = 1; i < argc;) {
    char const *arg = argv[i++];

    if(arg[0] == '-') {
      char const  opt = arg[1];
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

	case 'D':
	  sscanf(arg, " %m[A-Za-z_0-9] = %n", &name, &end);
	  defines.emplace(name, end? arg+end : "");
	  free(name);
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

    //root.dumpClauses(std::cout);
    std::cerr << std::endl << "Solving ... ";

    Result const  res = root.solve();
    std::cout << res << std::endl;
    if(res)  root.printConfig(std::cout);
  }
  catch(char const *const  msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }
  catch(std::string const& msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }

} // main()
