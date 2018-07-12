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

#include "config.h"

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/PortInterfaces.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/Mappings.hpp>

#include "Director.hpp"
#include "VPCBuilder.hpp"
#include "AbstractComponent.hpp"

#include "DebugOStream.hpp"

namespace SystemC_VPC { namespace Detail {

using namespace smoc::SimulatorAPI;

namespace po = boost::program_options;
namespace VC = SystemC_VPC;

class SystemCVPCSimulator
  : public SimulatorInterface
{
public:
  SystemCVPCSimulator();

  void populateOptionsDescription(
      int &argc, char ** &argv,
      boost::program_options::options_description &pub,
      boost::program_options::options_description &priv);

  EnablementStatus evaluateOptionsMap(
      boost::program_options::variables_map &vm);

  void registerTask(TaskInterface *task);

  void registerPort(TaskInterface *task, PortInInterface *port);
  void registerPort(TaskInterface *task, PortOutInterface *port);

  void simulationEnded();
};

SystemCVPCSimulator::SystemCVPCSimulator() {
}

void SystemCVPCSimulator::populateOptionsDescription(
    int &argc, char ** &argv,
    boost::program_options::options_description &pub,
    boost::program_options::options_description &priv)
{
  // Provide --systemoc-vpc-debug option
  {
    std::stringstream sstr;
    sstr << "set debug level; level 0 is off; level " << Debug::None.level << " is most verbose";
#ifdef SYSTEMCVPC_ENABLE_DEBUG
    pub.add_options()
#else //!defined(SYSTEMCVPC_ENABLE_DEBUG)
    priv.add_options()
#endif //!defined(SYSTEMCVPC_ENABLE_DEBUG)
      ("systemoc-vpc-debug",
       po::value<size_t>()->default_value(0),
       sstr.str().c_str())
      ;
  }
  // Provide --systemoc-vpc-config option and its backward compatibility version
  {
    pub.add_options()
      ("systemoc-vpc-config",
       getenv("VPCCONFIGURATION")
         ? po::value<std::string>()->default_value(getenv("VPCCONFIGURATION"))
         : po::value<std::string>(),
       "use specified SystemC-VPC configuration file")
      ;
    // Backward compatibility cruft
    priv.add_options()
      ("vpc-config",
       po::value<std::string>())
      ;
  }
}

SystemCVPCSimulator::EnablementStatus SystemCVPCSimulator::evaluateOptionsMap(
    boost::program_options::variables_map &vm)
{
  EnablementStatus retval;

  std::string vpcConfigFile;
  if (vm.count("systemoc-vpc-config")) {
    vpcConfigFile = vm["systemoc-vpc-config"].as<std::string>();
    retval = MUSTBE_ACTIVE;
  } else if (vm.count("vpc-config")) {
    vpcConfigFile = vm["systemoc-vpc-config"].as<std::string>();
    retval = MUSTBE_ACTIVE;
  } else {
    retval = IS_DISABLED;
  }

#ifdef SYSTEMCVPC_ENABLE_DEBUG
  int debugLevel = Debug::None.level - vm["systemoc-vpc-debug"].as<size_t>();
  getDbgOut().setLevel(debugLevel < 0 ? 0 : debugLevel);
  getDbgOut() << Debug::High;
#else  //!defined(SYSTEMCVPC_ENABLE_DEBUG)
  if (vm["systemoc-vpc-debug"].as<size_t>() != 0)
    std::cerr << "libsystemc-vpc: Warning debug support not compiled in and, thus, --systemoc-vpc-debug option ignored!" << std::endl;
#endif //!defined(SYSTEMCVPC_ENABLE_DEBUG)

  if (retval != IS_DISABLED) {
#ifdef _MSC_VER
    std::string envVar("VPCCONFIGURATION=" + vpcConfigFile);
    putenv((char *) envVar.c_str());
#else
    setenv("VPCCONFIGURATION", vpcConfigFile.c_str(), 1);
#endif // _MSC_VER

    try {
      VPCBuilder builder(&Director::getInstance());
      builder.buildVPC();
    } catch (InvalidArgumentException &e) {
      std::cerr << "VPCBuilder> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    } catch (const std::exception &e) {
      std::cerr << "VPCBuilder> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }

    Director::getInstance().beforeVpcFinalize();
    if (Director::getInstance().FALLBACKMODE) {
      if (getDbgOut().isVisible(Debug::High))
        getDbgOut() << "SystemC_VPC has invalid configuration " << getenv("VPCCONFIGURATION") << " => VPC still off" << std::endl;
      retval = IS_DISABLED;
    } else {
      if (getDbgOut().isVisible(Debug::High))
        getDbgOut() << "SystemC_VPC has valid configuration " << getenv("VPCCONFIGURATION") << " => turning VPC on" << std::endl;
    }
    unsetenv("VPCCONFIGURATION");
  }

  return retval;
}

void SystemCVPCSimulator::registerTask(TaskInterface *actor) {
  VC::VpcTask::Ptr vpcTask1 = VC::hasTask(static_cast<ScheduledTask &>(*actor))
    ? VC::getCachedTask(static_cast<ScheduledTask &>(*actor))
    : nullptr;
  VC::VpcTask::Ptr vpcTask2 = VC::hasTask(actor->name())
    ? VC::getCachedTask(actor->name())
    : nullptr;
  if (!vpcTask1 && !vpcTask2)
    throw VC::ConfigException(std::string(actor->name()) +
        " has NO configuration data at all.");
  if (vpcTask1 && vpcTask2 && vpcTask1 != vpcTask2) {
    // TODO: Check if a merging strategy is required.
    throw VC::ConfigException(std::string(actor->name()) +
        " has configuration data from XML and from configuration API.");
  }
  if (!vpcTask1)
    VC::setCachedTask(&static_cast<ScheduledTask &>(*actor), (vpcTask1 = vpcTask2));
  else if (!vpcTask2)
    VC::setCachedTask(actor->name(), (vpcTask2 = vpcTask1));
  assert(VC::hasTask(actor->name()) && VC::hasTask(static_cast<ScheduledTask &>(*actor)));
  assert(VC::getCachedTask(actor->name()) == VC::getCachedTask(static_cast<ScheduledTask &>(*actor)));

  VC::VpcTask::Ptr &task = vpcTask1;
  assert(VC::Mappings::getConfiguredMappings().find(task) != VC::Mappings::getConfiguredMappings().end());
  VC::Component::Ptr configComponent = VC::Mappings::getConfiguredMappings()[task];
  AbstractComponent * comp = static_cast<AbstractComponent *>(configComponent.get());
  comp->registerTask(actor);
}

void SystemCVPCSimulator::registerPort(TaskInterface *task, PortInInterface *port) {
  std::string actorName = task->name();
  std::string channelName = port->getSource()->name();
  // FIXME: Should be port not nullptr!
  port->setSchedulerInfo(Director::getInstance().registerRoute(channelName,
      actorName, nullptr));

}
void SystemCVPCSimulator::registerPort(TaskInterface *task, PortOutInterface *port) {
  std::string actorName = task->name();
  // FIXME: Routes have to be changed to support multicast themselves.
  for (ChannelSinkInterface *channelSinkInterface : port->getSinks()) {
    std::string channelName = channelSinkInterface->name();
    // FIXME: Should be port not nullptr!
    port->setSchedulerInfo(Director::getInstance().registerRoute(actorName,
        channelName, nullptr));
  }
}

void SystemCVPCSimulator::simulationEnded() {
  Director::endOfSystemcSimulation();
}

} } // namespace SystemC_VPC::Detail

// This must be outside of the namespace for SysteMoC plugin load pickup!
SystemC_VPC::Detail::SystemCVPCSimulator systemCVPCSimulator;
