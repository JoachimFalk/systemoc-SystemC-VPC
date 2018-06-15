// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_TASK_HPP
#define _INCLUDED_SYSTEMCVPC_TASK_HPP

#include <sstream>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include "TaskPool.hpp"
#include "FunctionTiming.hpp"
#include "tracing/TracerIf.hpp"

namespace SystemC_VPC {

  class ProcessControlBlock;

  using CoSupport::SystemC::Event;

  // Class representing a task instance, i.e., one execution of the task represented by the PCB.
  class TaskInstance
    : public Tracing::TTaskInstanceHolder {
    friend class NonPreemptiveComponent;
    friend class RoundRobinComponent;
    friend class AssociativePrototypedPool<ProcessId, TaskInstance>;
    friend class PrototypedPool<TaskInstance>;
  public:
    TaskInstance(TaskPool *pool);

    // getter, setter
    std::string getName() const                          {return name;}
    void        setName(std::string name)                {this->name = name;}
    ProcessId   getProcessId()                           {return pid;}
    void        setProcessId(ProcessId pid)              {this->pid = pid;}
    FunctionIds getFunctionIds()                         {return fid;}
    void        setFunctionIds(FunctionIds fid)          {this->fid = fid;}
    FunctionIds getGuardIds()                            {return gid;}
    void        setGuardIds(FunctionIds gid)             {this->gid = gid;}
    EventPair   getBlockEvent()                          {return blockEvent;}
    void        setBlockEvent(EventPair p)               {this->blockEvent = p;}
    ProcessControlBlock *getPCB()                        {return this->pcb;}
    void        setPCB(ProcessControlBlock *pcb)         {this->pcb = pcb;}
    void        setTiming(FunctionTimingPtr timing)      {this->timing = timing;}

    void       ackBlockingCompute(){
      blockAck = true;
      blockingCompute->notify();
    }
    void       abortBlockingCompute(){
      blockAck = false;
      blockingCompute->notify();
    }

    void       resetBlockingCompute(){this->setBlockingCompute(NULL);}
    void       setBlockingCompute(Coupling::VPCEvent::Ptr blocker)
      { blockingCompute = blocker; }
    bool       isBlocking()
      { return blockingCompute != NULL; }
    bool       isAckedBlocking()
      { return blockAck; }
    void       setExec( bool exec ) {this->exec=exec;}
    bool       isExec(  ) { return this->exec;}
    void       setWrite( bool write ) {this->write=write;}
    bool       isWrite(  ) { return this->write;}

    //////////////////////////////
    // Begin from Simone Mueller //
    //////////////////////////////

    void setFactorOverhead(int complexity)
      { this->factorOverhead = complexity; }
    int getFactorOverhead() const
      { return this->factorOverhead; }

    void setOverhead(const sc_core::sc_time &overhead)
      { this->overhead = overhead; }
    sc_core::sc_time getOverhead() const
      { return this->overhead; }
    //////////////////////////////
    // End from Simone Mueller //
    //////////////////////////////

    void setDelay(const sc_core::sc_time& delay)         {this->delay = delay;}
    sc_core::sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_core::sc_time& latency)     {this->latency = latency;}
    sc_core::sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_core::sc_time& delay){this->remainingDelay = delay;}
    sc_core::sc_time getRemainingDelay() const           {return this->remainingDelay;}
    int getInstanceId() const                   {return this->instanceId;}
    void setTimingScale( double scale )         {this->timingScale = scale;}
    double getTimingScale()                     {return this->timingScale;}

    void setScheduledTask(TaskInterface * st)
      {this->scheduledTask = st;}
    TaskInterface * getScheduledTask()
      {return this->scheduledTask;}
    bool hasScheduledTask() const
      {return this->scheduledTask != NULL;}

    /**
     * 
     */
    void initDelays();

    // Adaptor setter / getter for ProcessControlBlock
    int              getPriority() const;

    sc_core::sc_time getPeriod() const;

    bool             isPSM() const;

    void release() {
      //this->setBlockEvent(EventPair(NULL, NULL));
      this->getBlockEvent().dii     = NULL;
      this->getBlockEvent().latency = NULL;
      pool->free(this->getProcessId(), this);
    }

    ~TaskInstance();
  private:
    void traceReleaseTask();
    void traceFinishTaskLatency();

    static int globalInstanceId;

    int instanceId;

    ProcessId        pid;
    FunctionIds      fid;
    FunctionIds      gid;
    EventPair        blockEvent;

    Coupling::VPCEvent::Ptr blockingCompute;
    bool       blockAck;
    bool       exec;
    bool       write;

    sc_core::sc_time startTime;
    sc_core::sc_time endTime;
    sc_core::sc_time blockingTime;
    sc_core::sc_time delay;
    sc_core::sc_time latency;
    sc_core::sc_time remainingDelay;
    sc_core::sc_time overhead;
    
    int factorOverhead;

    FunctionTimingPtr    timing;
    ProcessControlBlock *pcb;
    TaskPool            *pool;

    std::string name;
    double timingScale;
    CoSupport::Tracing::TaskTracer::Ticket taskTracerTicket;
    TaskInterface * scheduledTask;
  };

  static inline
  TaskInstance *getTaskOfTaskInterface(TaskInterface const *task)
    { return reinterpret_cast<TaskInstance *>(task->getSchedulerInfo()); }

  typedef std::map<int, TaskInstance*>  TaskMap;

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
    TaskInstance *task;
    p_queue_entry(int fifo_order, TaskInstance *task) :
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

  struct timePcbPair{
    sc_core::sc_time time;
    TaskInstance *task;

    bool operator<(const timePcbPair& right) const
    {
      if (time > right.time)
        return true;
      else
        return false;
    }

  };

}
#endif /* _INCLUDED_SYSTEMCVPC_TASK_HPP */