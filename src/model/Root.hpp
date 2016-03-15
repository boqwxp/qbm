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
#ifndef ROOT_HPP
#define ROOT_HPP

#include "Bus.hpp"
#include "CompDecl.hpp"
#include "Component.hpp"
#include "Statement.hpp"
#include "Result.hpp"

#include <vector>

class Root {
public:
  static int const  FIRST_CONFIG =          2;
  static int const  FIRST_INPUT  = 0x3FFF0000;
  static int const  FIRST_SIGNAL = 0x40000000;

private:
  std::vector<int>  m_clauses;
  int  m_confignxt;
  int  m_inputnxt;
  int  m_signalnxt;

  Instantiation const  m_inst;
  Component     const  m_top;

  Result  m_res;

public:
  Root(CompDecl const &decl)
    : m_confignxt(FIRST_CONFIG),
      m_inputnxt (FIRST_INPUT),
      m_signalnxt(FIRST_SIGNAL),
      m_inst("<top>", decl), m_top(*this, m_inst) {}
  ~Root() {}

public:
  Bus allocateConfig(unsigned  width);
  Bus allocateInput (unsigned  width);
  Bus allocateSignal(unsigned  width);

public:
  void print(std::ostream &out, Bus const &bus) const;
  void addClause(int const *beg, int const *end);
  void dumpClauses(std::ostream &out) const;

public:
  Component const& top() const { return  m_top; }
  Result solve();
  bool resolve(int const  v) const {
    for(int  i : m_clauses) {
      if(abs(i) == v)  return  i > 0;
    }
    throw "Config variable not found.";
  }
  void printConfig(std::ostream &out) const;
};
#endif
