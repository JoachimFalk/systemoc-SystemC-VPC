// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCEIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCEIMPL_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/Extending/TaskInstance.hpp>
#include <systemcvpc/datatypes.hpp>

#include "TaskImpl.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <functional>

namespace SystemC_VPC { namespace Detail {

  class ObservableComponent;
  class AbstractExecModel;
  class AbstractComponent;

//using CoSupport::SystemC::Event;

  // Class representing a task instance, i.e., one execution of a task.
  class TaskInstanceImpl
    : public Extending::TaskInstance
  {
    typedef Extending::TaskInstance base_type;
  public:

    // Getters
    int             getInstanceId() const
      { return instanceId; }
    TaskImpl       *getTask() const
      { return static_cast<TaskImpl *>(this->base_type::getTask()); }
    PossibleAction *getFiringRule() const
      { return firingRuleInterface; }

    void diiExpired() { diiCallback(this); }
    void latExpired() { latCallback(this); }

    // Getters forwarding to taskImpl
    int              getPriority() const;
    sc_core::sc_time getPeriod() const;
    ProcessId        getProcessId() const;
    bool             isPSM() const;

  private:
    friend class ObservableComponent; // To access constructor and destructor.
    friend class AbstractExecModel;   // To access setDelay, setLatency, and setPower.
    friend class AbstractComponent;   // To access setDelay, setLatency, and setRemainingDelay.

    TaskInstanceImpl(
        TaskImpl                                       *taskImpl
      , Type                                            type
      , PossibleAction                                 *firingRuleInterface
      , std::function<void (TaskInstanceImpl *)> const &diiCallback
      , std::function<void (TaskInstanceImpl *)> const &latCallback);

    ~TaskInstanceImpl();

    static int globalInstanceId;

    int             instanceId;
    PossibleAction *firingRuleInterface;

    std::function<void (TaskInstanceImpl *)> const diiCallback;
    std::function<void (TaskInstanceImpl *)> const latCallback;
  };

  typedef std::map<int, TaskInstanceImpl*>  TaskMap;

  template<typename PAYLOAD>
  struct PriorityFcfsElement {
    int     priority;
    size_t  fcfsOrder;
    PAYLOAD payload;

    PriorityFcfsElement(int priority, size_t fcfsOrder, PAYLOAD payload) :
      priority(priority), fcfsOrder(fcfsOrder), payload(payload)
    {
    }

    bool operator<(const PriorityFcfsElement<PAYLOAD>& other) const
    {
      int p1=priority;
      int p2=other.priority;
      // lesser value means higher priority
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (fcfsOrder>other.fcfsOrder);
      else
        return false;
    }
  };

  struct p_queue_entry{
    int fifo_order;  // secondary scheduling policy
    TaskInstanceImpl *task;
    p_queue_entry(int fifo_order, TaskInstanceImpl *task) :
      fifo_order(fifo_order), task(task)
    {
    }

    //Threads with lesser priority value have higher priority
    bool operator<(const p_queue_entry& other) const
    {
      int p1=task->getPriority();
      int p2=other.task->getPriority();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (fifo_order>other.fifo_order);
      else
        return false;
    }

  };

  typedef std::map<int, TaskInstanceImpl *>  TaskMap;

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCEIMPL_HPP */
