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

class Context {
protected:
  Context() {}
  ~Context() {}
public:
  virtual Bus allocateConfig(unsigned  width) = 0;
  virtual Bus allocateInput (unsigned  width) = 0;
  virtual Bus allocateSignal(unsigned  width) = 0;

public:
  virtual void addClause(int const *beg, int const *end) = 0;
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
};
#endif
