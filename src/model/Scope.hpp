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
#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "Bus.hpp"

#include <string>
#include <map>

class Scope {
  std::string const             m_name;
  std::map<std::string, Bus>    m_configs;
  std::map<std::string, Scope>  m_children;

public:
  Scope(std::string const &name) : m_name(name) {}
  ~Scope();

public:
  std::string const& name() const { return  m_name; }

public:
  void addConfig(std::string const &name, Bus const &bus) {
    m_configs.emplace(name, bus);
  }
  Scope &createChild(std::string const &name);

public:
  class Visitor {
  protected:
    Visitor() {}
    ~Visitor() {}
  public:
    virtual void visitConfig(std::string const &name, Bus const &bus) = 0;
    virtual void visitChild(std::string const &name, Scope const &child) = 0;
  };
  void accept(Visitor &v) const;
};
#endif
