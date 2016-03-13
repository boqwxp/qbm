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

#include "Context.hpp"
#include "Bus.hpp"

#include <string>
#include <map>

class Instantiation;
class Expression;
class Root;

class Component : public Context {
  Context             &m_parent;
  Instantiation const &m_inst;
  
  std::map<std::string, int>        m_constants;
  std::map<std::string, Bus>        m_busses;
  std::map<std::string, Component>  m_components;
  
public:
  Component(Root      &parent, Instantiation const &inst);
  Component(Component &parent, Instantiation const &inst,
	    std::map<std::string, int> &params,
	    std::map<std::string, Bus> &connects);
  ~Component() {}

private:
  void compile();

public:
  Bus allocateConfig(unsigned  width) override;
  Bus allocateInput (unsigned  width) override;
  Bus allocateSignal(unsigned  width) override;

public:
  void addClause(int const *beg, int const *end) override;
  using Context::addClause;
public:
  void defineConstant(std::string const &name, int val);
  int resolveConstant(std::string const &name) const;
  int computeConstant(Expression  const &name) const;

  void registerBus(std::string const &name, Bus const &bus);
  Bus resolveBus(std::string const &name) const;
  Bus computeBus(Expression  const &name);

public:
  void addComponent(Instantiation        const &decl,
		    std::map<std::string, int> &params,
		    std::map<std::string, Bus> &connects);
};
#endif
