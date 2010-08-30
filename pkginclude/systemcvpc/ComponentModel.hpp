#ifndef HSCD_VPC_COMPONENT_MODEL_H
#define HSCD_VPC_COMPONENT_MODEL_H

#include "ComponentInfo.hpp"

namespace SystemC_VPC{
  class Task;
  typedef std::map<int, Task*>  TaskMap;

  class ComponentModel : public ComponentInfo {
  public:
    ComponentModel() : ComponentInfo(this){}

    const TaskMap& getReadyTasks(){
      return readyTasks;
    }

    const TaskMap& getRunningTasks(){
      return runningTasks;
    }

    virtual void setPowerMode(const PowerMode *mode) = 0;

  protected:
    void setComponentState(const ComponentState cs)
    {
      componentState = cs;
    }

    void setPowerConsumption(const double pc)
    {
      powerConsumption = pc;
    }

    TaskMap readyTasks;
    TaskMap runningTasks;
  };
}
#endif // HSCD_VPC_COMPONENT_MODEL_H