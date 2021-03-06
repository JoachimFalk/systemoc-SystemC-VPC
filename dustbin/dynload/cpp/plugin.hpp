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

#ifndef __PLUGIN_H
#define __PLUGIN_H


#include "dll.hpp"


//
// PlugIn is an abstract class.
//
// This is an example plug in.  This plug in only has one method, Show(),
// which we will use to show its name.
//
//

class PlugIn
{
 public:
	PlugIn();
	
	virtual ~PlugIn();
	
	virtual void Show() = 0;	
};


//
// The is an example factory for plug ins. 
//
// This example factory only announces when it is created/destroyed and
// has the single abstract method CreatePlugIn() which returns a type 
// of plug in.
//
// In the real world, you may have multiple different classes in each
// shared library that are made to work together.  All these classes
// must be created by the Factory class.
//
// You may find it useful to have the objects that you create with
// the factory class be given a pointer to the factory class so
// they can create their own objects that they need, using the same
// factory class.  Compiler support of covariant return types is 
// real useful here.
//


class PlugInFactory
{
 public:
	PlugInFactory() 
	{
		std::cout << "PlugInFactory Created" << std::endl;
	}
	
	virtual ~PlugInFactory()
	{
		std::cout << "PlugInFactory Destroy" << std::endl;		
	}
	
	virtual PlugIn * CreatePlugIn() = 0;
	
};

#endif
