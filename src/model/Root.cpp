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
#include "Root.hpp"
#include "Context.hpp"

#include "Quantor.hpp"

#include <iostream>
#include <sstream>

Root::Root(CompDecl const &decl)
  : m_top(""),
    m_confignxt(FIRST_CONFIG),
    m_inputnxt (FIRST_INPUT),
    m_signalnxt(FIRST_SIGNAL) {

  Context  ctx(*this, m_top);
  decl.forAllPorts([this, &ctx](PortDecl const &decl) {
      int const  width = ctx.computeConstant(decl.width());
      ctx.registerSignal(decl.name(), decl.direction() == PortDecl::Direction::in? allocateInput(width) : allocateSignal(width));
    });
  ctx.compile("<top>", decl);
}

Bus Root::allocateConfig(unsigned  width) {
  Node *const  nodes = new Node[width];
  for(unsigned  i = 0; i < width; i++) {
    nodes[i] = m_confignxt++;
  }
  return  Bus(width, nodes);
}

Bus Root::allocateInput (unsigned  width) {
  Node *const  nodes = new Node[width];
  for(unsigned  i = 0; i < width; i++) {
    nodes[i] = m_inputnxt++;
  }
  return  Bus(width, nodes);
}

Bus Root::allocateSignal(unsigned  width) {
  Node *const  nodes = new Node[width];
  for(unsigned  i = 0; i < width; i++) {
    nodes[i] = m_signalnxt++;
  }
  return  Bus(width, nodes);
}

namespace {
  class Lit {
    int  m_val;
  public:
    Lit(int  val) : m_val(val) {}
    ~Lit() {}
  public:
    operator int() const { return  m_val; }
    operator int&()      { return  m_val; }
  };
  std::ostream& operator<<(std::ostream& out, Lit  lit) {
    int   v = static_cast<int>(lit);
    char  c = 'X'; // this should not stick!
    if(v < 0) {
      out << '~';
      v = -v;
    }

    if(v >= Root::FIRST_SIGNAL) {
      c = 'n';
      v -= Root::FIRST_SIGNAL;
    }
    else if(v >= Root::FIRST_INPUT) {
      c = 'i';
      v -= Root::FIRST_INPUT;
    }
    else if(v >= Root::FIRST_CONFIG) {
      c = 'c';
      v -= Root::FIRST_CONFIG;
    }
    return  out << c << v;
  }
}

void Root::print(std::ostream &out, Bus const &bus) const {
  for(unsigned  i = bus.width(); i-- > 0;)  out << Lit(bus[i]) << ' ';
  out << std::endl;
}

void Root::addClause(int const *beg, int const *end) {
  auto const  size = m_clauses.size();
  while(beg < end) {
    int const v = *beg++;
    switch(v) {
    default:
      m_clauses.push_back(v);
    case Node::BOT:
      continue;
    case Node::TOP:
      // Rollback to remove already satisfied clause
      m_clauses.resize(size);
      return;
    }
  }
  m_clauses.push_back(0);
}

void Root::dumpClauses(std::ostream &out) const {
  for(int  v : m_clauses) {
    if(v == 0)  out << std::endl;
    else  out << Lit(v) << ' ';
  }
}

Result Root::solve() {
  if(m_res != QUANTOR_RESULT_UNKNOWN)  return  m_res;

  // Compute displacements to compact range of variables
  int const  delta_input  =  FIRST_INPUT  - m_confignxt;
  int const  delta_signal = (FIRST_SIGNAL - m_inputnxt) + delta_input;
  auto const  compact = [delta_input, delta_signal](int const  v) -> int {
    int  vv = std::abs(v);
    if(vv >= FIRST_SIGNAL)      vv -= delta_signal;
    else if(vv >= FIRST_INPUT)  vv -= delta_input;
    return  v < 0? -vv : vv;
  };

  qbm::Quantor  q;
  std::cout << "using Quantor_" << q.version() << " / " << q.backend() << std::endl;

  q.scope(QUANTOR_EXISTENTIAL_VARIABLE_TYPE);
  for(int  i = FIRST_CONFIG; i < m_confignxt; i++)  q.add(i);
  q.add(0);

  q.scope(QUANTOR_UNIVERSAL_VARIABLE_TYPE);
  for(int  i = FIRST_INPUT; i < m_inputnxt; i++)  q.add(i - delta_input);
  q.add(0);

  q.scope(QUANTOR_EXISTENTIAL_VARIABLE_TYPE);
  for(int  i = FIRST_SIGNAL; i < m_signalnxt; i++)  q.add(i - delta_signal);
  q.add(0);

  for(int lit : m_clauses) q.add(compact(lit));
  m_clauses.clear();

  m_res = q.sat();
  if(m_res) {
    int const *asgn = q.assignment();
    while(true) {
      int const  v = *asgn++;
      if(v == 0)  break;
      m_clauses.push_back(v);
    }
  }
  return  m_res;
}

void Root::printConfig(std::ostream &out) const {
  class Printer : public Scope::Visitor {
    Root const   &m_root;
    std::ostream &m_out;
    std::string   m_path;

  public:
    Printer(Root const &root, std::ostream &out) : m_root(root), m_out(out) {}
    ~Printer() {}

  public:
    void visitConfig(std::string const &name, Bus const &bus) {
      std::stringstream  s;
      for(unsigned  i = bus.width(); i-- > 0; s << m_root.resolve(bus[i]));
      m_out << m_path << name << " = \"" << s.str() << "\";" << std::endl;
    }
    void visitChild(std::string const &name, Scope const &child) {
      std::string const  prev = m_path;
      m_path += child.name();
      child.accept(*this);
      m_path = prev;
    }
  } prn(*this, out);
  m_top.accept(prn);
}
