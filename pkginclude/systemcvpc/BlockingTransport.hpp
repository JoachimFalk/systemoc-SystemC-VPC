/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * BlockingTransport.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/

#ifndef __INCLUDED__BLOCKINGTRANSPORT__H__
#define __INCLUDED__BLOCKINGTRANSPORT__H__
#include <list>
#include <utility>

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include "AbstractComponent.hpp"
#include "EventPair.hpp"
#include "ProcessControlBlock.hpp"
#include "Route.hpp"
#include "Timing.hpp"

namespace SystemC_VPC{
  template<class ROUTE>
  class RoutePool;

  /**
   *
   */
  class BlockingTransport :
    public Route,
    public CoSupport::SystemC::Event,
    protected CoSupport::SystemC::EventListener {
  public:

    void compute( Task* task );

    FunctionId getFunctionId(ProcessId pid, std::string function);

    void route( EventPair np );

    void signaled(EventWaiter *e);

    void eventDestroyed(EventWaiter *e);

    void addHop(std::string name, AbstractComponent * hop);

    void setPool(RoutePool<BlockingTransport> * pool);

    const ComponentList& getHops() const;

    BlockingTransport( std::string source, std::string dest );

    BlockingTransport( const BlockingTransport & route );

    ~BlockingTransport( );

    const char* getName() const;
  private:
    void resetHops();
    void resetLists();

  private:
    enum Phase {
      LOCK_ROUTE,
      COMPUTE_ROUTE
    };
    typedef std::list<std::pair<AbstractComponent *, Task *> > Components;

    Components                             hopList;
    Components                             lockList;
    Components::iterator                   nextHop;
    ComponentList                          components;

    // a rout is either input (read) or output (write)
    bool                                   isWrite;

    Task*                                  task;
    EventPair                              taskEvents;
    CoSupport::SystemC::RefCountEventPtr   dummyDii;
    CoSupport::SystemC::RefCountEventPtr   routeLat;
    std::string                            name;
    RoutePool<BlockingTransport>          *pool;

    // blocking transport has two phases:
    // - lock the route
    // - apply the route
    Phase                                  phase;

    
  };
}

#endif // __INCLUDED__BLOCKINGTRANSPORT__H__