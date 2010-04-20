
#ifndef TDMASCHEDULER_H
#define TDMASCHEDULER_H
#include <systemc.h>
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map>
#include <deque>

namespace SystemC_VPC{
  class Component;

  typedef size_t ProcessId;
  typedef std::deque< std::pair <std::string, std::string> > Properties;
  /*Dient zur Speicherung der TDMA-Zeitschlitz - Daten
    pid_fifo enthaelt die laufbereiten Prozesse  
  */
  struct TDMASlot{
    sc_time length;
    std::string name;
    std::deque<int> pid_fifo;
  };
  
  class TDMAScheduler : public Scheduler{
  public:
    
    TDMAScheduler()
      : tdmaCycle(SC_ZERO_TIME) {
      this->lastassign=sc_time(0,SC_NS);
      this->remainingSlice=sc_time(0,SC_NS);
      slicecount=0;
      curr_slicecount=0;
    }
    
    TDMAScheduler(const char *schedulername);
    
    virtual ~TDMAScheduler(){}
    
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    
    void addedNewTask(Task *task);
    
    void removedTask(Task *task);
    
    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks
                                           ,const  TaskMap &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    virtual void setAttribute(AttributePtr attPtr){
      if(attPtr->getAttributeSize() != 0 ||  attPtr->getParameterSize() != 0)
        return;

      this->setProperty(attPtr->getType().c_str(), attPtr->getValue().c_str());

    }

    sc_time* schedulingOverhead();
    
    void initialize();
    
  private:
    void _setProperty(const char* key, const char* value);
    
    sc_time  tdmaCycle;
    std::map<sc_time, unsigned int> slotOffsets;

    sc_time lastassign;
    sc_time remainingSlice;
    int slicecount;
    int curr_slicecount;
    int processcount;
    std::vector<TDMASlot> TDMA_slots;
    std::map <ProcessId,int> PIDmap;
    std::deque<std::pair<std::string, std::string> > _properties;
  };
}
#endif