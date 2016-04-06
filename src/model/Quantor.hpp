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
#ifndef QUANTOR_HPP
#define QUANTOR_HPP

extern "C" {
#  include "quantor.h"
}

#include "Result.hpp"

namespace qbm {
/**
 * This class provides a thin C++ wrapper around the quantor C-library.
 *
 * @author Thomas B. Preußer <thomas.preusser@utexas.edu>
 */
class Quantor {
  ::Quantor* const  quantor;

  //- Construction / Destruction
public:
  Quantor() : quantor(quantor_new()) {}
  ~Quantor() { quantor_delete(quantor); }

  //- Information
public:
  char const* id()        const {
    return  quantor_id();
  }
  char const* copyright() const {
    return  quantor_copyright();
  }
  char const* version()   const {
    return  quantor_version();
  }
  char const* backend()   const {
    return  quantor_backend();
  }

  //- Problem Construction
public:
  char const* scope(::QuantorQuantificationType const  quant) {
    return  quantor_scope(quantor, quant);
  }
  char const* add(int const  lit) {
    return  quantor_add(quantor, lit);
  }

  //- Solving / Result Retrieval
public:
  Result sat() {
    return  quantor_sat(quantor);
  }
  int const* assignment() const {
    return  quantor_assignment(quantor);
  }
};
}
#endif
