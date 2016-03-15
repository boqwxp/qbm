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
#ifndef LIB_HPP
#define LIB_HPP

#include "CompDecl.hpp"

#include <string>
#include <map>
#include <memory>

class CompDecl;
class Component;
class Root;

class Lib {
  std::map<std::string, CompDecl>  m_components;

public:
  Lib() {}
  ~Lib() {}

public:
  CompDecl& declareComponent(std::string const &name);
  CompDecl const& resolveComponent(std::string const &name) const {
    return  m_components.at(name);
  }
  Root* compile(std::string const &top);
};
#endif
