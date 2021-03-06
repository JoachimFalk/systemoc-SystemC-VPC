// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
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

#include "Configuration.hpp"

#include "Routing/IgnoreImpl.hpp"

#include <systemcvpc/ConfigException.hpp>

#include <CoSupport/String/QuotedString.hpp>

#include <sstream>
#include <fstream>

namespace SystemC_VPC { namespace Detail {

  using CoSupport::String::QuotedString;

  Configuration::Configuration()
    : finalized(false)
    , ignoreMissingRoutes(false)
    , resultFileName("STDERR")
    , resultFile(&std::cerr)
    , resultFileOwned(false) {}

  Configuration::~Configuration() {
    if (resultFileOwned)
      delete resultFile;
  }

  Configuration &Configuration::getInstance() {
    static Configuration conf;
    return conf;
  }

  /// This will return the map of all components.
  Components const &Configuration::getComponents() const
    { return components; }
  /// This might return a nullptr.
  AbstractComponent::Ptr Configuration::hasComponent(std::string const &name) const {
    Components::const_iterator iter = components.find(name);
    return iter != components.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the component does not
  /// exists, then a ConfigException will be thrown.
  AbstractComponent::Ptr Configuration::getComponent(std::string const &name) const {
    Components::const_iterator iter = components.find(name);
    if (iter != components.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get component " << QuotedString(name) << " before creation. Use createComponent first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a component of the given name. A factory
  /// function has to be supplied. If a component with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractComponent::Ptr Configuration::createComponent(std::string const &name,
      std::function<AbstractComponent::Ptr ()> constr) {
    assert(!finalized);
    std::pair<Components::iterator, bool> status =
        components.insert(Components::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        components.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of component " << QuotedString(name) << " is not supported. Use getComponent instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This will return the map of all tasks.
  VpcTasks const &Configuration::getVpcTasks() const
    { return vpcTasks; }
  /// This might return a nullptr.
  VpcTask::Ptr Configuration::hasVpcTask(std::string const &name) const {
    VpcTasks::const_iterator iter = vpcTasks.find(name);
    return iter != vpcTasks.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the task does not
  /// exists, then a ConfigException will be thrown.
  VpcTask::Ptr Configuration::getVpcTask(std::string const &name) const {
    VpcTasks::const_iterator iter = vpcTasks.find(name);
    if (iter != vpcTasks.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get VPC task " << QuotedString(name) << " before creation. Use createVpcTask first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a task of the given name. A factory
  /// function has to be supplied. If a task with an identical
  /// name already exists, a ConfigException will be thrown.
  VpcTask::Ptr Configuration::createVpcTask(std::string const &name,
      std::function<VpcTask::Ptr ()> constr) {
    assert(!finalized);
    std::pair<VpcTasks::iterator, bool> status =
        vpcTasks.insert(VpcTasks::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        vpcTasks.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of task " << QuotedString(name) << " is not supported. Use getVpcTask instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a task, i.e., a SysteMoC actor. For each registered
  /// SysteMoC actor there must be a corresponding createVpcTask
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void Configuration::registerTask(
      TaskInterface                     *task,
      std::list<PossibleAction *> const &firingRules)
  {
    assert(!finalized);
    sassert(registeredTasks.insert(
        RegisteredTasks::value_type(task->name(),
            RegisteredTask(task, firingRules))).second);
  }

  /// This will return the map of all routes.
  Routes const &Configuration::getRoutes() const
    { return routes; }

  /// This might return a nullptr.
  AbstractRoute::Ptr Configuration::hasRoute(std::string const &name) const {
    Routes::const_iterator iter = routes.find(name);
    return iter != routes.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the route does not
  /// exists, then a ConfigException will be thrown.
  AbstractRoute::Ptr Configuration::getRoute(std::string const &name) const {
    Routes::const_iterator iter = routes.find(name);
    if (iter != routes.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get route " << QuotedString(name) << " before creation. Use createRoute first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a route of the given name. A factory
  /// function has to be supplied. If a route with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractRoute::Ptr Configuration::createRoute(std::string const &name,
      std::function<AbstractRoute::Ptr ()> constr) {
    assert(!finalized);
    std::pair<Routes::iterator, bool> status =
        routes.insert(Routes::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        routes.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of route " << QuotedString(name) << " is not supported. Use getRoute instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a route, i.e., a SysteMoC port. For each registered
  /// SysteMoC port there must be a corresponding createRoute
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void Configuration::registerRoute(PortInInterface *port) {
    assert(!finalized);
    sassert(registeredRoutes.insert(
        RegisteredRoutes::value_type(port->name(), port)).second);
  }
  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a route, i.e., a SysteMoC port. For each registered
  /// SysteMoC port there must be a corresponding createRoute
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void Configuration::registerRoute(PortOutInterface *port) {
    assert(!finalized);
    sassert(registeredRoutes.insert(
        RegisteredRoutes::value_type(port->name(), port)).second);
  }

  /// This will return the map of all timing modifiers.
  TimingModifiers const &Configuration::getTimingModifiers() const
    { return timingModifiers; }
  /// This might return a nullptr.
  TimingModifier::Ptr Configuration::hasTimingModifier(std::string const &name) const {
    TimingModifiers::const_iterator iter = timingModifiers.find(name);
    return iter != timingModifiers.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the timing modifier does not
  /// exists, then a ConfigException will be thrown.
  TimingModifier::Ptr Configuration::getTimingModifier(std::string const &name) const {
    TimingModifiers::const_iterator iter = timingModifiers.find(name);
    if (iter != timingModifiers.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get timing modifier " << QuotedString(name) << " before creation. Use createTimingModifier first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a timing modifier of the given name. A factory
  /// function has to be supplied. If a timing modifier with an identical
  /// name already exists, a ConfigException will be thrown.
  TimingModifier::Ptr Configuration::createTimingModifier(std::string const &name,
      std::function<TimingModifier::Ptr ()> constr) {
    assert(!finalized);
    std::pair<TimingModifiers::iterator, bool> status =
        timingModifiers.insert(TimingModifiers::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        timingModifiers.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of timing modifier " << QuotedString(name) << " is not supported. Use getTimingModifier instead.";
    throw ConfigException(msg.str().c_str());
  }

  std::string   Configuration::getResultFileName()
    { return resultFileName; }
  std::ostream &Configuration::getResultFile()
    { return *resultFile; }

  void Configuration::setResultFile(std::string const &resultFileName) {
    std::unique_ptr<std::ofstream> out(new std::ofstream(resultFileName));
    if (out->is_open()) {
      this->resultFileName = resultFileName;
      if (this->resultFileOwned)
        delete this->resultFile;
      this->resultFile = out.release();
      this->resultFileOwned = true;
    } else {
      std::stringstream msg;
      msg << "Can't open '" << resultFileName << "' for writing:" << strerror(errno);
      throw std::runtime_error(msg.str());
    }
  }
  void Configuration::setResultFile(
      std::ostream      &resultFile
    , std::string const &resultFileName)
  {
    this->resultFileName  = resultFileName;
    this->resultFile      = &resultFile;
    this->resultFileOwned = false;
  }

  void Configuration::finalize() {
    std::stringstream msg;

    finalized = true;
    for (Components::value_type const &v : components) {
      AbstractComponent::Ptr const &comp = v.second;
      comp->finalize();
    }
    for (RegisteredTasks::value_type const &v : registeredTasks) {
      RegisteredTask const &registeredTask = v.second;
      VpcTasks::iterator iter = vpcTasks.find(v.first);

      if (iter == vpcTasks.end()) {
        msg << "Actor " << v.first << " has NO configuration data at all." << std::endl;
        continue;
      }

      VpcTask::Ptr &task = iter->second;
      Component::Ptr configComponent = task->getComponent();

      if (!configComponent) {
        msg << "Actor " << v.first << " is not mapped." << std::endl;
        continue;
      }

      AbstractComponent *comp = static_cast<AbstractComponent *>(configComponent.get());
      comp->registerTask(registeredTask.task);

      for (PossibleAction *fr : registeredTask.firingRules) {
        comp->registerFiringRule(registeredTask.task, fr);
      }
    }
    for (RegisteredRoutes::value_type const &v : registeredRoutes) {
      RegisteredRoute const &registeredRoute = v.second;
      Routes::iterator iter = routes.find(v.first);

      if (iter == routes.end() && ignoreMissingRoutes) {
        bool status;
        std::tie(iter, status) =
                routes.insert(Routes::value_type(v.first,
                    new Routing::IgnoreImpl(v.first)));
        assert(status && "Insert must succeed!");
      }
      if (iter == routes.end()) {
        msg << "Route " << v.first << " has NO configuration data at all." << std::endl;
        continue;
      }

      struct Visitor: public boost::static_visitor<void> {
        Visitor(AbstractRoute::Ptr &route)
          : route(route) {}

        result_type operator()(PortInInterface *port) {
          port->setSchedulerInfo(route.get());
          route->setPortInterface(port);
        }
        result_type operator()(PortOutInterface *port) {
          port->setSchedulerInfo(route.get());
          route->setPortInterface(port);
        }

        AbstractRoute::Ptr &route;
      } visitor(iter->second);
      boost::apply_visitor(visitor, registeredRoute);
      iter->second->finalize();
    }
    if (!msg.str().empty())
      throw ConfigException(msg.str().c_str());
  }

} } // namespace SystemC_VPC::Detail
