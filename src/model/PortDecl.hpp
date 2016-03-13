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
#ifndef PORTDECL_HPP
#define PORTDECL_HPP

#include "WireDecl.hpp"

class PortDecl : public WireDecl {
public:
  enum class Direction { in, out };
private:
  Direction const  m_dir;

public:
  PortDecl(Direction                  const  dir,
	   std::string                const &name,
	   std::shared_ptr<Expression const> width)
    : WireDecl(name, width), m_dir(dir) {}
  ~PortDecl();

public:
  void dump(std::ostream &out) const;

public:
  Direction direction() const { return  m_dir; }
};
#endif
