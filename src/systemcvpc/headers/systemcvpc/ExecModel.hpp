// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_EXECMODEL_HPP
#define _INCLUDED_SYSTEMCVPC_EXECMODEL_HPP

#include "Timing.hpp"
#include "Attribute.hpp"

#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include <boost/noncopyable.hpp>
#include <boost/intrusive_ptr.hpp>

#include <string>

namespace SystemC_VPC { namespace Detail {

  class AbstractExecModel;

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC {

class ExecModel
  : private boost::noncopyable
  , public CoSupport::SmartPtr::RefCountObject
{
  typedef ExecModel this_type;

  friend class Component; // for getImpl
public:
  typedef boost::intrusive_ptr<this_type>       Ptr;
  typedef boost::intrusive_ptr<this_type const> ConstPtr;

  virtual void add(Timing timing) = 0;
  virtual void addDefaultActorTiming(std::string actorName, Timing timing) = 0;

  virtual bool addAttribute(Attribute const &attr) = 0;

  const char *getType() const
    { return type; }
protected:
  ExecModel(const char *type, int implAdj);

  ~ExecModel();
private:
  Detail::AbstractExecModel       *getImpl();
  Detail::AbstractExecModel const *getImpl() const
    { return const_cast<this_type *>(this)->getImpl(); }
private:
  const char *type;
  int         implAdj;
};

ExecModel::Ptr createExecModel(const char *type);

template <typename EXECMODEL>
typename EXECMODEL::Ptr
createExecModel() {
  return boost::static_pointer_cast<EXECMODEL>(
      createExecModel(EXECMODEL::Type));
}

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_EXECMODEL_HPP */
