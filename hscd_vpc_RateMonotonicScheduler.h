#ifndef HSCD_VPC_RATEMONOTONICSCHEDULER_H
#define HSCD_VPC_RATEMONOTONICSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <queue.h>
#include <vector.h>

namespace SystemC_VPC{
  class Component;

  class RateMonotonicScheduler : public Scheduler{
  public:

    RateMonotonicScheduler(){
      order_counter=0;
    }
    RateMonotonicScheduler(const char *schedulername);
    virtual ~RateMonotonicScheduler(){}
    int getSchedulerTimeSlice(sc_time &time,const map<int,p_struct> &ready_tasks,const map<int,p_struct> &running_tasks);
    void addedNewTask(p_struct pcb);
    void removedTask(p_struct pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,p_struct> &ready_tasks,const  map<int,p_struct> &running_tasks);
    void setProperty(char* key, char* value);
  protected:
    int order_counter;
    rm_queue_compare comp;
    priority_queue<p_queue_entry,vector<p_queue_entry>,rm_queue_compare> pqueue;


  };
}
#endif