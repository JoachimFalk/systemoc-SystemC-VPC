#ifndef HSCD_VPC_FCFSSCHEDULER_H
#define HSCD_VPC_FCFSSCHEDULER_H
#include <systemc.h>
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map>
#include <deque>
namespace SystemC_VPC{

  class Component;

  class FCFSScheduler : public Scheduler{
  public:

    FCFSScheduler(const char *schedulername){
    }
    FCFSScheduler(){
    }
    virtual ~FCFSScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,const std::map<int,ProcessControlBlock*> &ready_tasks,const std::map<int,ProcessControlBlock*> &running_tasks);
    void addedNewTask(ProcessControlBlock *pcb);
    void removedTask(ProcessControlBlock *pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  std::map<int,ProcessControlBlock*> &ready_tasks,const  std::map<int,ProcessControlBlock*> &running_tasks);
    sc_time* schedulingOverhead(){return 0;}//new sc_time(1,SC_NS);
  protected:

    std::deque<int> fcfs_fifo;
    //  double TIMESLICE;
  };
}
#endif