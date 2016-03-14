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
#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "Bus.hpp"
#include "Statement.hpp"

#include <string>
#include <ostream>
#include <map>

class Root;
class Instantiation;
class CompDecl;
class Context;

class Component {
  Root                &m_root;
  Instantiation const &m_inst;

  std::map<std::string, Bus>        m_configs;
  std::map<std::string, Component>  m_components;
  
public:
  Component(Root &root, Instantiation const &inst);
  Component(Root &root, Instantiation const &inst,
	    std::map<std::string, int> &params,
	    std::map<std::string, Bus> &connects);
  ~Component() {}

public:
  std::string const& label() const { return  m_inst.label(); }
  CompDecl    const& type()  const { return  m_inst.decl(); }

private:
  void compile(Context &ctx);

public:
  void addConfig(std::string const &name, Bus const &bus) {
    m_configs.emplace(name, bus);
  }
  void addComponent(Instantiation        const &decl,
		    std::map<std::string, int> &params,
		    std::map<std::string, Bus> &connects);

public:
  class Visitor {
  protected:
    Visitor() {}
    ~Visitor() {}
  public:
    virtual void visitConfig(std::string const &name, std::string const &bits) = 0;
    virtual void visitChild(Component const &child) = 0;
  };
  void accept(Visitor &v) const;
};
#endif
