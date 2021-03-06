// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_SCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_SCHEDULER_HPP

#include "../TaskInstanceImpl.hpp"

#include <systemcvpc/Attribute.hpp>

#include <systemc>

namespace SystemC_VPC { namespace Detail {
  
  enum scheduling_decision {ONLY_ASSIGN // no old task, assign new task only
          ,PREEMPT    // preempt running task
          ,RESIGNED   // finished running task, no new task
          ,NOCHANGE}; // keep unchanged

  /**
   * \brief A call-back class called from Component to do Scheduling.
   *
   * Main part is virtual function scheduling_decision schedulingDecision(int&, int&, const TaskMap &, const TaskMap &)
   */
  class Scheduler{
  public:
    virtual ~Scheduler() {};

    /**
     * /brief Called from Component to determine a "time slice" used as time out.
     * 
     */
    virtual bool
    getSchedulerTimeSlice(sc_core::sc_time &time,
                          const TaskMap &ready_tasks,
                          const TaskMap &running_tasks)=0;

    /**
     * \brief Inform Scheduler about new tasks.
     */
    virtual void addedNewTask(TaskInstanceImpl *task)=0;
    
    /**
     * \brief Inform Scheduler about removed tasks.
     */
    virtual void removedTask(TaskInstanceImpl *task)=0;

    /**
     * \brief Call the Scheduler to do a scheduling decision.
     *
     * The tasks to resign and to assign have to be calculated.
     * \param [out] task_to_resign The task that have to be resigned.
     * \param [out] task_to_assign The task that have to be assigned.
     * \param [in] ready_tasks A map of ready tasks! Component knows this map.
     * \param [in] running_tasks A map of running tasks! Usually only one! Component knowes this map.
     * \return Returns a scheduling_decision enum. So Component knows what he has to do.
     */
    virtual scheduling_decision
    schedulingDecision(int& task_to_resign,
                       int& task_to_assign,
                       const TaskMap &ready_tasks,
                       const TaskMap &running_tasks)=0;

    /**
     *\brief The overhead needed to determine the scheduling decision.
     */
    virtual sc_core::sc_time* schedulingOverhead()=0;

    /**
     *\brief Customize scheduler options, like time slice or scheduling overhead.
     *
     * Does nothing by default.
     */
    virtual void setProperty(const char* key, const char* value){}
    
    virtual void setAttribute(Attribute const &attr)
    {
      if(!attr.getAttributes().empty()) {
        return;
      }
      this->setProperty(attr.getType().c_str(), attr.getValue().c_str());
    }

    virtual void initialize(){}

    virtual bool addStream(ProcessId pid) {
      return false;
    }

    virtual bool closeStream(ProcessId pid) {
      return false;
    }
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_SCHEDULER_HPP */
