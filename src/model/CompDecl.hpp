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
#ifndef COMPDECL_HPP
#define COMPDECL_HPP

#include "ParamDecl.hpp"
#include "PortDecl.hpp"

#include <vector>
#include <functional>

class Statement;
class CompDecl : public Decl {
  std::string const       m_name;
  std::vector<ParamDecl>  m_params;
  std::vector<PortDecl>   m_ports;

  std::vector<std::shared_ptr<Statement const>>  m_statements;

public:
  CompDecl(std::string const &name) : m_name(name) {}
  ~CompDecl();

public:
  std::string const& name() const { return  m_name; }
  void dump(std::ostream &out) const;

public:
  void addParameter(std::string const& param) {
    m_params.emplace_back(param);
  }
  unsigned countParameters() const {
    return  m_params.size();
  }
  ParamDecl const& getParameter(unsigned const  idx) const {
    return  m_params[idx];
  }
  void forAllParameters(std::function<void(ParamDecl const&)> f) const {
    for(ParamDecl const &decl : m_params)  f(decl);
  }
  
  void addPort(PortDecl::Direction        const  dir,
	       std::string                const& name,
	       std::shared_ptr<Expression const> width) {
    m_ports.emplace_back(dir, name, width);
  }
  unsigned countPorts() const {
    return  m_ports.size();
  }
  PortDecl const& getPort(unsigned const  idx) const {
    return  m_ports[idx];
  }
  void forAllPorts(std::function<void(PortDecl const&)> f) const {
    for(PortDecl const &decl : m_ports)  f(decl);
  }

  void addStatement(std::shared_ptr<Statement const> stmt) {
    m_statements.emplace_back(stmt);
  }
  void forAllStatements(std::function<void(Statement const&)> f) const {
    for(auto const &stmt : m_statements)  f(*stmt);
  }
};
#endif
