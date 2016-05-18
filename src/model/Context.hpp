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
#include "CompDecl.hpp"
#include "Statement.hpp"

#include <map>

class Expression;

class Context {
  Root  &m_root;
  Scope &m_scope;

  std::map<std::string, int>  m_constants;
  std::map<std::string, Bus>  m_busses;

  unsigned     m_subcnt;

public:
  Context(Root &root, Scope &scope) : m_root(root), m_scope(scope), m_subcnt(0) {}
  Context(Root &root, Scope &scope,
	  std::map<std::string, int> &constants,
	  std::map<std::string, Bus> &busses)
    : m_root(root), m_scope(scope), m_subcnt(0) {
    std::swap(m_constants, constants);
    std::swap(m_busses,    busses);
  }
  Context(Context const &parent, Scope &scope)
    : m_root(parent.m_root), m_scope(scope),
      m_constants(parent.m_constants), m_busses(parent.m_busses), m_subcnt(0) {}
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
  void compile(std::string const &name, CompDecl const &comp) {
    std::cout << "Compiling " << name << " : " << comp.name() << " ..." << std::endl;
    comp.forAllStatements([this](Statement const &stmt) { stmt.execute(*this); });
  }
  Scope& createChildScope(std::string const &name) {
    return  m_scope.createChild(name);
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
