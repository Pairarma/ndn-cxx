/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2022 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef NDN_CXX_SECURITY_TRANSFORM_STEP_SOURCE_HPP
#define NDN_CXX_SECURITY_TRANSFORM_STEP_SOURCE_HPP

#include "ndn-cxx/security/transform/transform-base.hpp"

namespace ndn {
namespace security {
namespace transform {

/**
 * @brief A source that can accept input step by step, and can close input explicitly.
 *
 * This source will not send data into the transformation chain when the chain is constructed.
 * Input will be explicitly sent into the chain using write(...) and will be closed explicitly
 * using end().
 *
 *   StepSource ss;
 *   ss >> transform1() >> transform2() >> sinkStream(...);
 *   ss.write(...);
 *   ...
 *   ss.write(...);
 *   ss.end();
 */
class StepSource : public Source
{
public:
  /**
   * @brief Accept input data and directly write input into next transformation module.
   *
   * One can keep calling this method to until end() is called, which
   * indicates the end of input.  After that, calling this method will cause Error.
   *
   * @return Number of bytes that have been written into the next module.
   */
  size_t
  write(span<const uint8_t> buf);

  /**
   * @brief Close the input interface and directly notify the next module the end of input.
   */
  void
  end();

private:
  /**
   * @brief This method intentionally does nothing.
   *
   * Use write() and end() explicitly to submit data.
   */
  void
  doPump() final;
};

using stepSource = StepSource;

} // namespace transform
} // namespace security
} // namespace ndn

#endif // NDN_CXX_SECURITY_TRANSFORM_STEP_SOURCE_HPP
