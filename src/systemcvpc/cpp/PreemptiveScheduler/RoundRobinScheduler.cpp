/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
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

#include <PreemptiveScheduler/RoundRobinScheduler.hpp>
#include <PreemptiveScheduler/PreemptiveComponent.hpp>
#include <systemcvpc/Director.hpp>


namespace SystemC_VPC{

  void RoundRobinScheduler::setProperty(const char* key, const char* value){
    if (std::string("timeslice") == key) {
      timeSlice_ = Director::createSC_Time(value);
    }
  }

  bool RoundRobinScheduler::getSchedulerTimeSlice(
    sc_core::sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
    time=timeSlice_;
    return true;
  }
  void RoundRobinScheduler::addedNewTask(Task *task){
    rr_fifo.push_back(task->getInstanceId());
  }
  void RoundRobinScheduler::removedTask(Task *task){
    std::deque<int>::iterator iter;
    for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
      if( *iter == task->getInstanceId()){
        rr_fifo.erase(iter);
        break;
      }
    }
  }

//
int RoundRobinScheduler::assignFromFront()
{
  int task_to_assign = rr_fifo.front();
  rr_fifo.pop_front();
  timeSliceExpires_ = sc_core::sc_time_stamp() + timeSlice_;
  return task_to_assign;
}

  scheduling_decision RoundRobinScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {

    scheduling_decision ret_decision=NOCHANGE;

    if(sc_core::sc_time_stamp() == timeSliceExpires_){// time slice expired
      if(rr_fifo.size()>0){    // select next task
        task_to_assign = assignFromFront();
        // default: old tasks execution delay is expired (no running task)
        ret_decision= ONLY_ASSIGN;
        if(!running_tasks.empty()){  // a running task is preempted
          TaskMap::const_iterator iter;
          iter=running_tasks.begin();
          Task *task=iter->second;
          task_to_resign=task->getInstanceId();
          rr_fifo.push_back(task->getInstanceId());
          ret_decision= PREEMPT;  
        }
      }    
    }else{
      // either a new task was added
      // or the running tasks delay is expired

      if(running_tasks.size()==0){       // if running tasks delay has expired
        if(rr_fifo.size()>0){            // schedule a new task
          task_to_assign = assignFromFront();
          // this is not preemption: the old task BLOCKED
          // and a new one is assigned
          ret_decision= ONLY_ASSIGN;
        }
      }
    } 
    return ret_decision;
  }


  /**
   *
   */
  sc_core::sc_time* RoundRobinScheduler::schedulingOverhead(){
    return NULL; //new sc_core::sc_time(1,sc_core::SC_NS);
  }
}
