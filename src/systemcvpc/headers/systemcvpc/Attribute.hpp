// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP
#define _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP

#include "InvalidArgumentException.hpp"

#include <boost/intrusive_ptr.hpp>

#include <list>
#include <map>
#include <string>

namespace SystemC_VPC {

  class Attribute;

  class Attributes {
  private:
    typedef Attributes this_type;
    typedef std::list<Attribute> AL;
  public:
    typedef Attribute           value_type;
    typedef value_type         &reference;
    typedef value_type const   &const_reference;

    typedef AL::iterator        iterator;
    typedef AL::const_iterator  const_iterator;

    typedef AL::difference_type difference_type;
    typedef AL::size_type       size_type;
  public:

    iterator       begin() noexcept
      { return attributeList.begin(); }
    const_iterator begin() const noexcept
      { return attributeList.begin(); }
    const_iterator cbegin() const noexcept
      { return attributeList.begin(); }

    iterator       end() noexcept
      { return attributeList.end(); }
    const_iterator end() const noexcept
      { return attributeList.end(); }
    const_iterator cend() const noexcept
      { return attributeList.end(); }

    bool      empty() const noexcept
      { return attributeList.empty(); }
    size_type size() const noexcept
      { return attributeList.size(); }
    size_type max_size() const noexcept
      { return attributeList.max_size(); }

    reference       front()
      { return attributeList.front(); }
    const_reference front() const
      { return attributeList.front(); }

    reference       back()
      { return attributeList.back(); }
    const_reference back() const
      { return attributeList.back(); }

    iterator insert(const_iterator iter, const_reference val);

    void     push_back(const_reference val)
      { insert(end(), val); }

    /**
     * Returns an iterator to an attribute with the given type.
     * If no such attribute is present, returns end(). If multiple
     * such attributes are present, returns an iterator to one of
     * them. Incrementing the returned iterator will, in general,
     * NOT point to the next attribute with the given type if multiple
     * of them are present!
     */
    iterator       find(std::string const &type);
    /**
     * Returns an iterator to an attribute with the given type.
     * If no such attribute is present, returns end(). If multiple
     * such attributes are present, returns an iterator to one of
     * them. Incrementing the returned iterator will, in general,
     * NOT point to the next attribute with the given type if multiple
     * of them are present!
     */
    const_iterator find(std::string const &type) const
      { return const_cast<this_type *>(this)->find(type); }

    /**
     * Returns attribute with given type. If no such attribute
     * is present, throws InvalidArgumentException. If multiple
     * such attributes are present, returns one of them.
     */
    reference       operator[](std::string const &type);
    /**
     * Returns attribute with given type. If no such attribute
     * is present, throws InvalidArgumentException. If multiple
     * such attributes are present, returns one of them.
     */
    const_reference operator[](std::string const &type) const
      { return (*const_cast<this_type *>(this))[type]; }

  private:
    typedef std::map<std::string, iterator> AM;

    AL attributeList;
    AM attributeMap;
  };

  class Attribute {
    typedef Attribute this_type;
  public:
    /**
     *
     */
    Attribute(std::string const &type, std::string const &value);
    Attribute(std::string const &type, Attributes const &attrs);
    Attribute(std::string const &type, std::string const &value, Attributes const &attrs);

    Attributes const &getAttributes() const
      { return attributes; }

    std::string const &getValue() const noexcept
      { return value; }
    std::string const &getType() const noexcept
      { return type; }

    /**
     *
     */
    bool isType(std::string const &check) const
      { return this->type==check; }

    ~Attribute(){}
  private:
    std::string type;
    std::string value;

    Attributes  attributes;
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP */

