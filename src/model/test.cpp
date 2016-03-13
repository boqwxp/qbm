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
#include "Lib.hpp"
#include "CompDecl.hpp"

#include "Statement.hpp"

int main(int const  argc, char const *const  argv[]) {
  Lib  lib;
  try {
    CompDecl &lut = lib.declareComponent("lut");
    lut.addParameter("K");
    lut.addPort(PortDecl::Direction::in,  "x", std::make_shared<NameExpression>("K"));
    lut.addPort(PortDecl::Direction::out, "y", std::make_shared<ConstExpression>(1));
    lut.addStatement(std::make_shared<ConstDecl>("N", std::make_shared<BiExpression>(BiExpression::Op::POW, std::make_shared<ConstExpression>(2), std::make_shared<NameExpression>("K"))));
    lut.addStatement(std::make_shared<ConfigDecl>("c", std::make_shared<NameExpression>("N")));
    lut.addStatement(std::make_shared<Equation>(std::make_shared<NameExpression>("y"), std::make_shared<NameExpression>("K")));

    CompDecl &top = lib.declareComponent("top");
    top.addPort(PortDecl::Direction::in,  "x", std::make_shared<ConstExpression>(4));
    top.addStatement(std::make_shared<SignalDecl>("y", std::make_shared<ConstExpression>(1)));

    std::shared_ptr<Instantiation>  inst(std::make_shared<Instantiation>("label", lut));
    inst->addParameter(std::make_shared<ConstExpression>(4));
    inst->addConnection(std::make_shared<NameExpression>("x"));
    inst->addConnection(std::make_shared<NameExpression>("y"));
    top.addStatement(inst);
  
    std::cout << lut << std::endl;
    std::cout << top << std::endl;

    lib.compile("top");
  }
  catch(char const *const  msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }
  catch(std::string const& msg) {
    std::cerr << "Error:\n\t" << msg << std::endl;
  }
}
