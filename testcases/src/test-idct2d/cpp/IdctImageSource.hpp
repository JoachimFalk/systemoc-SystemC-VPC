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

#ifndef _INCLUDED_MIDCTIMAGESOURCE_HPP
#define _INCLUDED_MIDCTIMAGESOURCE_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>

#include "smoc_synth_std_includes_idct.hpp"

class MIdctImageSource: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  size_t coeffs;
  size_t zeroRep;
  size_t dataOff;

  void process() {
    int val;
    
#ifndef KASCPAR_PARSING    
# define IMAGE_WIDTH  176
# define IMAGE_HEIGHT 144
    // ZRL coded IDCT coeffs
    const static int block_data[] = {
# include "Y_IdctCoeff.txt"
    };
    const size_t block_data_size =   
      sizeof(block_data)/sizeof(block_data[0]);
#endif

    if (zeroRep > 0) {
      out[0] = 0; --zeroRep;
    } else {
      out[0] = (val = block_data[dataOff]); ++dataOff;
      if (val == 0) {
        // RLZ
        zeroRep = block_data[dataOff] - 1; ++dataOff;
      }
    }
    ++coeffs;
    if (dataOff >= block_data_size)
      dataOff = 0;
  }
 
  smoc_firing_state start;
public:
  MIdctImageSource(sc_core::sc_module_name name, size_t periods)
    : smoc_actor(name, start), coeffs(0), zeroRep(0), dataOff(0)
  {
    SMOC_REGISTER_CPARAM(periods);
    start
      = (out(1) && VAR(coeffs) < periods * 64)  >>
        CALL(MIdctImageSource::process)         >> start
      ;
  }
};

#endif // _INCLUDED_MIDCTIMAGESOURCE_HPP
