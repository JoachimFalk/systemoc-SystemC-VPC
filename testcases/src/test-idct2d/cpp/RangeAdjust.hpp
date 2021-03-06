// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

#ifndef _INCLUDED_MRANGEADJ_HPP
#define _INCLUDED_MRANGEADJ_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

class RangeAdjust: public smoc_actor {
public:
  smoc_port_in<int>   in;
  smoc_port_out<int>  out;
private:
  const int levelAdj;
  const int min;
  const int max;

  void transform() {
    int val = in[0] + levelAdj;
    if(val < min)
      val = min;
    else if(val > max)
      val = max;
    out[0] = val;
  }

  smoc_firing_state main;
public:
  RangeAdjust(sc_core::sc_module_name name, int levelAdj, int min, int max)
    : smoc_actor(name, main), levelAdj(levelAdj), min(min), max(max)
  {
    SMOC_REGISTER_CPARAM(levelAdj);
    SMOC_REGISTER_CPARAM(min);
    SMOC_REGISTER_CPARAM(max);
    main
      = in(1)                       >>
        out(1)                      >>
        CALL(RangeAdjust::transform)  >> main
      ;
  }
};

#endif // _INCLUDED_MRANGEADJ_HPP
