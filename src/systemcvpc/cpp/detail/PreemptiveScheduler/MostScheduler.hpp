// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"
#include "MostSecondaryScheduler.hpp"

#include <systemc>

#include <map>
#include <deque>
#include <string.h>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;

  typedef size_t ProcessId;

  struct MostSlot{
    sc_core::sc_time length;
    ProcessId process;
    int Id;
    std::string name;
  };
  
  class MostScheduler : public Scheduler{
  public:
    
    MostScheduler()
      : secondaryScheduler() {

      slicecount = 0;
      streamcount = 0;
      lastassign = sc_core::SC_ZERO_TIME;
      this->remainingSlice = sc_core::SC_ZERO_TIME;
      curr_slicecount = -1;
      sysFreq = 48000;
      cycleSize = 372;
      std::map<sc_core::sc_time, unsigned int> IDmap;

      currSlotStartTime = sc_core::sc_time(0, sc_core::SC_NS);
    }
    
    MostScheduler(const char *schedulername);
    
    
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);

    sc_core::sc_time setboundary(int sysFreq,int framesize);

    sc_core::sc_time cycle(int sysFreq);

    bool area(int sysFreq,int framesize);
 
    void addedNewTask(TaskInstanceImpl *task);
    
    void removedTask(TaskInstanceImpl *task);
    
    sc_core::sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    
    sc_core::sc_time* schedulingOverhead(){
      return NULL;
    }
    
    void initialize(){}

    bool addStream(ProcessId pid);
    bool closeStream(ProcessId pid);

  private:

    
    std::map<sc_core::sc_time, unsigned int> slotOffsets;

    std::map<ProcessId, bool> areaMap;

    MostSecondaryScheduler secondaryScheduler;

    sc_core::sc_time lastassign;
    sc_core::sc_time remainingSlice;
    int slicecount;
    int curr_slicecount;
    int curr_slicecount_help;
    bool already_avail;
    std::deque<MostSlot> Most_slots;
    int sysFreq;
    int cycleSize;
    int streamcount;
    bool flag;
    sc_core::sc_time currSlotStartTime;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP */
