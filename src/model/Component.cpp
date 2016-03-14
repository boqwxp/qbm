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
#include "Component.hpp"

#include "Context.hpp"
#include "CompDecl.hpp"
#include "Statement.hpp"

#include <sstream>
#include <functional>

Component::Component(Root &root, Instantiation const &inst)
  : m_root(root), m_inst(inst) {

  Context  ctx(root, *this);

  // define busses for all ports
  inst.decl().forAllPorts([&root,&ctx](PortDecl const &decl) {
      int const  width = ctx.computeConstant(decl.width());
      ctx.registerSignal(decl.name(), decl.direction() == PortDecl::Direction::in? root.allocateInput(width) : root.allocateSignal(width));
    });

  // execute statements
  compile(ctx);
}
Component::Component(Root &root, Instantiation const &inst,
		     std::map<std::string, int> &params,
		     std::map<std::string, Bus> &connects)
  : m_root(root), m_inst(inst) {

  Context  ctx(root, *this, params, connects);
  compile(ctx);
}

void Component::compile(Context &ctx) {
  std::cerr << "Compiling " << m_inst.label() << " ..." << std::endl;
  m_inst.decl().forAllStatements([&ctx](Statement const& stmt) {
      std::cerr << stmt << std::endl;
      stmt.execute(ctx);
    });
  std::cerr << "Compiling " << m_inst.label() << " done." << std::endl;
}

void Component::addComponent(Instantiation        const &inst,
			     std::map<std::string, int> &params,
			     std::map<std::string, Bus> &connects) {
  std::string const &label = inst.label();
  auto const  res = m_components.emplace(std::piecewise_construct,
					 std::forward_as_tuple(label),
					 std::forward_as_tuple(m_root, inst, params,connects));
  if(!res.second)  throw "Label " + label + " already defined.";
}

void Component::accept(Visitor &v) const {
  std::stringstream  bits;
  for(auto const &e : m_configs) {
    Bus const &bus = e.second;
    for(int  i = bus.width(); i-- > 0;) {
      bits << (m_root.resolve(bus[i])? '1' : '0');
    }
    v.visitConfig(e.first, bits.str());
    bits.clear();
  }

  for(auto const &e : m_components) {
    v.visitChild(e.second);
  }
}
