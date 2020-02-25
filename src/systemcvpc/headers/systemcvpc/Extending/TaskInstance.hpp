// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP
#define _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/EventPair.hpp>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <functional>

namespace SystemC_VPC { namespace Extending {


  /**
   * This class represents the publicly visible information of a
   * task instance, i.e., one execution of a task on a component.
   */
  class TaskInstance {
  public:
  };

} } // namespace SystemC_VPC::Extending

#endif /* _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP */
