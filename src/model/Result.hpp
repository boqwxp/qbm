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
#ifndef RESULT_HPP
#define RESULT_HPP

extern "C" {
#  include "quantor.h"
}
#include <cassert>
#include <ostream>

/**
 * This class provides a thin C++ wrapper around a QuantorResult.
 *
 * @author Thomas B. Preußer <thomas.preusser@utexas.edu>
 */
class Result {
  ::QuantorResult  m_val;

  //- Construction / Destruction
public:
  Result() : m_val(QUANTOR_RESULT_UNKNOWN) {}
  Result(::QuantorResult  val) : m_val(val) {
    assert((0 <= val) && (val <= 40) && ((val % 10) == 0));
  }

  Result(Result const &o) : m_val(o.m_val) {}

  Result &operator=(Result const &o) {
    m_val = o.m_val;
    return *this;
  }

  ~Result() {}

  //- Utility Operators
public:
  /**
   * @brief operator bool is the utility operator version of isSatisfiable().
   */
  operator bool() const {
    return  isSatisfiable();
  }

  /**
   * @brief isSatisfiable
   * @return True if and only if this Result proved satisfyability.
   */
  bool isSatisfiable() const {
    return  m_val == QUANTOR_RESULT_SATISFIABLE;
  }

  /** Conversion to the original detailed QuantorResult. */
  operator ::QuantorResult() const {
    return   m_val;
  }

  /** Text representation of the original detailed QuantorResult. */
  operator char const* () const;
};

inline std::ostream &operator<<(std::ostream &out, Result  r) {
  out << r.operator char const*();
  return  out;
}
#endif
