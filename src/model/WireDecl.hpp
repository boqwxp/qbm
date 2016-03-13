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
#ifndef WIREDECL_HPP
#define WIREDECL_HPP

#include "Decl.hpp"
#include "Expression.hpp"

#include <memory>

class WireDecl : public Decl {
  std::string                const  m_name;
  std::shared_ptr<Expression const> m_width;

public:
  WireDecl(std::string                const &name,
	   std::shared_ptr<Expression const> width)
    : m_name(name), m_width(width) {}
  ~WireDecl();

public:
  void dump(std::ostream &out) const;

public:
  std::string const& name()  const { return  m_name; }
  Expression  const& width() const { return *m_width; }
};
#endif
