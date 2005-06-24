/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Component.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include "systemc.h"
#include "hscd_vpc_datatypes.h"
//#include "hscd_vpc_SchedulerProxy.h"
#include <vector.h>
#include <map.h>
namespace SystemC_VPC{
  class SchedulerProxy;


  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, sc_event) Funktion.
   */
  class AbstractComponent{
  public:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name, sc_event *end=NULL)=0;
    //    virtual void compute(int iprocess)=0;
    virtual ~AbstractComponent(){};
  };

  /**
   * \brief An implementation of AbstractComponent.
   * 
   * This is a "Virtual Component" used to simulate execution time and scheduling (also preemptive scheduling).
   * An event based communication to a Scheduler is realised, using the SchedulerProxy as counterpart.
   */
  class Component : public AbstractComponent{
  public:

    /**
     * \brief A map of tasks that newly called compute(const char *, sc_event).
     *
     * If a task calls compute he will noted down in a map. This funktion provides
     * access to this map.
     */
    map<int,p_struct> &getNewTasks();

    /**
     * \brief A vector of commandos, so the Scheduler can descide what to do.
     *
     * If a task calls compute, the command "ready" is generated. If the whole 
     * delay-time is delayed the command "block" is generated.
     */
    vector<action_struct> &getNewCommands();
    
    /**
     * \brief One implementation of AbstractComponent::compute(const char *, sc_event).
     */
    virtual void compute( const char *name, sc_event *end=NULL);
    //  virtual void compute(int process, sc_event *end=NULL);
    Component();
    /**
     * \brief Initialize a Component with a Scheduler.
     */
    Component(const char *name,const char *schedulername);
    virtual ~Component();
    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase.
     */
    virtual void informAboutMapping(string component);

  private:
    void compute(p_struct actualTask);
    map<string,sc_signal<trace_value>*> trace_map_by_name;
    map<int,p_struct>      new_tasks;
    vector<action_struct>  open_commands;
    sc_trace_file *trace;   ////////////////////////                              
    //  sc_trace_file *trace_wif;   ////////////////////////                              
    SchedulerProxy *schedulerproxy;
    //  int id;
    char name [VPC_MAX_STRING_LENGTH];
  };
  class FallbackComponent : public AbstractComponent{
  public:
    virtual void compute( const char *name, sc_event *end=NULL){}
    FallbackComponent(const char *name,const char *schedulername){}
    virtual ~FallbackComponent(){}
  private:
  };
}
#endif
