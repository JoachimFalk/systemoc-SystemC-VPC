// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_SELECTFASTESTPOWERMODEGLOBALGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_SELECTFASTESTPOWERMODEGLOBALGOVERNOR_HPP

#include <map>
#include <deque>
#include <systemc>

#include <systemcvpc/PowerMode.hpp>

#include "PluggablePowerGovernor.hpp"

namespace SystemC_VPC { namespace Detail {

  class InternalSelectFastestPowerModeGovernor :
    public PluggableGlobalPowerGovernor
  {
  public:
    InternalSelectFastestPowerModeGovernor();

    void notify_top(Component *ci, GenericParameter *param);

  private:
    PowerMode                        m_lastMode;
    std::map<Component *, PowerMode> m_components;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_SELECTFASTESTPOWERMODEGLOBALGOVERNOR_HPP */
