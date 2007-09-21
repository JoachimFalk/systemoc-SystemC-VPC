/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_Director.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <iostream>

#include <hscd_vpc_Director.h>
#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Term.h>
#include <hscd_vpc_VPCBuilder.h>

#include <systemc.h>
#include <map>

namespace SystemC_VPC{

  //
  std::auto_ptr<Director> Director::singleton(new Director());

  //
  Director& Director::getResource( const char* name){
    return *(this->singleton);
  }

  /**
   *
   */
  Director::Director()
    : FALLBACKMODE(false),
      mappings(),
      end(0),
      componentIdMap(),
      globalProcessId(0)
  {
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
  void Director::checkConstraints() {
    std::vector<Constraint*>::const_iterator iter=constraints.begin();
    for(;iter!=constraints.end();iter++){
      (*iter)->isSatisfied();
    }
  }

  /**
   *
   */
  void Director::getReport(){
    std::vector<Constraint*>::const_iterator iter=constraints.begin();
    char *cons_name;
    double start=-1;
    double end=-1;
    // if there are any constaints to be viewed calculate time
    if(this->constraints.size() > 0){
      for(;iter!=constraints.end();iter++){
        cons_name=(*iter)->getName();
        if(0==strncmp("start",cons_name,5))
          start=(*iter)->getSatisfiedTime();
        else if(0==strncmp("end",cons_name,3))
          end=(*iter)->getSatisfiedTime();
      }
    }else{ // else take total simulation time
      start = 0;
      end = this->end;
    }
#ifdef VPC_DEBUG
    std::cerr << "start: " << start << " end: " << end  << std::endl;
#endif //VPC_DEBUG
    if ((start != -1) && (end != -1)){
      if(0 != this->vpc_result_file.compare("")){

#ifdef VPC_DEBUG
        std::cerr << "Director> result_file: "
                  << this->vpc_result_file << std::endl;
#endif //VPC_DEBUG
        ofstream resultFile;
        resultFile.open(this->vpc_result_file.c_str());
        if(resultFile){
          resultFile << (end-start);
        }
        resultFile.flush();
        resultFile.close();
      }else{
        std::cerr << "latency: " << end - start << std::endl;
      }
    }
  }

  /**
   *
   */
  Director::~Director(){

    getReport();
    
    // clear components
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it ){
      delete *it;
    }
    
    componentIdMap.clear();
  }

  //
  ProcessControlBlock* Director::getProcessControlBlock( const char *name ){
    ProcessId pid = getProcessId(name);
    return this->getProcessControlBlock( pid );
  }

  //
  ProcessControlBlock* Director::getProcessControlBlock( ProcessId pid ){

    assert(!FALLBACKMODE);

    try{
      return this->pcbPool.allocate( pid );
    }catch(NotAllocatedException& e){
      std::cerr << "Director> getProcessControlBlock failed due to"
                << std::endl << e.what() << std::endl;
      std::cerr << "HINT: probably actor binding not specified in"
                << " configuration file!" << std::endl;
      exit(-1);
    }

  }

  //
  PCBPool& Director::getPCBPool(){
    return this->pcbPool;
  }


  //
  void Director::compute( FastLink fLink,
                          EventPair endPair ){

    if(FALLBACKMODE){
      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end

      // do nothing, just return
      return;
    }


    ProcessControlBlock* pcb = this->getProcessControlBlock(fLink.process);
    pcb->setFunctionId(fLink.func);
    
    int lockid = -1;
    
    //HINT: also treat mode!!
    //if( endPair.latency != NULL ) endPair.latency->notify();

    if( endPair.dii == NULL ){
      // prepare active mode
      pcb->setBlockEvent(EventPair(new VPC_Event(), new VPC_Event()));
      // we could use a pool of VPC_Events instead of new/delete
      lockid = this->pcbPool.lock(pcb);
    }else{
      // prepare passiv mode
      pcb->setBlockEvent(endPair);
    }

    
    if (mappings.size() < fLink.process ||
        mappings[fLink.process] == NULL) {
      cerr << "Unknown mapping <" << pcb->getName() << "> to ??" << std::endl;
      
      assert(mappings.size() >= fLink.process &&
             mappings[fLink.process] != NULL);
    }
    
    // get Component
    AbstractComponent* comp = mappings[fLink.process];

    // compute task on found component
    assert(!FALLBACKMODE);
    comp->compute(pcb);

    if( endPair.dii == NULL){
      // active mode -> waits until simulated delay time has expired
      
      CoSupport::SystemC::wait(*(pcb->getBlockEvent().dii));
      delete pcb->getBlockEvent().dii;
      delete pcb->getBlockEvent().latency;
      pcb->setBlockEvent(EventPair());
      // as psb has been locked -> unlock it
      this->pcbPool.unlock(pcb->getPid(), lockid);
      // and free it
      this->pcbPool.free(pcb);
    }
    
  }

  void Director::compute(const char* name,
                         const char* funcname,
                         VPC_Event* end)
  {
    //HINT: treat mode!!
    //if (mode) { ....
    compute(name, funcname, EventPair(end, NULL));
    //} else{
    //  compute(name, funcname, EventPair(NULL, end));
    //}
  }

  void Director::compute(const char* name,
                         const char* funcname,
                         EventPair endPair)
  {
    this->compute( this->getFastLink(name, funcname),
                   endPair );
  }

  //
  void Director::compute(const char *name, EventPair endPair){
    compute( name, "", endPair);
  }

  //
  void Director::compute(const char* name, VPC_Event* end){
    this->compute(name, "", end);
  }
    
  /**
   * \brief Implementation of Director::addConstraint
   */
  void Director::addConstraint(Constraint* cons){
    this->constraints.push_back(cons);
  }

  /**
   * \brief Implementation of Director::registerComponent
   */
  void Director::registerComponent(AbstractComponent* comp){
    ComponentId cid = comp->getComponentId();
    if(cid >= components.size())
      components.resize(cid+100, NULL);

    this->componentIdMap[comp->basename()] = cid;

    this->components[cid] = comp;

#ifdef VPC_DEBUG
    cerr << " Director::registerComponent(" << comp->basename()
         << ") [" << comp->getComponentId() << "] # " << components.size()
         << endl;
#endif //VPC_DEBUG
  }
    
  /**
   * \brief Implementation of Director::registerMapping
   */
  void Director::registerMapping(const char* taskName, const char* compName){
    assert(!FALLBACKMODE);

    ProcessId       pid = getProcessId( taskName );
    if( pid >= mappings.size() ){
      mappings.resize( pid + 100, NULL );
    }

    assert(pid <= mappings.size());
    
    ComponentId cid = this->getComponentId(compName);

    AbstractComponent * comp = components[cid];

    assert( comp != NULL );
    mappings[pid] = comp;
  }
    
  /**
   * \brief Implementation of  Director::generatePCB
   */
  ProcessControlBlock& Director::generatePCB(const char* name){
    assert(!FALLBACKMODE);

    //cerr << "generatePCB( " << name << ")" << endl;

    ProcessId       pid = getProcessId( name );
    
    ProcessControlBlock& pcb = this->pcbPool.registerPCB( pid );
    pcb.setName(name);
    pcb.setPid( pid );
    return pcb;
  }

  /**
   * \brief Implementation of Director::signalProcessEvent
   */
  void Director::signalProcessEvent(ProcessControlBlock* pcb){
    assert(!FALLBACKMODE);

#ifdef VPC_DEBUG
    std::cerr << "Director> got notified from: " << pcb->getName()
              << std::endl;
#endif //VPC_DEBUG
    if(pcb->getState() != activation_state(aborted)){
#ifdef VPC_DEBUG
      std::cerr << "Director> task successful finished: " << pcb->getName()
                << std::endl;
#endif //VPC_DEBUG
      if(NULL != pcb->getBlockEvent().latency)
        pcb->getBlockEvent().latency->notify();
      // remember last acknowledged task time
      this->end = sc_time_stamp().to_default_time_units();
      
      // free allocated pcb
      this->pcbPool.free(pcb);
    }else{
#ifdef VPC_DEBUG
      std::cerr << "Director> re-compute: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
      // get Component
      AbstractComponent* comp = mappings[pcb->getPid()];
      comp->compute(pcb);
    }
    wait(SC_ZERO_TIME);
  }


  ProcessId Director::uniqueProcessId() {
    return globalProcessId++;
  }

  ProcessId Director::getProcessId(std::string process) {
    ProcessIdMap::const_iterator iter = processIdMap.find(process);
    if( iter == processIdMap.end() ) {
      processIdMap[process] = this->uniqueProcessId();
    }
    iter = processIdMap.find(process);
    return iter->second;
  }

  ComponentId Director::getComponentId(std::string component) {
#ifdef VPC_DEBUG
    cerr << " Director::getComponentId(" << component
         << ") # " << componentIdMap.size()
         << endl;
#endif //VPC_DEBUG

    ComponentIdMap::const_iterator iter = componentIdMap.find(component);
    assert( iter != componentIdMap.end() );
    return iter->second;
      
  }


  FastLink Director::getFastLink(std::string process, std::string function) {
    //cerr << "getFastLink( " << process << ", " << function << ")" << endl;
    if(FALLBACKMODE) return FastLink();
    assert(!FALLBACKMODE);

    ProcessId       pid = getProcessId(  process  );

    ProcessControlBlock* pcb = getProcessControlBlock( pid );
    FunctionId           fid = pcb->DelayMapper::getFunctionId( function );

    // pcb has been allocated by calling "getProcessControlBlock"-> free it
    this->pcbPool.free(pcb);

    return FastLink(pid, fid);
  }

}

