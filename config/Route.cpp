/*
 * Route.cpp
 *
 *  Created on: Feb 18, 2011
 *      Author: streubuehr
 */

#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/Route.hpp>

#include <iostream>

namespace SystemC_VPC
{

namespace Config
{

//
Hop::Hop(Component::Ptr component) :
  component_(component), transferTiming_(component->getTransferTiming()),
      priority_()
{
}

//
Hop & Hop::setPriority(size_t priority_)
{
  this->priority_ = priority_;
  return *this;
}

Hop & Hop::setTransferTiming(Timing transferTiming_)
{
  this->transferTiming_ = transferTiming_;
//grocki: random
  std::cout << "test" << std::endl;
//grocki: end
  return *this;
}

Component::Ptr Hop::getComponent() const
{
  return component_;
}

size_t Hop::getPriority() const
{
  return priority_;
}

Timing Hop::getTransferTiming() const
{
//grocki: random
  std::cout << "test" << std::endl;
//grocki: end
  return transferTiming_;
}

//
Route::Type Route::parseRouteType(std::string name)
{
  static const std::string B_TRANSPORT = "blocking";
  static const std::string STATIC_ROUTE = "static_route";

  if (name == STATIC_ROUTE) {
    return StaticRoute;
  } else if (name == B_TRANSPORT) {
    return BlockingTransport;
  }

  throw Config::ConfigException("Unknown scheduler \"" + name
      + "\" for component: " + name);
  return StaticRoute;
}

Route::Route(Route::Type type, std::string source, std::string dest) :
  tracing_(false), source_(source), destination_(dest), type_(type),
  routeInterface_(NULL)
{
}

bool Route::getTracing() const
{
  return tracing_;
}

void Route::setTracing(bool tracing_)
{
  this->tracing_ = tracing_;
}
//
ComponentId Route::getComponentId() const
{
  return this->getSequentialId();
}

std::string Route::getDestination() const
{
  return destination_;
}

std::list<Hop> Route::getHops() const
{
  return hops_;
}

std::string Route::getSource() const
{
  return source_;
}

//
std::string Route::getName() const {
  return "msg_" + getSource() + "_2_" + getDestination();
}

Route::Type Route::getType() const
{
  return type_;
}

void Route::inject(std::string source, std::string destination)
{
  this->source_ = source;
  this->destination_ = destination;
}

RouteInterface::Ptr Route::getRouteInterface() const
{
  //TODO: assert simulation phase
  assert(routeInterface_ != NULL);
  return routeInterface_;
}

//
Hop & Route::addHop(Component::Ptr component)
{
  Hop hop(component);
  hops_.push_back(hop);
  return hops_.back();
}

} // namespace Config
} // namespace SystemC_VPC
