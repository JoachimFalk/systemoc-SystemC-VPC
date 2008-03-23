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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "smoc_synth_std_includes.hpp"

#include "MIdct2D.hpp"

#ifdef REAL_BLOCK_DATA
# include "MIdctImageSource.hpp"
# undef  DEFAULT_BLOCK_COUNT
# define DEFAULT_BLOCK_COUNT ((IMAGE_WIDTH)/8*(IMAGE_HEIGHT)/8)
#else
# include "MCounterSource.hpp"
# ifndef DEFAULT_BLOCK_COUNT
#   define DEFAULT_BLOCK_COUNT 1
# endif
#endif

#ifdef IDCT_NULL_OUTPUT
# include "MNullSink.hpp"
#else
# include "MImageSink.hpp"
#endif


class MTopIdct2D
: public smoc_graph {
private:
#ifdef REAL_BLOCK_DATA
  MIdctImageSource  mSrc;
#else
  MCounterSource    mSrc;
#endif
  MIdct2D           mIdct2D;

#ifdef IDCT_NULL_OUTPUT
  MNullSink         mSnk;
#else
  MImageSink        mSnk;
#endif


public:
  MTopIdct2D(sc_module_name name, size_t periods)
    : smoc_graph(name),
      mSrc("mSrc", periods),
      mIdct2D("mIdct2D", 128, 0, 255),
#ifndef IDCT_NULL_OUTPUT
      mSnk("mSnk", IMAGE_WIDTH, IMAGE_HEIGHT)
#else
      mSnk("mSnk")
#endif
  {
#ifndef KASCPAR_PARSING
    connectNodePorts(mSrc.out,     mIdct2D.in, smoc_fifo<int>(128));
# ifdef REAL_BLOCK_DATA
    connectNodePorts(mIdct2D.out,  mSnk.in,    smoc_fifo<int>(IMAGE_WIDTH/8*64));
# else
    connectNodePorts(mIdct2D.out,  mSnk.in,    smoc_fifo<int>(128));
# endif
#endif
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  size_t periods            =
    (argc > 1)
    ? atoi(argv[1])
    : DEFAULT_BLOCK_COUNT;
  
  MTopIdct2D mTopIdct2D("mTopIdct2D", periods);
  
  smoc_top mTop(&mTopIdct2D);
  
  sc_start();
  
  return 0;
}
#endif
