#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#include <systemc.h>

#include <string>
#include <map.h>

//#include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {

  // for debugging purposes
//#define DELETE std::cerr << "delete called in " << __FILE__ << " at line " << __LINE__ << std::endl; delete

  using std::string;

  // set for debugging output
  //#define VPC_DEBUG true;

#define VPC_MAX_STRING_LENGTH 128

  /**********************************
   * SECTION Tags config file
   **********************************/

#define STR_VPC_MEASURE_FILE "measure.xml"
#define STR_VPC_RESULT_FILE "result"
#define STR_VPC_CONF_FILE "config"


  /**********************************
   * SECTION task states
   **********************************/

#define S_BLOCKED 'b'
#define S_READY   'w'
#define S_RUNNING 'R'
  // extension definition for tracing task state
#define S_SUSPENDED 's'
#define S_KILLED 'k'

  // definition for tracing configurations state
#define S_ACTIV 'A'
#define S_PASSIV 'P'
#define S_CONFIG 'c'


  // definition for hiding cosupport as vpc_event
  typedef CoSupport::SystemC::Event VPC_Event;
  typedef CoSupport::SystemC::EventOrList VPC_EventOrList;


  /***********************************
   * SECTION component types
   ***********************************/

#define STR_VPC_THREADEDCOMPONENTSTRING "threaded"
#define STR_VPC_COMPONENTSTRING "normal"
#define STR_VPC_RECONFIGURABLECOMPONENTSTRING "reconfigurable"

  /***********************************
   * SECTION controller types
   ***********************************/

#define STR_VPC_CONTROLLER  "controller"

  /***********************************
   * SECTION binder types
   ***********************************/

  // definition for SimpleBinder instance
#define STR_VPC_SIMPLEBINDER "SimpleBinder"
#define STR_VPC_SB           "SB"
  // definition for RoundRobinBinder instance
#define STR_VPC_RRBINDER     "RoundRobin"
#define STR_VPC_RRB          "RR"
//definition for PriorityBinder using LeastCurrentlyBoundPE
#define STR_VPC_LCBBINDER    "LeastCurrentlyBoundBinder"
#define STR_VPC_LCBB         "LCBB"
//definition for PriorityBinder using LeastCurrentlyBoundPE
#define STR_VPC_LFBBINDER    "LeastFrequentlyUsedBinder"
#define STR_VPC_LFBB         "LFUB"
 
  /***********************************
   * SECTION Mapper
   ***********************************/

  //definition yet used as no special mapper is implemented
#define STR_NONE  "NONE"
  
  /***********************************
   * SECTION Scheduler
   ***********************************/

#define STR_ROUNDROBIN "RoundRobin"
#define STR_RR "RR"
#define STR_PRIORITYSCHEDULER "PriorityScheduler"
#define STR_PS "PS"
#define STR_RATEMONOTONIC "RateMonotonic"
#define STR_RM "RM"
#define STR_FIRSTCOMEFIRSTSERVE "FirstComeFirstServe"
#define STR_FCFS "FCFS"
  // definition for EarliestDeadlineFirst scheduler
#define STR_EARLIESTDEADLINEFIRST "EarliestDeadlineFirst"
#define STR_EDF "EDF"

  /***********************************
   * SECTION attribute values
   ***********************************/

#define STR_VPC_DELAY "delay"
#define STR_VPC_PRIORITY "priority"
#define STR_VPC_PERIOD "period"
#define STR_VPC_DEADLINE "deadline"

  /***********************************
   * SECTION Defs for debugging
   ***********************************/

#define RED(str) "\e[31;1m" <<str<< "\e[0m"
#define GREEN(str) "\e[32;1m" <<str<< "\e[0m"
#define YELLOW(str) "\e[33;1m" <<str<< "\e[0m" 
#define BLUE(str) "\e[34;1m" <<str<< "\e[0m"
#define WHITE(str) "\e[37;40m" <<str<< "\e[0m"
#define VPC_ERROR __FILE__<<":"<<__LINE__<<"\e[1;31;40mVPC: ERROR> " 
#define NORMAL "\e[0m"
#define NENDL "\e[0m"<<endl;

  /***********************************
   * SECTION Def for Adaption to Modes
   ***********************************/

#ifdef MODES_EVALUATOR
#define  NO_VCD_TRACES
#endif // MODES_EVALUATOR


  enum action_command { ASSIGN,RESIGN,BLOCK,READY};

  typedef struct{
    int target_pid;
    action_command command;
  }action_struct;

} // namespace SystemC_VPC
#endif // HSCD_VPC_P_STRUCT_H
