// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_AVBSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_AVBSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "PriorityScheduler.hpp"
#include "Scheduler.hpp"

#include <systemc>

#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;
  
  class AVBListEntry{
  public:
    AVBListEntry(){
      bw_alloc = 0;
      priority_level = 0;
      task_queue = 0;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
      wasEmpty_flag=true;
    }

    AVBListEntry(std::queue<TaskInstanceImpl*>* t_queue, int p_level, float b_alloc){
      bw_alloc = b_alloc;
      priority_level = p_level;
      task_queue = t_queue;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
      wasEmpty_flag=true;
    }

    float bw_alloc;
    sc_core::sc_time queue_credit;
    bool has_credit_flag;
    bool wasEmpty_flag;
    int priority_level;
    std::queue<TaskInstanceImpl*>* task_queue;

    int get_priority_level(void){
      return priority_level;
    }

    float get_bw_alloc(void){
      return bw_alloc;
    }

    void increment_credit(sc_core::sc_time increment){
      if(has_credit_flag){
        queue_credit += increment;
      }else{
        if(increment < queue_credit){
          queue_credit -= increment;
        }else{
          queue_credit = increment - queue_credit;
          has_credit_flag = true;
        }
      }
    }

    void decrement_credit(sc_core::sc_time decrement){
      if(has_credit_flag){
        if(decrement <= queue_credit){
          queue_credit -= decrement;
        }else{
          queue_credit = decrement - queue_credit;
          has_credit_flag = false;
        }
      }else{
        queue_credit += decrement;
      }
    }

    bool has_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
      return has_credit_flag;
      
    }

    void setWasEmpty(bool flag){
      wasEmpty_flag = flag;
    }

    bool wasEmpty(void){
      return wasEmpty_flag;
    }

    void reset_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
      has_credit_flag = true;
      queue_credit = sc_core::SC_ZERO_TIME;
      
    }

    sc_core::sc_time get_credit(void){
      if(has_credit_flag){
        return queue_credit;
      }else{
        return sc_core::SC_ZERO_TIME - queue_credit;
      }
    }

  };

  class AVBScheduler : public Scheduler{
  public:

    AVBScheduler();
    virtual ~AVBScheduler(){}
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(TaskInstanceImpl *task);
    void removedTask(TaskInstanceImpl *task);
    sc_core::sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    void setAttribute(Attribute const &attribute);

    sc_core::sc_time* schedulingOverhead(){return 0;}//;
  protected:
    std::list<AVBListEntry*> p_list;
    std::deque<std::pair<std::string, std::string> > _properties;
    int last_active;
    sc_core::sc_time time_last_assign;
  private:
    bool firstrun;

  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_AVBSCHEDULER_HPP */
