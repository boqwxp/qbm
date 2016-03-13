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

void Root::addClause(int const *beg, int const *end) {
  while(beg < end) {
#ifdef DEBUG
    std::cerr << *beg << ' ';
#endif
    m_clauses.push_back(*beg++);
  }
#ifdef DEBUG
  std::cerr << std::endl;
#endif
  m_clauses.push_back(0);
}
