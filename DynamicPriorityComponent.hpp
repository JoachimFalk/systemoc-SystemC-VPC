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

#ifndef DYNAMICPRIORITYCOMPONENT_HPP_
#define DYNAMICPRIORITYCOMPONENT_HPP_

#include <systemcvpc/NonPreemptiveComponent.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Task.hpp>

#include <systemcvpc/config/Component.hpp>

#include <list>

namespace SystemC_VPC
{

class DynamicPriorityComponent: public NonPreemptiveComponent
{
public:
  typedef std::list<ScheduledTask *> PriorityList;

  DynamicPriorityComponent(Config::Component::Ptr component,
      Director *director = &Director::getInstance());

  void addTask(Task *newTask);

  Task * scheduleTask();

  void notifyActivation(ScheduledTask * scheduledTask, bool active);

  bool releaseActor();

  bool hasReadyTask();

  virtual void setDynamicPriority(PriorityList priorityList);
  virtual void schedulerAfterTransition();

private:
  PriorityList priorities_;
  bool mustYield_;
  Task * lastTask_;
  ScheduledTask * releasedTask_;
  void buildInitialPriorityList(Config::Component::Ptr component);
};

} //namespace SystemC_VPC
#endif /* DYNAMICPRIORITYCOMPONENT_HPP_ */
