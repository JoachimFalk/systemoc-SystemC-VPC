// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_VCDTRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_VCDTRACER_HPP

#include "TracerIf.hpp"

#include <systemcvpc/config/Component.hpp>

#include <systemc>

#include <string>

namespace SystemC_VPC { namespace Tracing {

class VcdTracer: public TracerIf {
public:
  VcdTracer(Config::Component::Ptr component);

  ~VcdTracer();

  TTask         *registerTask(std::string const &name);

  TTaskInstance *release(TTask *ttask);

  void           assign(TTaskInstance *ttaskInstance);

  void           resign(TTaskInstance *ttaskInstance);

  void           block(TTaskInstance *ttaskInstance);

  void           finishDii(TTaskInstance *ttaskInstance);

  void           finishLatency(TTaskInstance *ttaskInstance);
private:
  class VcdTask;
  class VcdTaskInstance;

  std::string getName() const;

  sc_core::sc_trace_file *traceFile_;
  std::string name_;
//std::map<std::string, Trace::Tracing*> trace_map_by_name_;
};

} } // namespace SystemC_VPC::Tracing

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_VCDTRACER_HPP */
