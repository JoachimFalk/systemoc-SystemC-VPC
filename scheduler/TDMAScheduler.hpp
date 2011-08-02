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

#ifndef TDMASCHEDULER_H
#define TDMASCHEDULER_H
#include <systemcvpc/datatypes.hpp>
#include "Scheduler.hpp"

#include <systemc.h>

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
    
//    TDMAScheduler()
//      : tdmaCycle(SC_ZERO_TIME) {
//      this->lastassign=sc_time(0,SC_NS);
//      this->remainingSlice=sc_time(0,SC_NS);
//      slicecount=0;
//      curr_slicecount=0;
//    }
    
    TDMAScheduler();
    
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