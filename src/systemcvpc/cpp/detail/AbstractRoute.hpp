// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Jens Gladigau <jens.gladigau@cs.fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP

#include <systemcvpc/Route.hpp>

#include "AbstractComponent.hpp"

#include <smoc/SimulatorAPI/PortInterfaces.hpp>

#include <CoSupport/Tracing/TracingFactory.hpp>

#include <boost/variant.hpp>
#include <boost/pool/pool.hpp>

#include <vector>
#include <memory>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief Interface for classes implementing routing simulation.
   */
  class AbstractRoute {
    typedef AbstractRoute this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    typedef smoc::SimulatorAPI::PortInInterface         PortInInterface;
    typedef smoc::SimulatorAPI::PortOutInterface        PortOutInterface;
    typedef smoc::SimulatorAPI::ChannelSourceInterface  ChannelSourceInterface;
    typedef smoc::SimulatorAPI::ChannelSinkInterface    ChannelSinkInterface;

    // This class will never be defined. It is
    // a stand in for ChannelSinkInterface or ChannelSourceInterface.
    class ChannelInterface;

    struct Destination {
      Destination(const char *name, ChannelInterface *ci)
        : name(name), ci(ci) {}

      const char       *name;
      ChannelInterface *ci;
    };
    typedef std::vector<Destination> Destinations;

    AbstractRoute(std::string const &name, int facadeAdj);

    template <class T>
    void start(size_t quantitiy, T *userData, void (*completed)(T *, size_t n, ChannelSourceInterface *)) {
      assert(portInterface.which() == 1); // port must be a PortInInterface
      start(quantitiy, userData,
          reinterpret_cast<void (*)(void *, size_t n, ChannelInterface *)>(completed));
    }
    template <class T>
    void start(size_t quantitiy, T *userData, void (*completed)(T *, size_t n, ChannelSinkInterface *)) {
      assert(portInterface.which() == 2); // port must be a PortOutInterface
      start(quantitiy, userData,
          reinterpret_cast<void (*)(void *, size_t n, ChannelInterface *)>(completed));
    }

    Destinations const &getDestinations()
      { return destinations; }

    std::string const &getName() const
      { return routeName; }

    int getRouteId() const
      { return routeId; }

    ///
    /// Interface used by Configuration::finalize(), which is run at end of elaboration.
    ///

    void setPortInterface(PortInInterface  *port);
    void setPortInterface(PortOutInterface *port);
    virtual void finalize() = 0;

    Route       *getRoute() {
      // Pointer magic. Shift our this pointer
      // so that it points to the Route
      // base class of our real implementation.
      return reinterpret_cast<Route *>(
          reinterpret_cast<char *>(this) + facadeAdj);
    }
    Route const *getRoute() const
      { return const_cast<this_type *>(this)->getRoute(); }

    virtual ~AbstractRoute(){}
  protected:
    typedef void (*CallBack)(void *, size_t n, ChannelInterface *);

    /// This will update link with the channel interface for the channel
    /// denoted via chan. However, update of link will be delayed until
    /// setPortInterface has been called.
    /// @param[in]  chan  The name of the channel for which link should be performed.
    /// @param[out] link  Where to store the channel interface.
    void acquireChannelInterface(std::string const &chan, std::vector<ChannelInterface *> &link);

    struct MessageInstance {
      MessageInstance(
          AbstractRoute  *route,
          size_t          quantitiy,
          void           *userData,
          CallBack        completed)
        : route(route), quantitiy(quantitiy)
        , userData(userData), completed(completed) {}

      AbstractRoute *route;
      size_t         quantitiy;
      void          *userData;
      CallBack       completed;
    };

    void *allocate(size_t size) {
      if (!messagePool) {
        messagePool.reset(new boost::pool<>(size));
      } else {
        assert(messagePool->get_requested_size() == size);
      }
      return messagePool->malloc();
    }
    template <class MSG>
    void  release(MSG *msg) {
      msg->~MSG();
      messagePool->free(msg);
    }

    virtual void start(size_t quantitiy, void *userData, CallBack completed) = 0;

    void traceStart() {
      if (ptpTracer) ticket = ptpTracer->startOoo();
    }

    void traceStop() {
      if (ptpTracer) ptpTracer->stopOoo(ticket);
    }

    boost::variant<boost::blank,
      PortInInterface *,
      PortOutInterface *>       portInterface;
  private:
    struct ChannelLink {
      ChannelLink(std::vector<ChannelInterface *> &link)
        : link(&link) {}

      std::vector<ChannelInterface *> *link;
    };

    typedef std::map<std::string, ChannelLink> ChannelLinks;


    std::string const           routeName;
    RouteId                     routeId;
    int                         facadeAdj;
    ChannelLinks                channelLinks;
    Destinations                destinations;

    std::unique_ptr<boost::pool<>> messagePool;

    CoSupport::Tracing::PtpTracer::Ptr     ptpTracer;
    CoSupport::Tracing::PtpTracer::Ticket  ticket;
  };

  inline
  void intrusive_ptr_add_ref(AbstractRoute *p) {
    p->getRoute()->add_ref();
  }

  inline
  void intrusive_ptr_release(AbstractRoute *p) {
    if (p->getRoute()->del_ref())
      // AbstractRoute has virtual destructor
      delete p;
  }

  static inline
  AbstractRoute *getAbstractRouteOfPort(smoc::SimulatorAPI::PortInInterface const &port)
    { return static_cast<AbstractRoute *>(port.getSchedulerInfo()); }
  static inline
  AbstractRoute *getAbstractRouteOfPort(smoc::SimulatorAPI::PortOutInterface const &port)
    { return static_cast<AbstractRoute *>(port.getSchedulerInfo()); }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP */
