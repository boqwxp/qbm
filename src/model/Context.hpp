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
#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "Bus.hpp"
#include "Root.hpp"

#include <map>
#include <sstream>

class Component;

class Context {
  Root      &m_root;
  Component &m_comp;

  std::map<std::string, int>  m_constants;
  std::map<std::string, Bus>  m_busses;

  std::string  m_prefix;
  unsigned     m_subcnt;

public:
  Context(Root &root, Component &comp) : m_root(root), m_comp(comp), m_subcnt(0) {}
  Context(Root &root, Component &comp,
	  std::map<std::string, int> &constants,
	  std::map<std::string, Bus> &busses)
    : m_root(root), m_comp(comp), m_subcnt(0) {
    std::swap(m_constants, constants);
    std::swap(m_busses,    busses);
  }
  Context(Context &parent)
    : m_root(parent.m_root), m_comp(parent.m_comp),
      m_constants(parent.m_constants), m_busses(parent.m_busses),
      m_subcnt(0) {
    std::stringstream  s;
    s << parent.m_prefix << parent.m_subcnt++ << '.';
    m_prefix = s.str();
  }
  ~Context() {}

public:
  Root& root() { return  m_root; }

public:
  Bus allocateConfig(unsigned  width) { return  m_root.allocateConfig(width); }
  Bus allocateInput (unsigned  width) { return  m_root.allocateInput (width); }
  Bus allocateSignal(unsigned  width) { return  m_root.allocateSignal(width); }

public:
  void addClause(int const *beg, int const *end) { m_root.addClause(beg, end); }
  void addClause(int a) {
    std::array<int const, 1>  clause{a};
    addClause(clause.begin(), clause.end());
  }
  void addClause(int a, int b) {
    std::array<int const, 2>  clause{a, b};
    addClause(clause.begin(), clause.end());
  }
  void addClause(int a, int b, int c) {
    std::array<int const, 3>  clause{a, b, c};
    addClause(clause.begin(), clause.end());
  }

public:
  void addComponent(Instantiation        const &decl,
		    std::map<std::string, int> &params,
		    std::map<std::string, Bus> &connects) {
    m_comp.addComponent(m_prefix+decl.label(), decl, params, connects);
  }

public:
  void defineConstant(std::string const &name, int val);
  int resolveConstant(std::string const &name) const;
  int computeConstant(Expression  const &name) const;

  void registerConfig(std::string const &name, Bus const &bus);
  void registerSignal(std::string const &name, Bus const &bus);
  Bus resolveBus(std::string const &name) const;
  Bus computeBus(Expression  const &name);
};
#endif
