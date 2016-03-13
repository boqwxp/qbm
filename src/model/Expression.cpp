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
#include "Expression.hpp"

ConstExpression::~ConstExpression() {}
void ConstExpression::accept(Visitor &vis) const { vis.visit(*this); }

NameExpression::~NameExpression() {}
void NameExpression::accept(Visitor &vis) const { vis.visit(*this); }

BiExpression::~BiExpression() {}
void BiExpression::accept(Visitor &vis) const { vis.visit(*this); }
std::array<char const *const, 11>  BiExpression::OPS = {
  "&", "|", "^", "+", "-", "*", "/", "%", "**", "@", "??"
};

void ExpressionPrinter::visit(ConstExpression const &expr) {
  m_out << expr.value();
}
void ExpressionPrinter::visit(NameExpression const &expr) {
  m_out << expr.name();
}
void ExpressionPrinter::visit(BiExpression const &expr) {
  m_out << '(';
  expr.lhs().accept(*this);
  m_out << ')' << BiExpression::OPS[std::min((size_t)expr.op(), BiExpression::OPS.size()-1)] << '(';
  expr.rhs().accept(*this);
  m_out << ')';
}
