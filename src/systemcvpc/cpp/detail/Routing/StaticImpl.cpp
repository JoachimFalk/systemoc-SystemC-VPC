// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include "StaticImpl.hpp"
#include "../DebugOStream.hpp"

#include <CoSupport/Tracing/TracingFactory.hpp>
#include <CoSupport/Tracing/PtpTracer.hpp>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  StaticImpl::StaticImpl(std::string const &name)
    : AbstractRoute(name,
        reinterpret_cast<char *>(static_cast<Route         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)))
    , Static(
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)) -
        reinterpret_cast<char *>(static_cast<Route         *>(this)))
  {
  }

  StaticImpl::Hop  *StaticImpl::addHop(Component::Ptr component, Hop *parent) {
    AbstractComponent::Ptr comp(
        static_cast<AbstractComponent *>(component.get()));
    std::pair<HopImpls::iterator, bool> status =
        hopImpls.insert(new HopImpl(component));
    assert(status.second);
    HopImpl *hopImpl = *status.first;
    if (!parent) {
      std::pair<HopImpls::iterator, bool> status =
          firstHopImpls.insert(hopImpl);
      assert(status.second);
    } else {
      HopImpl *parentImpl = static_cast<HopImpl *>(parent);
#ifndef NDEBUG
      HopImpls::iterator iter = hopImpls.find(parentImpl);
      assert(iter != hopImpls.end());
#endif //NDEBUG
      parentImpl->getChildHops().push_back(hopImpl);
    }
    return hopImpl;
  }

  void              StaticImpl::addDest(std::string const &chan, Hop *parent) {
    HopImpl *parentImpl = static_cast<HopImpl *>(parent);
#ifndef NDEBUG
    HopImpls::iterator iter = hopImpls.find(parentImpl);
    assert(iter != hopImpls.end());
#endif //NDEBUG
    acquireChannelInterface(chan, parentImpl->destinations);
  }

  std::set<StaticImpl::Hop *> const &StaticImpl::getFirstHops() const {
    return reinterpret_cast<std::set<Hop *> const &>(firstHopImpls);
  }

  std::set<StaticImpl::Hop *> const &StaticImpl::getHops() const {
    return reinterpret_cast<std::set<Hop *> const &>(hopImpls);
  }

  void StaticImpl::start(size_t quantitiy, void *userData, CallBack completed) {
    // This will auto delete.
    size_t bytes = sizeof(MessageInstance) + sizeof(HopImpl *)*(getDestinations().size()-1);

    new(allocate(bytes)) MessageInstance(this, quantitiy, userData, completed);
  }

  void StaticImpl::recurseHop(HopImpl *hopImpl) {
    hopImpl->taskImpl = hopImpl->getComponent()->createTask(getName());
    hopImpl->taskImpl->setPriority(hopImpl->getPriority());
    for (HopImpl *childHop : hopImpl->getChildHops())
      recurseHop(childHop);
  }

  void StaticImpl::finalize() {
    assert(!firstHopImpls.empty() && "Oops, no route specified!");
    for (HopImpl *firstHopImpl : firstHopImpls)
      recurseHop(firstHopImpl);
  }

  void StaticImpl::MessageInstance::startHop(size_t hop) {
    assert(currHop[hop] != nullptr);
    currHop[hop]->getComponent()
        ->executeHop(currHop[hop]->taskImpl, currHop[hop]->getTransferTiming(), quantitiy, [this, hop](TaskInstanceImpl *) {
          this->finishHop(hop);
      });
  }

  void StaticImpl::MessageInstance::finishHop(size_t hop) {
    for (ChannelInterface *ci : currHop[hop]->destinations) {
      (*completed)(userData, quantitiy, ci);
    }
    if (!currHop[hop]->getChildHops().empty()) {
      for (HopImpl *nextHop : currHop[hop]->getChildHops()) {
        currHop[hop] = nextHop;
        startHop(hop);
        hop = nextFreeCurrHop++;
      }
    } else {
      currHop[hop] = nullptr;
      for (size_t i = 0; i < nextFreeCurrHop; ++i) {
        if (currHop[i] != nullptr)
          // Still some stuff running
          return;
      }
      // Its all over
      static_cast<StaticImpl *>(route)->release(this);
    }
  }

  bool StaticImpl::closeStream(){
    for (HopImpls::value_type const &v : hopImpls) {
      v->getComponent()->closeStream(getRouteId());
    }
    return true;
  }

  bool StaticImpl::addStream(){
    for (HopImpls::value_type const &v : hopImpls) {
      v->getComponent()->addStream(getRouteId());
    }
    return true;
  }

  StaticImpl::~StaticImpl( ){
    DBG_OUT("Static::~Static( )" << std::endl);
  }


} } } // namespace SystemC_VPC::Detail::Routing
