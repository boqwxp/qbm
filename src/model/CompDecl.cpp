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
#include "CompDecl.hpp"
#include "Statement.hpp"

CompDecl::~CompDecl() {}
void CompDecl::dump(std::ostream &out) const {
  out << "component " << m_name;
  if(!m_params.empty()) {
    auto  it = m_params.begin();
    out << '<';
    while(true) {
      out << *it++;
      if(it != m_params.end()) {
	out << ", ";
	continue;
      }
      out << '>';
      break;
    }
  }
  out << '(';
  char const *sep = "";
  for(auto const &port : m_ports) {
    if(port.direction() == PortDecl::Direction::out)  sep = " -> ";
    out << sep << port.name() << '[' << port.width() << ']';
    sep = ", ";
  }
  out << ")\n";
  for(auto const &stmt : m_statements)  out << "  " << *stmt << ";\n";
  out << "end;\n";
}
