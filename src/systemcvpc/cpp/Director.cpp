// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "config.h"
//#include <systemcvpc/vpc_config.h>

#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/config/VpcApi.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include "AbstractComponent.hpp"
#include "ConfigCheck.hpp"
#include "config/Mappings.hpp"
#include "DebugOStream.hpp"
#include "HysteresisLocalGovernor.hpp"

#include "NonPreemptiveScheduler/DynamicPriorityComponent.hpp"
#include "NonPreemptiveScheduler/FcfsComponent.hpp"
#include "NonPreemptiveScheduler/NonPreemptiveComponent.hpp"
#include "NonPreemptiveScheduler/PriorityComponent.hpp"
#include "NonPreemptiveScheduler/RoundRobinComponent.hpp"
#include "PreemptiveScheduler/AVBScheduler.hpp"
#include "PreemptiveScheduler/FlexRayScheduler.hpp"
#include "PreemptiveScheduler/MostScheduler.hpp"
#include "PreemptiveScheduler/MostSecondaryScheduler.hpp"
#include "PreemptiveScheduler/PriorityScheduler.hpp"
#include "PreemptiveScheduler/RateMonotonicScheduler.hpp"
#include "PreemptiveScheduler/RoundRobinScheduler.hpp"
#include "PreemptiveScheduler/StreamShaperScheduler.hpp"
#include "PreemptiveScheduler/TDMAScheduler.hpp"
#include "PreemptiveScheduler/TimeTriggeredCCScheduler.hpp"

#include "PluggablePowerGovernor.hpp"
#include "PowerSumming.hpp"
#include "PreemptiveScheduler/PreemptiveComponent.hpp"
#include "ProcessControlBlock.hpp"
#include "RoutePool.hpp"
#include "SelectFastestPowerModeGlobalGovernor.hpp"
#include "StaticRoute.hpp"
#include "Task.hpp"
#include "tracing/DataBaseTracer.hpp"
#include "tracing/NullTracer.hpp"
#include "tracing/PajeTracer.hpp"
#include "tracing/VcdTracer.hpp"
#include "VPCBuilder.hpp"

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemc>

#include <boost/foreach.hpp>

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace SystemC_VPC {

  namespace {
    namespace VC = Config;

    static
    void injectRoute(std::string src, std::string dest, sc_core::sc_port_base * leafPort)
    {
      ProcessId pid = Director::getProcessId(src, dest);
      if (VC::Routing::has(pid) && VC::Routing::has(leafPort)) {
        if(VC::Routing::get(pid) != VC::Routing::get(leafPort)) {
            std::cout<<"debug Multicast: " << VC::Routing::get(pid)->getDestination() << " and " << VC::Routing::get(leafPort)->getDestination() << std::endl;
          /*throw VC::ConfigException("Route " + src + " -> " + dest +
              " has configuration data from XML and from configuration API.");*/
        }
      } else if (!VC::Routing::has(pid) && !VC::Routing::has(leafPort)) {
        throw VC::ConfigException("Route " + src + " -> " + dest +
            " has NO configuration data at all.");
      } else if (VC::Routing::has(pid)) {
        VC::Route::Ptr route = VC::Routing::get(pid);
        VC::Routing::set(leafPort, route);
      } else if (VC::Routing::has(leafPort)) {
        VC::Route::Ptr route = VC::Routing::get(leafPort);
        VC::Routing::set(pid, route);
        route->inject(src, dest);
      }

      assert(VC::Routing::has(pid) && VC::Routing::has(leafPort));
      //assert(VC::Routing::get(pid) == VC::Routing::get(leafPort));
    }

    static
    AbstractComponent * createComponent(Config::Component::Ptr component) {
      AbstractComponent *comp = NULL;
      switch (component->getScheduler()) {
        case VC::Scheduler::FCFS:
          comp = new FcfsComponent(component);
          break;
        case VC::Scheduler::StaticPriorityNoPreempt:
          comp = new PriorityComponent(component);
          break;
        case VC::Scheduler::RoundRobinNoPreempt:
          comp = new RoundRobinComponent(component);
          break;
        case VC::Scheduler::DynamicPriorityUserYield:
          comp = new DynamicPriorityComponent(component);
          break;
        case Config::Scheduler::RoundRobin:
          comp = new PreemptiveComponent(component, new RoundRobinScheduler());
          break;
        case Config::Scheduler::StaticPriority:
          comp = new PreemptiveComponent(component, new PriorityScheduler());
          break;
        case Config::Scheduler::RateMonotonic:
          comp = new PreemptiveComponent(component, new RateMonotonicScheduler());
          break;
        case Config::Scheduler::TDMA:
          comp = new PreemptiveComponent(component, new TDMAScheduler());
          break;
        case Config::Scheduler::FlexRay:
          comp = new PreemptiveComponent(component, new FlexRayScheduler());
          break;
        case Config::Scheduler::AVB:
          comp = new PreemptiveComponent(component, new AVBScheduler());
          break;
        case Config::Scheduler::TTCC:
          comp = new PreemptiveComponent(component, new TimeTriggeredCCScheduler());
          break;
        case Config::Scheduler::MOST:
          comp = new PreemptiveComponent(component, new MostScheduler());
          break;
        case Config::Scheduler::StreamShaper:
          comp = new PreemptiveComponent(component, new StreamShaperScheduler());
          break;
        default:
          assert(!"Oops, I don't know this scheduler!");
      }

      switch(component->getTracing()){
        case Config::Traceable::NONE:
          comp->addTracer(new Tracing::NullTracer(component));
          break;
        case Config::Traceable::PAJE:
          comp->addTracer(new Tracing::PajeTracer(component));
          break;
        case Config::Traceable::VCD:
          comp->addTracer(new Tracing::VcdTracer(component));
          break;
        case Config::Traceable::DB:
          comp->addTracer(new Tracing::DataBaseTracer(component));
          break;
        default:
          assert(!"Oops, I don't know this tracer!");
      }

      VC::Mappings::getComponents()[component] = comp;
      Director::getInstance().registerComponent(comp);
      std::vector<AttributePtr> atts = component->getAttributes();
      for(std::vector<AttributePtr>::const_iterator iter = atts.begin();
          iter != atts.end(); ++iter){
        comp->setAttribute(*iter);
      }
      return comp;
    }

  } // namespace anonymous

  std::unique_ptr<Director> Director::singleton;

  sc_core::sc_time Director::end = sc_core::SC_ZERO_TIME;

  /**
   *
   */
  Director::Director()
    : FALLBACKMODE(false)
    , defaultRoute(false)
    , checkVpcConfig(true)
    , topPowerGov(new InternalSelectFastestPowerModeGovernor)
    , topPowerGovFactory(NULL)
#ifndef NO_POWER_SUM
    , powerConsStream("powerconsumption.dat")
    , powerSumming(NULL)
#endif // NO_POWER_SUM
    , taskPool(new TaskPool())
  {
    sc_core::sc_report_handler::set_actions(
        sc_core::SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_,
        sc_core::SC_DO_NOTHING);
    //sc_core::sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_,
    //                               SC_DO_NOTHING);
    sc_core::sc_report_handler::set_actions(
        sc_core::SC_WARNING,
        sc_core::SC_DO_NOTHING);

    try{
      VPCBuilder builder((Director*)this);
      builder.buildVPC();
    }catch(InvalidArgumentException& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }catch(const std::exception& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }
  }

  /**
   *
   */
  Director::~Director(){

    sc_core::sc_time start = sc_core::SC_ZERO_TIME;
    sc_core::sc_time end = this->end;
#ifdef DBG_DIRECTOR
    std::cerr << "start: " << start << " end: " << end  << std::endl;
#endif //DBG_DIRECTOR

    if(0 != this->vpc_result_file.compare("")) {
#ifdef DBG_DIRECTOR
      std::cerr << "Director> result_file: "
                << this->vpc_result_file << std::endl;
#endif //DBG_DIRECTOR
      std::ofstream resultFile;
      resultFile.open(this->vpc_result_file.c_str());
      if(resultFile){
        resultFile << (end-start).to_default_time_units();
      }
      resultFile.flush();
      resultFile.close();
    }else{
      std::cerr << "[VPC] overall simulated time: " << end - start << std::endl;
    }

#ifndef NO_POWER_SUM
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it )
    {
      if(*it != NULL) {
        (*it)->removeObserver(powerSumming);
      }
    }

    delete powerSumming;
#endif // NO_POWER_SUM

    // clear components
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it ){
      if(*it != NULL) {
        delete *it;
      }
    }

    componentIdMap.clear();

    delete taskPool;
  }

  Task *Director::preCompute(FastLink const *fLink) {
    try {
      Task *task = this->allocateTask( fLink->process );
      task->setFunctionIds( fLink->actionIds );
      task->setGuardIds(fLink->guardIds);
      task->setFactorOverhead(fLink->complexity);
    
      //HINT: also treat mode!!
      //if( endPair.latency != NULL ) endPair.latency->notify();
      assert(fLink->component);
      return task;
    } catch (NotAllocatedException &e) {
      std::cerr << "Unknown Task: ID = " << fLink->process
           << " name = " << this->getTaskName(fLink->process)  << std::endl;

      debugUnknownNames();
    }
    throw NotAllocatedException(this->getTaskName(fLink->process));
  }

  //
  void Director::postCompute( Task * task,
                              EventPair endPair ){

    if( endPair.dii == NULL){
      // active mode -> waits until simulated delay time has expired
      
      EventPair blockEvent =  task->getBlockEvent();

      CoSupport::SystemC::wait(*blockEvent.dii);
//    blockEvent.dii = NULL;
//    blockEvent.latency = NULL;
    }
  }

  void Director::check(FastLink const *fLink) {
    if (FALLBACKMODE)
      return;
    Task *task = preCompute(fLink);
    task->setName(task->getName().append("_check"));
    task->setTimingScale(1);

    Delayer *comp = fLink->component;
    assert(comp);
    comp->check(task);
  }

  //
  void Director::compute(FastLink const *fLink, EventPair endPair) {
    if (FALLBACKMODE) {
      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end
      // do nothing, just return
      return;
    }

    Task * task = preCompute(fLink);
    task->setBlockEvent( endPair );
    task->setTimingScale(1);

    Delayer* comp = fLink->component;
    comp->compute(task);
    postCompute(task, endPair);
  }

  //
  void Director::read(FastLink const *fLink,
                      size_t quantum,
                      EventPair endPair ) {
    if (FALLBACKMODE) {
      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end
      // do nothing, just return
      return;
    }

    // FIXME: treat quantum
    Task * task = preCompute(fLink);
    task->setBlockEvent( endPair );
    task->setWrite(false);
    task->setTimingScale(quantum);
    assert(!FALLBACKMODE);

    Delayer* comp = fLink->component;
    comp->compute(task);
    postCompute(task, endPair);
  }

  //
  void Director::write(FastLink const *fLink,
                       size_t quantum,
                       EventPair endPair ) {
    if (FALLBACKMODE) {
      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end
      // do nothing, just return
      return;
    }

    // FIXME: treat quantum
    Task * task = preCompute(fLink);
    task->setBlockEvent(endPair);
    task->setWrite(true);
    task->setTimingScale(quantum);
    assert(!FALLBACKMODE);

    Delayer* comp = fLink->component;
    comp->compute(task);
    postCompute(task, endPair);
  }

  /**
   * \brief Implementation of Director::registerComponent
   */
  void Director::registerComponent(Delayer* comp){
    ComponentId cid = comp->getComponentId();
    if(cid >= components.size())
      components.resize(cid+100, NULL);

    this->componentIdMap[comp->getName()] = cid;

    this->components[cid] = comp;

    DBG_OUT(" Director::registerComponent(" << comp->getName()
            << ") [" << comp->getComponentId() << "] # " << components.size()
            << std::endl);
  }

  void Director::finalizeMapping(
      TaskInterface       *actor,
      std::string   const &actorName,
      FunctionNames const &actionNames,
      FunctionNames const &guardNames)
  {
    AbstractComponent * comp = static_cast<AbstractComponent *>(actor->getScheduler());

    const ProcessId pid = Director::getInstance().getProcessId(actorName);
    // Get existing ProcessControlBlock
    ProcessControlBlock *pcb = comp->getPCB(pid);

    if (!taskPool->contains( pid )) {
      Task &task = taskPool->createObject(pid);
      task.setPCB(pcb);
      task.setProcessId( pid );
      task.setScheduledTask(actor);
      task.setName(actor->name());
      actor->setSchedulerInfo(allocateTask(pid));
    }

    VC::VpcTask::Ptr task = VC::getCachedTask(static_cast<ScheduledTask &>(*actor));

    //TODO: VC::Timing -> Timing
    const VC::Components & components = VC::getComponents();
    BOOST_FOREACH(VC::Components::value_type component_pair, components)
    {
      std::string componentName = component_pair.first;
      VC::Component::Ptr component = component_pair.second;

      if (VC::Mappings::isMapped(task, component)) {
        VC::TimingsProvider::Ptr provider = component->getTimingsProvider();
        pcb->setPriority(task->getPriority());  // GFR BUGFIX
        pcb->setTaskIsPSM(task->isPSM());
        if (provider->hasDefaultActorTiming(actorName)) {

                SystemC_VPC::Config::functionTimingsPM timingsPM = provider->getActionTimings(actorName);

                for (SystemC_VPC::Config::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
                {
                        std::string powermode = (*it).first;
                        pcb->setTiming(provider->getActionTiming(actorName,powermode));
                }

        }
        BOOST_FOREACH(std::string guard, guardNames)
        {
          if (provider->hasGuardTimings(guard)) {

                  SystemC_VPC::Config::functionTimingsPM timingsPM = provider->getGuardTimings(guard);
                for (SystemC_VPC::Config::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
                {
                        std::string powermode = (*it).first;
                        pcb->setTiming(provider->getGuardTiming(guard,powermode));

                        ConfigCheck::configureTiming(pid, guard);
                }
          }
        }
        BOOST_FOREACH(std::string action, actionNames)
        {
          if (provider->hasActionTimings(action)) {

                  SystemC_VPC::Config::functionTimingsPM timingsPM = provider->getActionTimings(action);
                  for (SystemC_VPC::Config::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
                  {
                          std::string powermode = (*it).first;
                          pcb->setTiming(provider->getActionTiming(action,powermode));
                          ConfigCheck::configureTiming(pid, action);
                  }
          }
        }
      }
    }
//TODO:
//    p.setPeriod(1);
//    p.setBaseDelay();
//    p.setBaseLatency();
  }

  /**
   *
   */ 
  void Director::registerRoute(Route* route){
    assert(!FALLBACKMODE);
    this->registerComponent(route);
    const std::string & taskName = route->getName();
    const std::string & compName = route->getName();

    ProcessId       pid = getProcessId( taskName );
//  if( pid >= mappings.size() ){
//    mappings.resize( pid + 100, NULL );
//  }
    DBG_OUT("registerRoute( " << taskName << " " << pid << " )"<< std::endl);

    if( !taskPool->contains( pid ) ){
      Task &task = taskPool->createObject( pid );
      task.setProcessId( pid );
      task.setName( taskName );
    }

//  assert(pid <= mappings.size());
//  ComponentId cid = this->getComponentId(compName);
//  Delayer * comp = components[cid];

//  assert( comp != NULL );
//  mappings[pid] = comp;
//  const ComponentList& hops = route->getHops();
//  for(ComponentList::const_iterator iter = hops.begin();
//      iter != hops.end();
//      ++iter){
//    ComponentId hid = this->getComponentId((*iter)->getName());
//  }
  }

  Task* Director::allocateTask(ProcessId pid){
    return this->taskPool->allocate(pid);
  }

  //
  const Delayer * Director::getComponent(FastLink const *vpcLink) const {
    assert(vpcLink->component);
    return vpcLink->component;
  }

  //
  void Director::signalLatencyEvent(Task* task){
    assert(!FALLBACKMODE);

#ifdef DBG_DIRECTOR
    std::cerr << "Director> got notified from: " << task->getName()
              << std::endl;
    std::cerr << "Director> task successful finished: " << task->getName()
              << std::endl;
#endif //DBG_DIRECTOR
    if(NULL != task->getBlockEvent().latency)
      task->getBlockEvent().latency->notify();
    // remember last acknowledged task time
    this->end = sc_core::sc_time_stamp();
    
    // free allocated task
    task->release();
  }


  ProcessId getNextProcessId() {
    static ProcessId       globalProcessId = 0;
    return globalProcessId++;
  }

  ProcessId Director::getProcessId(std::string process_or_source,
      std::string destination)
  {
    typedef std::map<std::string, ProcessId> ProcessIdMap;
    static ProcessIdMap processIdMap;

    if (destination == "") {
      std::string & process = process_or_source;
      ProcessIdMap::const_iterator iter = processIdMap.find(process);
      if (iter == processIdMap.end()) {
        ProcessId id = getNextProcessId();
        processIdMap[process] = id;
        ConfigCheck::setProcessName(id, process);
      }
      iter = processIdMap.find(process);
      return iter->second;

    } else {
      std::string & source = process_or_source;
      std::string name_hack = "msg_" + source + "_2_" + destination;
      ProcessIdMap::const_iterator iter = processIdMap.find(name_hack);
      if (iter == processIdMap.end()) {
        ProcessId id = getNextProcessId();
        processIdMap[name_hack] = id;
        ConfigCheck::setRouteName(id, source, destination);
      }
      iter = processIdMap.find(name_hack);
      return iter->second;
    }
  }

  ComponentId Director::getComponentId(std::string component) {
#ifdef DBG_DIRECTOR
    std::cerr << " Director::getComponentId(" << component
         << ") # " << componentIdMap.size()
         << std::endl;
#endif //DBG_DIRECTOR

    ComponentIdMap::const_iterator iter = componentIdMap.find(component);
    assert( iter != componentIdMap.end() );
    return iter->second;
      
  }

  typedef std::map<std::string, FunctionId>  FunctionIdMap;

  FunctionId uniqueFunctionId() {
    static FunctionId globalFunctionId = 1;
    return globalFunctionId++;
  }


  FunctionIdMap& getFunctionIdMap(){
    static FunctionIdMap functionIdMap;
    return functionIdMap;
  }

  // FunctionIds are created by VPC during XML parsing.
  FunctionId Director::createFunctionId(const std::string& function) {
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    if( iter == getFunctionIdMap().end() ) {
      // map empty function names to default timing
      if(function == "") {
        getFunctionIdMap()[function] = defaultFunctionId;
      }else{
        getFunctionIdMap()[function] = uniqueFunctionId();
      }
    }
    iter = getFunctionIdMap().find(function);
    return iter->second;
  }

  bool Director::hasFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    return (iter != getFunctionIdMap().end());
  }

  // FunctionIds are used (get) by SysteMoC.
  // The default ID (and a default timing) is used if no ID was created during
  // parsing. (The function name was not given in the XML.)
  FunctionId Director::getFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);

    // the function name was not set in configuration
    // -> we have to use the default delay
    if( iter == getFunctionIdMap().end() ) {
      return defaultFunctionId;
    }
    return iter->second;
    
  }

  /// begin section: VpcApi.hpp related stuff

  void Director::beforeVpcFinalize()
  {
    // create AbstractComponents and configure mappings given from config API
    const VC::Components & components = VC::getComponents();

    BOOST_FOREACH(VC::Components::value_type component_pair, components) {
      std::string componentName = component_pair.first;
      VC::Component::Ptr component = component_pair.second;
      assert(componentName == component->getName());

      VC::Component::MappedTasks tasks = component->getMappedTasks();

      AbstractComponent *comp = createComponent(component);
      assert(comp != NULL);

      BOOST_FOREACH(TaskInterface* task, tasks) {
        VC::Mappings::getConfiguredMappings()[VC::getCachedTask(*task)] = component;
      }
    }


#ifndef NO_POWER_SUM
    powerSumming = new PowerSumming(powerConsStream);
#endif // NO_POWER_SUM
    for( Components::iterator it = this->components.begin();
         it != this->components.end();
         ++it )
    {
      if(*it != NULL) {
#ifndef NO_POWER_SUM
        (*it)->addObserver(powerSumming);
#endif // NO_POWER_SUM
        (*it)->initialize(this);

      }
    }
  }
  /// end section: VpcApi.hpp related stuff

  //
  void Director::endOfVpcFinalize()
  {
    if (checkVpcConfig) {
      ConfigCheck::check();
    }
  }

  //
  bool Director::hasValidConfig() const
  {
    return !FALLBACKMODE;
  }

  FastLink Director::registerActor(TaskInterface * actor,
      std::string actorName,
      const FunctionNames &actionNames,
      const FunctionNames &guardNames,
      const int complexity)
  {
    //TODO: registerActor is called multiple times (for each transition)

    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    ProcessId pid = getProcessId(  actorName  );

    try {
      //injectTaskName(actor, actorName);
      finalizeMapping(actor, actorName, actionNames, guardNames);
    }catch(std::exception & e){
      std::cerr << "Actor registration failed for \"" << actorName <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    FunctionIds     actionIds;
    FunctionIds     guardIds;

    for(FunctionNames::const_iterator iter = actionNames.begin();
        iter != actionNames .end();
        ++iter){
      debugFunctionNames[pid].insert(*iter);
      //check if we have timing data for this function in the XML configuration
      if (hasFunctionId(*iter)) {
        actionIds.push_back( getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }
    for(FunctionNames::const_iterator iter = guardNames.begin();
        iter != guardNames.end();
        ++iter){
      debugFunctionNames[pid].insert(*iter);
      if(hasFunctionId(*iter)){
        guardIds.push_back( getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }

    AbstractComponent *comp =
        static_cast<AbstractComponent *>(actor->getScheduler());
    assert(comp);
    return FastLink(comp, pid, actionIds, guardIds, complexity);
  }

  FastLink Director::registerRoute(std::string source,
    std::string destination,
    sc_core::sc_port_base * leafPort)
  {
    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    ProcessId       pid = getProcessId( source, destination );
    FunctionIds     fids; // empty functionIds
    fids.push_back( getFunctionId("1") );

    if (!VC::Routing::has(pid) && !VC::Routing::has(leafPort) && defaultRoute) {
      // default behavior: add empty route
      VC::Route::Ptr route = VC::createRoute(source, destination);
      route->setTracing(false);
    }

    try{
      injectRoute(source, destination, leafPort);
    }catch(std::exception & e){
      std::cerr << "Route registration failed for route\"" << source << " - " <<
          destination <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    assert( !taskPool->contains(pid) );

    VC::Route::Ptr configuredRoute = VC::Routing::get(pid);
    Route * route = VC::Routing::create(configuredRoute);

    Director::registerRoute(route);

    return FastLink(route, pid, fids, FunctionIds(),0);
  }

  sc_core::sc_time Director::createSC_Time(const char* timeString)
    throw(InvalidArgumentException)
  {
    try{
      return CoSupport::SystemC::createSCTime(timeString);
    } catch(std::string &msg){
      throw InvalidArgumentException(msg);
    }
  }

  sc_core::sc_time Director::createSC_Time(std::string timeString)
    throw(InvalidArgumentException)
  {
    return Director::createSC_Time( timeString.c_str());
  }

  void Director::loadGlobalGovernorPlugin(std::string plugin,
                                          AttributePtr attPtr){
    //std::cerr << "Director::loadGlobalGovernorPlugin" << std::endl;
    topPowerGovFactory =
      new DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
        (plugin.c_str());
    if( topPowerGovFactory->factory){
      delete topPowerGov;

      topPowerGovFactory->factory->processAttributes(attPtr);
      topPowerGov = topPowerGovFactory->factory->createPlugIn();
    }
  }

  void Director::debugUnknownNames( ) const {
    bool route = false;
    bool mappings = false;
    for(std::map<ProcessId, std::pair<std::string, std::string> >::const_iterator
          iter = ConfigCheck::routeNames().begin();
        iter != ConfigCheck::routeNames().end();
        ++iter){
      if( !taskPool->contains( iter->first ) ){
           if(!route){
             std::cout << "Found unknown routes.\n"
                       <<" Please add the following routes to the config file:"
                       << std::endl;
             route = true;
           }

        std::cout << "  <route  source=\""
                  << iter->second.first
                  << "\" destination=\""
                  << iter->second.second
                  << "\">\n   <hop name=\"?\">  </hop>\n  </route>"
                  << std::endl;
      }
    }
    for(std::map<ProcessId, std::string>::const_iterator iter =
          ConfigCheck::processNames().begin();
        iter != ConfigCheck::processNames().end();
        ++iter){
      if( !taskPool->contains( iter->first ) ){
        if(!mappings){
          std::cout << "\n" << std::endl;
          std::cout << "Unknown mapping for tasks.\n"
                    <<" Please add the following mapping to the config file:"
                    << std::endl;
          mappings = true;
        }
        std::cout << "  <mapping source=\""
                  << iter->second
                  << "\" target=\"?\">\n"
                  << "    <!-- we may use a default delay: "
                  << "    <timing dii=\"? us\""
                  << " latency=\"? us\" />"
                  << " --> "
                  << std::endl;

        const std::set<std::string>& functionNames =
          debugFunctionNames.find(iter->first)->second;

        for(std::set<std::string>::const_iterator fiter =
            functionNames.begin();
            fiter != functionNames.end();
            ++fiter){
          if(0 == fiter->compare("???")){
            std::cout << "    <!-- the \"???\" is caused by a SysteMoC"
                      << " transition without function CALL  -->" << std::endl;
          }
          std::cout << "    <timing fname=\""
                    << *fiter
                    << "\" dii=\"? us\""
                    << " latency=\"? us\" />"
                    << std::endl;
        }
        std::cout <<"  </mapping>" << std::endl;
      }
    }
    if(route || mappings){
      std::cout << "\n" << std::endl;
    }
    exit(-1);
  }

  //
  std::string Director::getTaskName(ProcessId id) {
    if(ConfigCheck::hasProcessName(id)){
      return ConfigCheck::getProcessName(id);
    }else{
      return "Route from " + ConfigCheck::getRouteName(id).first +
        " to: " + ConfigCheck::getRouteName(id).second;
    }
  }

  /// This stuff is here to pull SystemCVPCSimulator.cpp into the link.
  class SystemCVPCSimulator;
  extern SystemCVPCSimulator systemCVPCSimulator;
  static __attribute__ ((unused))
  SystemCVPCSimulator *pSimulator = &systemCVPCSimulator;

}

