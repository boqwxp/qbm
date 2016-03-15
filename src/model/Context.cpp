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
#include "Context.hpp"

#include "CompDecl.hpp"
#include <cmath>

namespace {
  class Computer : public Expression::Visitor {
    Context const &m_ctx;
  public:
    int m_val;

  public:
    Computer(Context const &ctx) : m_ctx(ctx) {}
    ~Computer() {}

  public:
    void visit(ConstExpression const &expr) {
      m_val = expr.value();
    }
    void visit(NameExpression const &expr) {
      m_val = m_ctx.resolveConstant(expr.name());
    }
    void visit(UniExpression const &expr) {
      expr.arg().accept(*this);
      switch(expr.op()) {
      case UniExpression::Op::NOT: m_val = ~m_val; break;
      case UniExpression::Op::NEG: m_val = -m_val; break;
      case UniExpression::Op::LD: {
	unsigned  val = 0;
	for(unsigned  r = m_val-1; r != 0; r >>= 1)  val++;
	m_val = val;
	break;
      }
      default: throw "Unsupported Operation.";
      }
    }
    void visit(BiExpression const &expr) {
      expr.lhs().accept(*this);
      int const  lhs = m_val;
      expr.rhs().accept(*this);
      int const  rhs = m_val;
      switch(expr.op()) {
      case BiExpression::Op::ADD: m_val = lhs + rhs; break;
      case BiExpression::Op::SUB: m_val = lhs - rhs; break;
      case BiExpression::Op::MUL: m_val = lhs * rhs; break;
      case BiExpression::Op::DIV: m_val = lhs / rhs; break;
      case BiExpression::Op::MOD: m_val = lhs % rhs; break;
      case BiExpression::Op::POW: m_val = (int)roundl(pow(lhs, rhs)); break;
      default: throw "Unsupported Operation.";
      }
    }
    void visit(CondExpression const &expr) {
      expr.cond().accept(*this);
      if(m_val)  expr.pos().accept(*this);
      else 	 expr.neg().accept(*this);
    }
    void visit(RangeExpression const &expr) {
      expr.base().accept(*this);
      unsigned const  base = m_val;
      expr.left().accept(*this);
      int const  left = m_val;
      expr.right().accept(*this);
      int const  right = m_val;
      m_val = (left < right)? 0 : (base >> right) & ((1u<<(left-right+1))-1);
    }
  }; // class Computer

  class Builder : public Expression::Visitor {
    Context &m_ctx;
  public:
    Bus  m_val;

  public:
    Builder(Context &ctx) : m_ctx(ctx) {}
    ~Builder() {}

  public:
    void visit(ConstExpression const &expr) override {
      m_val = Bus(expr.value());
    }
    void visit(NameExpression const &expr) override {
      m_val = m_ctx.resolveBus(expr.name());
    }
    void visit(UniExpression const &expr) override {
      switch(expr.op()) {
      case UniExpression::Op::NOT:
	m_val = ~m_val;
	break;

      default:
	throw "Unsupported Operation";
      }
    }
    void visit(BiExpression const &expr) override {
      expr.lhs().accept(*this);
      Bus const  lhs = m_val;
      expr.rhs().accept(*this);
      Bus const  rhs = m_val;

      std::function<void(Node, Node, Node)>  op;
      switch(expr.op()) {
      case BiExpression::Op::AND:
	op = [this](Node y, Node a, Node b) {
	  m_ctx.addClause(y, -a, -b);
	  m_ctx.addClause(-y, a);
	  m_ctx.addClause(-y, b);
	};
	break;

      case BiExpression::Op::OR:
	op = [this](Node y, Node a, Node b) {
	  m_ctx.addClause(-y, a, b);
	  m_ctx.addClause(y, -a);
	  m_ctx.addClause(y, -b);
	};
	break;

      case BiExpression::Op::XOR:
	op = [this](Node y, Node a, Node b) {
	  m_ctx.addClause(-y, -a, -b);
	  m_ctx.addClause(-y,  a,  b);
	  m_ctx.addClause( y, -a,  b);
	  m_ctx.addClause( y,  a, -b);
	};
	break;

      case BiExpression::Op::SEL: {
	m_val = m_ctx.allocateSignal(1);
	Node     const  y = m_val[0];
	unsigned const  range = lhs.width();

	unsigned  width = 0;
	for(unsigned  r = range-1; r != 0; r >>= 1)  width++;
	std::unique_ptr<int[]>  clause(new int[width + 2]);

	// Select appropriate wire from lhs
	for(unsigned  line = 0; line < range; line++) {
	  for(unsigned  i = 0; i < width; i++) {
	    clause[i] = (line & (1<<i)) != 0? -rhs[i] : (unsigned)rhs[i];
	  }
	  clause[width]   =  lhs[line];
	  clause[width+1] = -y;
	  m_ctx.addClause(clause.get(), clause.get()+width+2);
	  clause[width]   = -lhs[line];
	  clause[width+1] =  y;
	  m_ctx.addClause(clause.get(), clause.get()+width+2);
	}

	// Disallow rhs selector values beyond the index range of lhs
	for(unsigned  line = range; line < (1u << width); line++) {
	  for(unsigned  i = 0; i < width; i++) {
	    clause[i] = (line & (1<<i)) != 0? -rhs[i] : (unsigned)rhs[i];
	  }
	  m_ctx.addClause(clause.get(), clause.get()+width);
	}

	// Tie unused selector bits to BOT
	for(unsigned  i = width; i < rhs.width(); i++) {
	  m_ctx.addClause(-rhs[i]);
	}
	return;
      }
      default:
	throw "Unsupported Operation";
      }

      Bus const  res = m_ctx.allocateSignal(std::max(lhs.width(), rhs.width()));
      for(unsigned  i = res.width(); i-- > 0;) 	op(res[i], lhs[i], rhs[i]);
      m_val = res;
      return;
    }
    void visit(CondExpression const &expr) override {
      expr.cond().accept(*this);
      Bus const  cond = m_val;
      expr.pos().accept(*this);
      Bus const  pos  = m_val;
      expr.neg().accept(*this);
      Bus const  neg  = m_val;

      Bus const  res = m_ctx.allocateSignal(std::max(std::max(cond.width(), pos.width()), neg.width()));
      for(unsigned  i = res.width(); i-- > 0;) {
	m_ctx.addClause(-cond[i], -pos[i],  res[i]);
	m_ctx.addClause(-cond[i],  pos[i], -res[i]);
	m_ctx.addClause( cond[i], -neg[i],  res[i]);
	m_ctx.addClause( cond[i],  neg[i], -res[i]);
      }
      m_val = res;
    }
    void visit(RangeExpression const &expr) override {
      Computer  comp(m_ctx);
      expr.left().accept(comp);
      unsigned const  left = comp.m_val;
      expr.right().accept(comp);
      unsigned const  right = comp.m_val;

      expr.base().accept(*this);
      m_val = m_val(left, right);
    }
  }; // class Builder
}

void Context::defineConstant(std::string const &name, int val) {
  if(!m_constants.emplace(name, val).second) {
    throw "Constant " + name + " already defined.";
  }
}
int Context::resolveConstant(std::string const &name) const {
  auto const  it = m_constants.find(name);
  if(it != m_constants.end())  return  it->second;
  throw  m_comp.type().name() + ": " + name + " is not defined.";
}
int Context::computeConstant(Expression const &expr) const {
  Computer  comp(*this);
  expr.accept(comp);
  return  comp.m_val;
}

void Context::registerConfig(std::string const &name, Bus const &bus) {
  registerSignal(name, bus);
  m_comp.addConfig(name, bus);
}
void Context::registerSignal(std::string const &name, Bus const &bus) {
  if(!m_busses.emplace(name, bus).second) {
    throw "Bus " + name + " already defined.";
  }
}

Bus Context::resolveBus(std::string const &name) const {
  { // Name of physical bus?
    auto const  it = m_busses.find(name);
    if(it != m_busses.end())  return  it->second;
  }
  // Try a constant ...
  return  Bus(resolveConstant(name));
}

Bus Context::computeBus(Expression const &expr) {
  Builder  bld(*this);
  expr.accept(bld);
  return  bld.m_val;
}