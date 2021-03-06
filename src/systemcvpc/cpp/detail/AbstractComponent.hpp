// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Ralph Siegel <siegel@codesign.informatik.uni-erlangen.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ABSTRACTCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ABSTRACTCOMPONENT_HPP

#include <systemcvpc/Component.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Attribute.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/VPCEvent.hpp>

#include "TaskImpl.hpp"
#include "TaskInstanceImpl.hpp"
#include "PluggablePowerGovernor.hpp"
#include "SequentiallyIdedObject.hpp"
#include "AbstractExecModel.hpp"
#include "ObservableComponent.hpp"
#include "TimeQueue.hpp"

#include "config.h"

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>

#include <CoSupport/SystemC/systemc_support.hpp>

#include <systemc>

#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <list>
#include <vector>
#include <map>
#include <string>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief The interface of a Virtual-Processing-Component (VPC).
   */
  class AbstractComponent
    : public sc_core::sc_module
    , public SystemC_VPC::Component
    , public SystemC_VPC::ComponentInterface
    , public ObservableComponent
    , private AbstractExecModel::ComponentMixIn
    , private SequentiallyIdedObject
    , private smoc::SimulatorAPI::SchedulerInterface
  {
    typedef AbstractComponent this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    using ObservableComponent::addObserver;

    ///
    /// Handle interfaces for SystemC_VPC::Component
    ///

    /// Realize debug file interface from SystemC_VPC::Component with
    /// a default unsupported implementation.
    virtual bool        hasDebugFile() const;
    /// Realize debug file interface from SystemC_VPC::Component with
    /// a default unsupported implementation.
    virtual void        setDebugFileName(std::string const &fileName);
    /// Realize debug file interface from SystemC_VPC::Component with
    /// a default unsupported implementation.
    virtual std::string getDebugFileName() const;

    const ComponentId   getComponentId() const
      { return this->getSequentialId(); }
    const std::string  &getName() const
      { return componentName; }

    PowerMode const    &getPowerMode() const
      { return powerMode; }

    ComponentState      getComponentState() const
      { return compState; }

    Power               getPowerConsumption() const
      { return powerConsumption; }

    void setExecModel(AbstractExecModel *model)
      { model->attachToComponent(this); }
    AbstractExecModel *getExecModel()
      { return this->ComponentMixIn::getExecModel(); }

    ///
    /// Handle interfaces for SystemC_VPC::ComponentInterface
    ///

    void changePowerMode(std::string powerMode);
    /// virtual bool hasWaitingOrRunningTasks() = 0; still open

    void registerComponentWakeup(const ScheduledTask * actor, VPCEvent::Ptr event);
    void registerComponentIdle(const ScheduledTask * actor, VPCEvent::Ptr event);

    void setCanExec(bool canExec);

    /// Realize dynamic priority interface from SystemC_VPC::ComponentInterface with
    /// a default unsupported implementation.
    virtual void                       setDynamicPriority(std::list<ScheduledTask *> priorityList);
    /// Realize dynamic priority interface from SystemC_VPC::ComponentInterface with
    /// a default unsupported implementation.
    virtual std::list<ScheduledTask *> getDynamicPriority();

    virtual void scheduleAfterTransition();

    virtual bool addStream(ProcessId pid);
    virtual bool closeStream(ProcessId pid);

    ///
    /// Interface used by Configuration::finalize(), which is run at end of elaboration.
    ///

    void registerTask(TaskInterface *task);

    void registerFiringRule(
        TaskInterface *task, PossibleAction *fr);

    ///
    /// Other stuff
    ///

    void requestCanExecute();

    bool getCanExecuteTasks() const;

    void setCanExecuteTasks(bool canExecuteTasks);

    virtual void reactivateExecution();


    /**
     * \brief Simulate an execution on this "Virtual Component".
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute(TaskInstanceImpl* task)=0;

    /**
     * \brief Simulate the delay caused by the transition guard check on this "Virtual Component".
     * While the simulation is running SystemC simulation time is consumed.
     */
    virtual void check(TaskInstanceImpl* task) {}

    virtual void finalize();

    TaskInstanceImpl *executeHop(TaskImpl *taskImpl
      , Timing const &transferTiming
      , size_t quantum
      , std::function<void (TaskInstanceImpl *)> const &cb);
  protected:
    AbstractComponent(std::string const &name);

    /// Called once per actor firing to indicate that the task instance is ready to execute on resource.
    void releaseTaskInstance(TaskInstanceImpl *ti);

    /// Called possibly multiple times to assign the task instance to the resource.
    void assignTaskInstance(TaskInstanceImpl *ti);

    /// Called possibly multiple times to adjust remaining delay of assigned task instance.
    void ranTaskInstance(TaskInstanceImpl *ti);

    /// Called possibly multiple times to resign the task instance from the resource.
    void resignTaskInstance(TaskInstanceImpl *ti);

    /// Called possibly multiple times to indicate that the task is blocked waiting for something.
    void blockTaskInstance(TaskInstanceImpl *ti);

    /// Called once per actor firing to indicate that the DII of the task instance is over.
    void finishDiiTaskInstance(TaskInstanceImpl *ti);

  private:
    /// Called once per actor firing to indicate that the latency of the task instance is over.
    void finishLatencyTaskInstance(TaskInstanceImpl *ti);
  protected:

    std::map<const PowerMode*, sc_core::sc_time> transactionDelays;
    bool requestExecuteTasks;

    PlugInFactory<PluggableLocalPowerGovernor> *localGovernorFactory;
    PluggableLocalPowerGovernor *midPowerGov;
    typedef std::map<std::string,
                     DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >* >
      Factories;
    static Factories factories;

    bool requestShutdown();

    virtual ~AbstractComponent();
  private:
    class InputsAvailableListener;

    /// Implement SchedulerInterface for SysteMoC
    void checkFiringRule(TaskInterface *task, PossibleAction *fr);

    /// Implement SchedulerInterface for SysteMoC
    void executeFiringRule(TaskInterface *task, PossibleAction *fr);

    void loadLocalGovernorPlugin(std::string plugin);

    std::string componentName;
    bool canExecuteTasks;
    sc_core::sc_time shutdownRequestAtTime;
    VPCEvent::Ptr componentWakeup;
    VPCEvent::Ptr componentIdle;

    TaskInstanceImpl *assignedTaskInstance;
    sc_core::sc_time  assignedTaskInstanceTime;

    // Queue for task instances where their DII has expired but still
    // waiting for their latency.
    TimeQueue<TaskInstanceImpl> latencyQueue;

    PowerMode      powerMode;
    ComponentState compState;
    Power          powerConsumption;
  };

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC {

  static inline
  Detail::AbstractComponent::Ptr  getImpl(Component::Ptr const &p)
    { return boost::static_pointer_cast<Detail::AbstractComponent>(p); }
  static inline
  Detail::AbstractComponent      *getImpl(Component            *p)
    { return static_cast<Detail::AbstractComponent *>(p); }

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ABSTRACTCOMPONENT_HPP */
