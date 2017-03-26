/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2017 Regents of the University of California.
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

#ifndef NDN_UTIL_NETWORK_MONITOR_IMPL_OSX_HPP
#define NDN_UTIL_NETWORK_MONITOR_IMPL_OSX_HPP

#include "ndn-cxx-config.hpp"
#include "../network-monitor.hpp"

#ifndef NDN_CXX_HAVE_OSX_FRAMEWORKS
#error "This file should not be compiled ..."
#endif

#include "../network-interface.hpp"
#include "../scheduler.hpp"
#include "../scheduler-scoped-event-id.hpp"
#include "../../security/tpm/helper-osx.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include <boost/asio/ip/udp.hpp>

namespace ndn {
namespace util {

class NetworkMonitor::Impl
{
public:
  Impl(NetworkMonitor& nm, boost::asio::io_service& io);

  ~Impl();

  uint32_t
  getCapabilities() const
  {
    return NetworkMonitor::CAP_ENUM | NetworkMonitor::CAP_IF_ADD_REMOVE |
      NetworkMonitor::CAP_STATE_CHANGE | NetworkMonitor::CAP_ADDR_ADD_REMOVE;
  }

  shared_ptr<NetworkInterface>
  getNetworkInterface(const std::string& ifname) const;

  std::vector<shared_ptr<NetworkInterface>>
  listNetworkInterfaces() const;

  static void
  afterNotificationCenterEvent(CFNotificationCenterRef center,
                               void* observer,
                               CFStringRef name,
                               const void* object,
                               CFDictionaryRef userInfo);

private:
  void
  scheduleCfLoop();

  void
  pollCfLoop();

  void
  addNewInterface(const std::string& ifName);

  void
  enumerateInterfaces();

  std::set<std::string>
  getInterfaceNames();

  InterfaceState
  getInterfaceState(const std::string& ifName);

  void
  updateInterfaceInfo(NetworkInterface& netif);

  size_t
  getInterfaceMtu(const std::string& ifName);

  static void
  onConfigChanged(SCDynamicStoreRef store, CFArrayRef changedKeys, void* context);

  void
  onConfigChanged(CFArrayRef changedKeys);

private:
  NetworkMonitor& m_nm;
  std::map<std::string /*ifname*/, shared_ptr<NetworkInterface>> m_interfaces; ///< interface map

  Scheduler m_scheduler;
  scheduler::ScopedEventId m_cfLoopEvent;

  SCDynamicStoreContext m_context;
  CFReleaser<SCDynamicStoreRef> m_scStore;
  CFReleaser<CFRunLoopSourceRef> m_loopSource;

  boost::asio::ip::udp::socket m_nullUdpSocket;
};

} // namespace util
} // namespace ndn

#endif // NDN_UTIL_NETWORK_MONITOR_IMPL_OSX_HPP
