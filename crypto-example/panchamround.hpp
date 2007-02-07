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

/*****************************************************************
 Pancham is an MD5 compliant IP core for cryptographic applicati
 -ons. 
 Copyright (C) 2003  Swapnajit Mittra, Project VeriPage
 (Contact email: verilog_tutorial at hotmail.com
  Website      : http://www.angelfire.com/ca/verilog)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the 
 
 Free Software Foundation, Inc.
 59 Temple Place, Suite 330
 Boston, MA  02111-1307 
 USA
 ******************************************************************/
/* 
 * pancham_round performs the rounds 1-4 of the MD5 algorithm 
 * described in RFC1321 for a 128-bit long input message. 
 * 
 * Inputs: [abcd m[k] s abs(sin(2*pi*t/64))] as described 
 *         in RFC1321.Also the round number (1-4).
 * 
 * Outputs: the modified 'a' value as describes in RFC1321
 *         on the left hand side of the round #n equation.
 * 
 */

#ifndef PANCHAMROUND_HH
#define PANCHAMROUND_HH

#include "panchamdefines.hpp"
#include "systemc.h"

/**
*/
class PanchamRound
{
  private:
  sc_uint<32>
  F(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z);

  sc_uint<32>
  G(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z);
  
  sc_uint<32>
  H(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z);
  
  sc_uint<32>
  I(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z);
    
  public:
    PanchamRound();

    ~PanchamRound();

    sc_uint<32>
    perform(sc_uint<32> a, sc_uint<32> b, sc_uint<32> c, sc_uint<32> d,
	    // Note that for a 128-bit long input message, X[k] = M[k] = m :
	    sc_uint<32> m, sc_uint<32> s,
	    sc_uint<32> t,     // t-th sample of abs(sin(i)), i = 1, 2, ..., 64
	    sc_uint<2>  round); // round number (1-4).

};

#endif
