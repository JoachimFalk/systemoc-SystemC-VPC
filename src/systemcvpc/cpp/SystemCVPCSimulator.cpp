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

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

#include "DebugOStream.hpp"

namespace SystemC_VPC {

using namespace smoc::SimulatorAPI;

namespace po = boost::program_options;

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

  SchedulerInterface *registerTask(TaskInterface *task);

};

SystemCVPCSimulator::SystemCVPCSimulator() {
  registeredSimulators.push_back(this);
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

  pub.add_options()
    ("systemoc-vpc-config",
     po::value<std::string>(),
     "use specified SystemC-VPC configuration file")
    ;
  // Backward compatibility cruft
  priv.add_options()
    ("vpc-config",
     po::value<std::string>())
    ;
}

SystemCVPCSimulator::EnablementStatus SystemCVPCSimulator::evaluateOptionsMap(
    boost::program_options::variables_map &vm)
{
  EnablementStatus retval;

  if (vm.count("systemoc-vpc-config") ||
      vm.count("vpc-config")) {
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

    return retval;
}

SchedulerInterface *SystemCVPCSimulator::registerTask(TaskInterface *task)
{
  return nullptr;
}

SystemCVPCSimulator systemCVPCSimulator;

} // namespace SystemC_VPC