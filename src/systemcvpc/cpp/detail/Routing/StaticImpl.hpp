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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP

#include <systemcvpc/Routing/Static.hpp>
#include <systemcvpc/EventPair.hpp>

#include "../AbstractRoute.hpp"
#include "../AbstractComponent.hpp"
#include "../TaskImpl.hpp"

#include <systemc>

#include <list>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  /**
   *
   */
  class StaticImpl
    : public AbstractRoute
    , public SystemC_VPC::Routing::Static
  {
    typedef StaticImpl this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    StaticImpl(std::string const &name);

    ~StaticImpl();

    ///
    /// Handle interfaces for SystemC_VPC::Route
    ///

    // For resolving ambiguity
    using AbstractRoute::getName;
    using AbstractRoute::getRouteId;

    ///
    /// Handle interfaces for SystemC_VPC::Routing::Static
    ///

    Hop  *addHop(Component::Ptr component, Hop *parent);
    void  addDest(std::string const &chan, Hop *parent);
    std::set<Hop *> const &getFirstHops() const;
    std::set<Hop *> const &getHops() const;

    bool addStream();
    bool closeStream();

  private:
    ///
    /// Handle interfaces for AbstractRoute
    ///

    void start(size_t quantitiy, void *userData, CallBack completed);

    void finalize();

    ///
    /// Other stuff
    ///

    struct HopImpl: public Hop {
      HopImpl(Component::Ptr component)
        : Hop(component)
        , taskImpl(nullptr) {}

      AbstractComponent::Ptr getComponent() const
        { return SystemC_VPC::getImpl(this->Hop::getComponent()); }

      std::list<HopImpl *> &getChildHops() {
        return reinterpret_cast<std::list<HopImpl *> &>(childHops);
      }

      TaskImpl                        *taskImpl;
      std::vector<ChannelInterface *>  destinations;
    };

    void recurseHop(HopImpl *hopImpl);

    std::set<HopImpl *>         firstHopImpls;

    typedef std::set<HopImpl *> HopImpls;
    HopImpls                    hopImpls;

    struct MessageInstance: public AbstractRoute::MessageInstance {
      MessageInstance(
          StaticImpl     *route,
          size_t          quantitiy,
          void           *userData,
          CallBack        completed)
        : AbstractRoute::MessageInstance(route, quantitiy, userData, completed)
        , nextFreeCurrHop(0)
      {
        for (HopImpl *firstHopImpl : route->firstHopImpls) {
          currHop[nextFreeCurrHop] = firstHopImpl;
          startHop(nextFreeCurrHop++);
        }
      }
    private:
      void startHop(size_t hop);
      void finishHop(size_t hop);
    private:
      size_t                  nextFreeCurrHop;
      HopImpl                *currHop[1];
    };
  };

  inline
  void intrusive_ptr_add_ref(StaticImpl *p) {
    ::intrusive_ptr_add_ref(p);
  }

  inline
  void intrusive_ptr_release(StaticImpl *p) {
    ::intrusive_ptr_release(p);
  }

} } } // namespace SystemC_VPC::Detail::Routing

namespace SystemC_VPC { namespace Routing {

  static inline
  Detail::Routing::StaticImpl       *getImpl(Static       *route)
    { return static_cast<Detail::Routing::StaticImpl       *>(route); }
  static inline
  Detail::Routing::StaticImpl const *getImpl(Static const *route)
    { return static_cast<Detail::Routing::StaticImpl const *>(route); }

} } // namespace SystemC_VPC::Detail::Routing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP */
