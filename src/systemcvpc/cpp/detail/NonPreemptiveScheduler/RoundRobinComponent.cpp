// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of
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

#include "RoundRobinComponent.hpp"
#include "../tracing/TracerIf.hpp"

namespace SystemC_VPC { namespace Detail {

RoundRobinComponent::RoundRobinComponent(std::string const &name)
  : AbstractComponent(name)
  , useActivationCallback(false)
  , actualTask(NULL)
{
  /// FIXME: WTF?! SLOW hardcoded?
  this->setPowerMode(this->translatePowerMode("SLOW"));

  SC_THREAD(scheduleThread);
}

void RoundRobinComponent::setActivationCallback(bool flag) {
  if (useActivationCallback == flag)
    return;
  useActivationCallback = flag;
  if (!flag) {
    for (TaskInterface *scheduledTask: taskList)
      scheduledTask->setUseActivationCallback(flag);
  } else {
    for (std::vector<TaskInterface *>::iterator iter = taskList.begin();
         iter != taskList.end();
         ++iter) {
      // This will trigger notifyActivation with
      // either false when the actor is still not fireabke
      // or     true when the actor is fireable.
      (*iter)->setUseActivationCallback(true);
      // Then, notifyActivation will reset useActivationCallback
      // back to false.
      if (!useActivationCallback) {
        /// Oops, undo it
        (*iter)->setUseActivationCallback(false);
        while (iter != taskList.begin()) {
          --iter;
          (*iter)->setUseActivationCallback(false);
        }
        return;
      }
    }
  }
}

void RoundRobinComponent::end_of_elaboration() {
  PCBPool const &pcbPool = getPCBPool();
  for (PCBPool::const_iterator it=pcbPool.begin(); it!=pcbPool.end(); ++it) {
    if (it->second->hasScheduledTask()) {
      TaskInterface *scheduledTask = it->second->getScheduledTask();
      scheduledTask->setUseActivationCallback(false);
      taskList.push_back(scheduledTask);
    }
  }
}

void RoundRobinComponent::notifyActivation(TaskInterface * scheduledTask, bool active) {
  if (active) {
    setActivationCallback(false);
    readyEvent.notify();
  }
}

void RoundRobinComponent::compute(TaskInstance *actualTask) {
  std::cout << "\t " << sc_core::sc_time_stamp() << " : task PID " << actualTask->getProcessId() << std::endl;
  if (actualTask->getPCB()->hasScheduledTask()) {
    assert(!useActivationCallback);
    scheduleMessageTasks();
    this->actualTask = actualTask;
    releaseTask(actualTask->getPCB(), actualTask);
    assignTaskInstance(actualTask);
    std::cout << "compute: " <<  actualTask->getName() << "@" << sc_core::sc_time_stamp() << std::endl;
    wait(actualTask->getDelay());
    finishDiiTaskInstance(actualTask);
    /// FIXME: What about DII != latency
    finishLatencyTaskInstance(actualTask);
  } else
    readyMsgTasks.push_back(actualTask);
  readyEvent.notify();
}

void RoundRobinComponent::check(TaskInstance *actualTask) {
  if (!useActivationCallback) {
    releaseTask(actualTask->getPCB(), actualTask);
    assignTaskInstance(actualTask);
    wait(actualTask->getDelay());//Director::getInstance().getOverhead() +
    finishDiiTaskInstance(actualTask);
    finishLatencyTaskInstance(actualTask);

    std::cout << "check: " <<  actualTask->getName() << "@" << sc_core::sc_time_stamp() << std::endl;
  }
}

/**
 *
 */
void RoundRobinComponent::requestBlockingCompute(TaskInstance* task, VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::execBlockingCompute(TaskInstance* task, VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::abortBlockingCompute(TaskInstance* task, VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::updatePowerConsumption() {
}

/*
 * from ComponentInterface
 */
bool RoundRobinComponent::hasWaitingOrRunningTasks()
{
  /// FIXME: Implement this;
  assert(false);
  return false;
}

bool RoundRobinComponent::scheduleMessageTasks() {
  bool progress = !readyMsgTasks.empty();
  while (!readyMsgTasks.empty()) {
    TaskInstance *messageTask = readyMsgTasks.front();
    readyMsgTasks.pop_front();
    assert(!messageTask->getPCB()->hasScheduledTask());
    releaseTask(messageTask->getPCB(), messageTask);
    assignTaskInstance(messageTask);
    /// This will setup the trigger for schedule_method to be called
    /// again when the task execution time is over.
    wait(messageTask->getDelay());
    finishDiiTaskInstance(messageTask);
    /// FIXME: What about dii != latency
    finishLatencyTaskInstance(messageTask);
  }
  return progress;
}

void RoundRobinComponent::scheduleThread() {
  while (true) {
    bool progress = scheduleMessageTasks();
    for (TaskInterface *scheduledTask: taskList) {
      std::cout << "Checking " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
      while (scheduledTask->canFire()) {
        progress = true;
        assert(readyMsgTasks.empty());
        std::cout << "Scheduling " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
        assert(!this->actualTask);
        // This will invoke our compute callback and setup actualTask.
        scheduledTask->schedule();
        while (!actualTask ||
               !actualTask->getPCB()->hasScheduledTask() ||
               actualTask->getPCB()->getScheduledTask() != scheduledTask) {
          scheduleMessageTasks();
          if (!actualTask ||
              !actualTask->getPCB()->hasScheduledTask() ||
              actualTask->getPCB()->getScheduledTask() != scheduledTask)
            wait(readyEvent);
        }
        actualTask = nullptr;
        scheduleMessageTasks();
      }
    }
    if (!progress) {
      setActivationCallback(true);
      while (useActivationCallback) {
        wait(readyEvent);
        scheduleMessageTasks();
      }
    }
  }
}

RoundRobinComponent::~RoundRobinComponent() {}

} } // namespace SystemC_VPC::Detail