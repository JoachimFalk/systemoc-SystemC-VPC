/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Director.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_PCB
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC{

  FunctionTiming::FunctionTiming( )
    : funcDelays(1, SC_ZERO_TIME),
      funcLatencies(1, SC_ZERO_TIME)
  {
    setBaseDelay(SC_ZERO_TIME);
    setBaseLatency(SC_ZERO_TIME);
  }

  FunctionTiming::FunctionTiming( const FunctionTiming &delay )
    : funcDelays(    delay.funcDelays    ),
      funcLatencies( delay.funcLatencies )
  {
    setBaseDelay(   delay.getBaseDelay()   );
    setBaseLatency( delay.getBaseLatency() );
  }

  void FunctionTiming::addDelay( FunctionId fid,
                                              sc_time delay ){
    DBG_OUT( "::addDelay(" << fid << ") " << delay
             << std::endl);
    if( fid >= funcDelays.size()){
      funcDelays.resize( fid + 100, SC_ZERO_TIME );
    }
    this->funcDelays[fid] = delay;
  }

  void FunctionTiming::setBaseDelay( sc_time delay ){
    DBG_OUT( "::setBaseDelay() " << delay
             << std::endl);
    this->funcDelays[defaultFunctionId] = delay;
  }

  sc_time FunctionTiming::getBaseDelay( ) const {
    return this->funcDelays[defaultFunctionId];
  }

  sc_time summarizeFunctionTimes(const FunctionIds& functions,
      const FunctionTimes& functionTimes){

	  size_t fs = functions.size();
	  size_t fts = functionTimes.size();


    sc_time ret = SC_ZERO_TIME;
    for(FunctionIds::const_iterator iter = functions.begin();
        iter != functions.end();
        ++iter) {
      FunctionId fid = *iter;
      assert(fid < functionTimes.size());
      ret += functionTimes[fid];
    }
    return ret;
  }

  sc_time FunctionTiming::getDelay(
    FunctionIds functions) const
  {
    if (functions.begin() == functions.end()){
      return getBaseDelay();
    }
    return summarizeFunctionTimes(functions, funcDelays);
  }

  void FunctionTiming::addLatency( FunctionId fid,
                                                        sc_time latency ){
    if( fid >= funcLatencies.size())
      funcLatencies.resize( fid + 100, SC_ZERO_TIME );

    this->funcLatencies[fid] = latency;
  }

  void FunctionTiming::setBaseLatency( sc_time latency ){
    this->funcLatencies[defaultFunctionId] = latency;
  }

  sc_time FunctionTiming::getBaseLatency( ) const {
    return this->funcLatencies[defaultFunctionId];
  }

  sc_time FunctionTiming::getLatency(
    FunctionIds functions) const
  {
    if (functions.begin() == functions.end()){
      return getBaseLatency();
    }
   return summarizeFunctionTimes(functions, funcLatencies);
  }

  void FunctionTiming::setTiming(const Config::Timing& timing){
    this->addDelay(timing.getFunctionId(),   timing.getDii());
    this->addLatency(timing.getFunctionId(), timing.getLatency());
  }

  /**
   * SECTION ProcessControlBlock
   */
  
  
  ProcessControlBlock::ProcessControlBlock( AbstractComponent * component )
    : name("NN"), component(component) {
    this->init();
  }

  void ProcessControlBlock::init(){

    this->deadline = sc_time(DBL_MAX, SC_SEC);
    this->period = sc_time(DBL_MAX, SC_SEC);
    this->priority = 0;
    this->traceSignal = NULL;
  }

  void ProcessControlBlock::configure(std::string name, bool tracing,
      bool vcd){
    this->name = name;
    taskTracer =
        CoSupport::Tracing::TracingFactory::getInstance().createTaskTracer(name,
            component->getName());
#ifndef NO_VCD_TRACES
    if (vcd){
      this->setTraceSignal(component->addToTraceFile(name));
    }
#endif //NO_VCD_TRACES
  }

  std::string const& ProcessControlBlock::getName() const{
    return this->name;
  }

  void ProcessControlBlock::setPid( ProcessId pid){
    this->pid=pid;
  }

  ProcessId ProcessControlBlock::getPid( ) const{
    return this->pid;
  }
      
  void ProcessControlBlock::setFunctionId( FunctionId fid){
    this->fid=fid;
  }

  FunctionId ProcessControlBlock::getFunctionId( ) const{
    return this->fid;
  }

  const char* ProcessControlBlock::getFuncName() const{
    assert(0);
    return "";
  }

  void ProcessControlBlock::setPeriod(sc_time period){
    this->period = period;
  }

  sc_time ProcessControlBlock::getPeriod() const{
    return this->period;
  }

  void ProcessControlBlock::setPriority(int priority){
    this->priority = priority;
  }

  int ProcessControlBlock::getPriority() const{
    return this->priority;
  }

  void ProcessControlBlock::setDeadline(sc_time deadline){
    if(deadline > SC_ZERO_TIME){
      this->deadline = deadline;
    }else{
      this->deadline = SC_ZERO_TIME;
    }
  }

  sc_time ProcessControlBlock::getDeadline() const{
    return this->deadline;
  }

  void ProcessControlBlock::setTraceSignal(Tracing* signal){
    this->traceSignal = signal;
  }

  Tracing* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  void ProcessControlBlock::setTiming(const Config::Timing& timing){
    const PowerMode *mode = this->component->translatePowerMode(timing.getPowerMode());
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setTiming(timing);
  }

  void ProcessControlBlock::setBaseDelay(sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseDelay(delay);
  }

  void ProcessControlBlock::setBaseLatency(sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseLatency(latency);
  }

  void ProcessControlBlock::addDelay(FunctionId fid, sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addDelay(fid, delay);
  }

  void ProcessControlBlock::addLatency(FunctionId fid, sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addLatency(fid, latency);
  }

}
