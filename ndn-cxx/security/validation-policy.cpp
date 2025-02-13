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

#include "ndn-cxx/security/validation-policy.hpp"
#include "ndn-cxx/security/signing-info.hpp"

namespace ndn {
namespace security {
inline namespace v2 {

void
ValidationPolicy::setInnerPolicy(unique_ptr<ValidationPolicy> innerPolicy)
{
  if (innerPolicy == nullptr) {
    NDN_THROW(std::invalid_argument("Inner policy argument cannot be nullptr"));
  }

  if (m_validator != nullptr) {
    innerPolicy->setValidator(*m_validator);
  }

  if (m_innerPolicy == nullptr) {
    m_innerPolicy = std::move(innerPolicy);
  }
  else {
    m_innerPolicy->setInnerPolicy(std::move(innerPolicy));
  }
}

ValidationPolicy&
ValidationPolicy::getInnerPolicy()
{
  return *m_innerPolicy;
}

void
ValidationPolicy::setValidator(Validator& validator)
{
  m_validator = &validator;
  if (m_innerPolicy != nullptr) {
    m_innerPolicy->setValidator(validator);
  }
}

Name
getKeyLocatorName(const SignatureInfo& si, ValidationState& state)
{
  if (si.getSignatureType() == tlv::DigestSha256) {
    return SigningInfo::getDigestSha256Identity();
  }

  if (!si.hasKeyLocator()) {
    state.fail({ValidationError::INVALID_KEY_LOCATOR, "KeyLocator is missing"});
    return {};
  }

  const KeyLocator& kl = si.getKeyLocator();
  if (kl.getType() != tlv::Name) {
    state.fail({ValidationError::INVALID_KEY_LOCATOR, "KeyLocator type is not Name"});
    return {};
  }

  return kl.getName();
}

SignatureInfo
getSignatureInfo(const Interest& interest, ValidationState& state)
{
  auto fmt = state.getTag<SignedInterestFormatTag>();
  BOOST_ASSERT(fmt);

  if (*fmt == SignedInterestFormat::V03) {
    BOOST_ASSERT(interest.getSignatureInfo().has_value());
    return *interest.getSignatureInfo();
  }

  // Try the old Signed Interest format from Packet Specification v0.2
  const Name& name = interest.getName();
  if (name.size() < signed_interest::MIN_SIZE) {
    state.fail({ValidationError::MALFORMED_SIGNATURE,
                "Interest name too short `" + name.toUri() + "`"});
    return {};
  }

  try {
    return SignatureInfo(name[signed_interest::POS_SIG_INFO].blockFromValue());
  }
  catch (const tlv::Error& e) {
    state.fail({ValidationError::MALFORMED_SIGNATURE,
                "Malformed SignatureInfo in `" + name.toUri() + "`: " + e.what()});
    return {};
  }
}

Name
extractIdentityNameFromKeyLocator(const Name& keyLocator)
{
  // handling special cases
  if (keyLocator == SigningInfo::getDigestSha256Identity() ||
      keyLocator == SigningInfo::getHmacIdentity()) {
    return keyLocator;
  }

  auto len = static_cast<ssize_t>(keyLocator.size());
  // note that KEY_COMPONENT_OFFSET is negative
  auto lowerBound = std::max<ssize_t>(len + Certificate::KEY_COMPONENT_OFFSET, 0);
  for (ssize_t i = len - 1; i >= lowerBound; --i) {
    if (keyLocator[i] == Certificate::KEY_COMPONENT) {
      return keyLocator.getPrefix(i);
    }
  }

  NDN_THROW(KeyLocator::Error("KeyLocator `" + keyLocator.toUri() +
                              "` does not respect the naming conventions"));
}

} // inline namespace v2
} // namespace security
} // namespace ndn
