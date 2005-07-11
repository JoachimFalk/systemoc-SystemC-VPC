/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Component.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include "hscd_vpc_Component.h"
#include "hscd_vpc_SchedulerProxy.h"
#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_Director.h"

namespace SystemC_VPC{
  void Component::compute( const char *name, sc_event *end) { 
    p_struct  actualTask = Director::getInstance().getProcessControlBlock(name);

#ifndef MODES_EVALUATOR
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask.name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask.name);
      trace_signal=(iter->second);
    }
    if (trace_signal != NULL ) {
      *trace_signal = READY;
    }
#endif //MODES_EVALUATOR

    //cerr<<"VPC says: PG node "<<name<<" start execution "<<sc_simulation_time()<<" on: "<<this->name<<std::endl;
    //wait((80.0*rand()/(RAND_MAX+1.0)), SC_NS);
    //wait(10, SC_NS);
    compute(actualTask);

#ifndef MODES_EVALUATOR
    if (trace_signal != NULL ) {
      *trace_signal = BLOCKED;
    }
#endif //MODES_EVALUATOR

      // std::cerr << "VPC says: PG node " << name << " stop execution " << sc_simulation_time() << std::endl;
  }

  /*void Component::compute(int process, sc_event *end){
    p_struct actualTask = Director::getInstance().getProcessControlBlock(process);
    compute(actualTask);
    }*/
  void Component::compute(p_struct actualTask){
    sc_event interupt;
    action_struct *cmd;
    double last_delta_start_time;
    double rest_of_delay;
    bool task_is_running=false;
    int process=actualTask.pid;

#ifndef MODES_EVALUATOR
    sc_signal<trace_value> *trace_signal=NULL;
    if(1==trace_map_by_name.count(actualTask.name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask.name);
      trace_signal=(iter->second);
    }
#endif //MODES_EVALUATOR


    actualTask.interupt = &interupt; 
    rest_of_delay=actualTask.delay;
    new_tasks[process]=actualTask;
    cmd = new action_struct;
    cmd->target_pid = process;
    cmd->command = ADD;
    open_commands.push_back(*cmd);               //          add

    sc_event& e = schedulerproxy->getNotifyEvent();
    notify(SC_ZERO_TIME,e);
    while(1){
      if(task_is_running){
	last_delta_start_time=sc_simulation_time();

#ifndef MODES_EVALUATOR
	if(trace_signal!=0)*trace_signal=RUNNING;     
#endif //MODES_EVALUATOR

	//   cout << actualTask.name << " is running! "<< sc_simulation_time() << endl; 
	wait(rest_of_delay,SC_NS,*actualTask.interupt);
	//cout << actualTask.name << " is stoped! "<< sc_simulation_time() << endl; 

#ifndef MODES_EVALUATOR
	if(trace_signal!=0)*trace_signal=READY;
#endif //MODES_EVALUATOR

	rest_of_delay-=sc_simulation_time()-last_delta_start_time;
	if(rest_of_delay==0){ // Process beim Scheduler abmelden
	  cmd=new action_struct;
	  cmd->target_pid = process;
	  cmd->command = RETIRE;
	  open_commands.push_back(*cmd);
	  notify(e);    //Muss auf den Scheduler gewarted werden? (Nein)
	  wait(SC_ZERO_TIME);
	  return;
	}else{}     //Scheduler fragen: Was ist los?

      }else{
	wait(*actualTask.interupt);  // Scheduler abwarten
	//Scheduler fragen: Was ist los?

      }
      //Scheduler fragen: Was ist los?
      action_struct *cmd;
      cmd=schedulerproxy->getNextNewCommand(process);
      if(NULL != cmd){
	switch(cmd->command){
	case RETIRE:
	  //cerr << "retire" << endl;
	  // Kann nicht sein
	  break;
	case ADD: 
	  //cerr << "add" << endl;
	  // Ok aber keine Information, besser: Nichts zu tun!
	  break;
	case ASSIGN:
	  //	cerr << "assign" << endl;
	  task_is_running=true;
	  break;
	case RESIGN:
	  //cerr << "resign" << endl;
	  task_is_running=false;
	  break;
	default:
	  //	cerr << "def" << endl;
	  break;
	}
      }
    }
    //new_tasks.erase(process);
  } 
  Component::Component(const char *name,const char *schedulername){
    strcpy(this->name,name);
    schedulerproxy=new SchedulerProxy(this->name);
    schedulerproxy->setScheduler(schedulername);
    schedulerproxy->registerComponent(this);

#ifndef MODES_EVALUATOR
    string tracefilename=this->name;
    char tracefilechar[VPC_MAX_STRING_LENGTH];
    char* traceprefix= getenv("VPCTRACEFILEPREFIX");
    if(0!=traceprefix){
      tracefilename.insert(0,traceprefix);
    }
    strcpy(tracefilechar,tracefilename.c_str());
    //cerr << "Trace: "<<tracefilechar <<endl;
    this->trace =sc_create_vcd_trace_file (tracefilechar);
    ((vcd_trace_file*)this->trace)->sc_set_vcd_time_unit(-9);
    //    this->trace_wif = sc_create_wif_trace_file (tracefilechar);
#endif //MODES_EVALUATOR

  }

  Component::~Component(){

#ifndef MODES_EVALUATOR
    sc_close_vcd_trace_file(this->trace);
#endif //MODES_EVALUATOR
    delete schedulerproxy;

  }
  void Component::informAboutMapping(string module){

#ifndef MODES_EVALUATOR
    sc_signal<trace_value> *newsignal=new sc_signal<trace_value>();
    trace_map_by_name.insert(pair<string,sc_signal<trace_value>*>(module,newsignal));
    sc_trace(this->trace,*newsignal,module.c_str());
    //    sc_trace(this->trace_wif,*newsignal,module.c_str());
#endif //MODES_EVALUATOR

  }

  vector<action_struct> &Component::getNewCommands() {
    return open_commands;
  }

  map<int,p_struct> &Component::getNewTasks() {
    return new_tasks;
  }
}