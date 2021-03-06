// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ComponentTracer.hpp>
#include <systemcvpc/Extending/ComponentTracerIf.hpp>

#include <sys/socket.h>
#include <netdb.h>

namespace SystemC_VPC {

  const char *Component::Tracer::DB = "DB";

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Detail { namespace Tracers {

  class ComponentDBTracer
    : public Extending::ComponentTracerIf
    , public ComponentTracer
  {
  public:
    ComponentDBTracer(Attributes const &attrs);

    ///
    /// Implement interface for ComponentTracerIf
    ///

    void componentOperation(ComponentOperation co
      , Component const &c
      , OComponent      &oc);

    void taskOperation(TaskOperation to
      , Component const &c
      , OComponent      &oc
      , Task      const &t
      , OTask           &ot);

    void taskInstanceOperation(TaskInstanceOperation tio
      , Component    const &c
      , OComponent         &oc
      , OTask              &ot
      , TaskInstance const &ti
      , OTaskInstance      &oti);

    ///
    /// Implement interface for ComponentTracer
    ///

    bool addAttribute(Attribute const &attr);

  private:
    class DataBaseProxy;
    class DBTask;
    class DBTaskInstance;
    class RegisterMe;

    static RegisterMe registerMe;

    void addEvent(
        std::string const &resourceName,
        DBTaskInstance    &dbTaskInstance,
        char const        *state);

    DataBaseProxy &dbProxy_;
  };

  class ComponentDBTracer::DataBaseProxy {
  public:
    static DataBaseProxy &getDataBaseProxy() {
      static DataBaseProxy dbProxy("VPC");
      return dbProxy;
    }

    ~DataBaseProxy();

    // addEvent
    void addEvent(const char* resourceName, const char* taskName,
        const char* status, unsigned long long timeStamp, unsigned int taskId);

  private:
    const char* databaseName_;
    uint16_t portNumber_;
    FILE* socket_;

    // create database with given name
    DataBaseProxy(const char* database_name, uint16_t port = 5555);

    void open();

    void close();
  };

  ComponentDBTracer::DataBaseProxy::DataBaseProxy(const char* database_name, uint16_t port)
    : databaseName_(database_name)
    , portNumber_(port)
    { open(); }

  ComponentDBTracer::DataBaseProxy::~DataBaseProxy()
    { close(); }

  void ComponentDBTracer::DataBaseProxy::addEvent(
      const char *resourceName,
      const char *taskName,
      const char *status,
      const unsigned long long timeStamp,
      const unsigned int taskId)
  {
    fprintf(socket_, "%s %s %s %llu %u\r\n", resourceName, taskName, status,
        timeStamp, taskId);
  }

  void ComponentDBTracer::DataBaseProxy::open() {

    struct sockaddr_in6 socketAddr;
    int socketFD;

    socketAddr.sin6_family = AF_INET6;
    socketAddr.sin6_addr = in6addr_any;
    socketAddr.sin6_port = htons(portNumber_);

    if ((socketFD = socket(PF_INET6, SOCK_STREAM, 0)) == -1) {
      perror("server socket");
      exit(EXIT_FAILURE);
    }

    if (connect(socketFD, (const struct sockaddr *) &socketAddr,
        sizeof(socketAddr)) == -1) {
      perror("connection error");
      exit(EXIT_FAILURE);
    }

    if ((socket_ = fdopen(socketFD, "a+")) == nullptr) {
      perror("fdopen");
      exit(EXIT_FAILURE);
    }

    fprintf(socket_, "%s\r\n", databaseName_);
  }

  void ComponentDBTracer::DataBaseProxy::close() {
    fprintf(socket_, "CLOSE\r\n");
  }

  class ComponentDBTracer::DBTask: public OTask {
  public:
    DBTask(std::string const &name)
      : name(name) {}

    std::string name;

    ~DBTask() {}
  };

  class ComponentDBTracer::DBTaskInstance: public OTaskInstance {
  public:
    DBTaskInstance(DBTask *dbTask)
      : dbTask(dbTask), instanceId(instanceIdCounter++) {}

    static size_t instanceIdCounter;

    DBTask *dbTask;
    size_t  instanceId;

    ~DBTaskInstance() {}
  };

  class ComponentDBTracer::RegisterMe {
  public:
    RegisterMe() {
      ComponentDBTracer::registerTracer("DB",
        [](Attributes const &attrs) { return new ComponentDBTracer(attrs); });
    }
  } ComponentDBTracer::registerMe;

  size_t ComponentDBTracer::DBTaskInstance::instanceIdCounter = 0;

  ComponentDBTracer::ComponentDBTracer(Attributes const &attrs)
    : Extending::ComponentTracerIf(
          reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this))
        , 0, sizeof(DBTask), sizeof(DBTaskInstance))
    , ComponentTracer(
         reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this))
       , "DB")
    , dbProxy_(DataBaseProxy::getDataBaseProxy()) {}

  void ComponentDBTracer::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
  {
    // Ignore
  }

  void ComponentDBTracer::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
  {
    DBTask &dbTask = static_cast<DBTask &>(ot);

    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::ALLOCATE) {
      new (&dbTask) DBTask(t.getName());
    }
    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::DEALLOCATE) {
      dbTask.~DBTask();
    }
  }

  void ComponentDBTracer::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
  {
    DBTask         &dbTask         = static_cast<DBTask &>(ot);
    DBTaskInstance &dbTaskInstance = static_cast<DBTaskInstance &>(oti);

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::ALLOCATE) {
      new (&dbTaskInstance) DBTaskInstance(&dbTask);
    }

    std::string const &resourceName = c.getName();

    switch (TaskInstanceOperation((int) tio & ~ (int) TaskInstanceOperation::MEMOP_MASK)) {
      case TaskInstanceOperation::RELEASE:
        this->addEvent(resourceName, dbTaskInstance, "s");
        break;
      case TaskInstanceOperation::ASSIGN:
        this->addEvent(resourceName, dbTaskInstance, "a");
        break;
      case TaskInstanceOperation::RESIGN:
        this->addEvent(resourceName, dbTaskInstance, "r");
        break;
      case TaskInstanceOperation::BLOCK:
        this->addEvent(resourceName, dbTaskInstance, "b");
        break;
      case TaskInstanceOperation::FINISHDII:
        this->addEvent(resourceName, dbTaskInstance, "d");
        break;
      case TaskInstanceOperation::FINISHLAT:
        this->addEvent(resourceName, dbTaskInstance, "l");
        break;
      default:
        break;
    }

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::DEALLOCATE) {
      dbTaskInstance.~DBTaskInstance();
    }
  }

  bool ComponentDBTracer::addAttribute(Attribute const &attr) {
    throw ConfigException("The DB tracer does not support attributes!");
  }

  void ComponentDBTracer::addEvent(
      std::string const &resourceName,
      DBTaskInstance    &dbTaskInstance,
      char const        *state)
  {
    dbProxy_.addEvent(resourceName.c_str(),
        dbTaskInstance.dbTask->name.c_str(),
        state,
        sc_core::sc_time_stamp().value(),
        dbTaskInstance.instanceId);
  }

} } } // namespace SystemC_VPC::Detail::Tracers
