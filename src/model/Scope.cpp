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
#include "Scope.hpp"

Scope::~Scope() {}

Scope &Scope::createChild(std::string const &name) {
  auto  res = m_children.emplace(std::piecewise_construct,
				 std::forward_as_tuple(name),
				 std::forward_as_tuple(name));
  if(!res.second)  throw name + " already defined.";
  return  res.first->second;
}

void Scope::accept(Visitor &v) const {
  for(auto const &e : m_configs) {
    v.visitConfig(e.first, e.second);
  }

  for(auto const &e : m_children) {
    v.visitChild(e.first, e.second);
  }
}
