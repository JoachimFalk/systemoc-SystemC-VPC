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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_IGNOREIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_IGNOREIMPL_HPP

#include <systemcvpc/Routing/Ignore.hpp>
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
  class IgnoreImpl
    : public AbstractRoute
    , public SystemC_VPC::Routing::Ignore
  {
    typedef IgnoreImpl this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    IgnoreImpl(std::string const &name);

    ~IgnoreImpl();

    ///
    /// Handle interfaces for SystemC_VPC::Route
    ///

    // For resolving ambiguity
    using AbstractRoute::getName;
    using AbstractRoute::getRouteId;

    ///
    /// Handle interfaces for SystemC_VPC::Routing::Ignore
    ///
  private:
    ///
    /// Handle interfaces for AbstractRoute
    ///

    void start(size_t quantitiy, void *userData, CallBack completed);

    void finalize();

    ///
    /// Other stuff
    ///

    class Visitor;
  };

  inline
  void intrusive_ptr_add_ref(IgnoreImpl *p) {
    ::intrusive_ptr_add_ref(p);
  }

  inline
  void intrusive_ptr_release(IgnoreImpl *p) {
    ::intrusive_ptr_release(p);
  }

} } } // namespace SystemC_VPC::Detail::Routing

namespace SystemC_VPC { namespace Routing {

  static inline
  Detail::Routing::IgnoreImpl       *getImpl(Ignore       *route)
    { return static_cast<Detail::Routing::IgnoreImpl       *>(route); }
  static inline
  Detail::Routing::IgnoreImpl const *getImpl(Ignore const *route)
    { return static_cast<Detail::Routing::IgnoreImpl const *>(route); }

} } // namespace SystemC_VPC::Detail::Routing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_IGNOREIMPL_HPP */
