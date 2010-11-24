#ifndef HSCD_VPC_TASK_H_
#define HSCD_VPC_TASK_H_

#include <sstream>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/TaskPool.hpp>

namespace SystemC_VPC {

  using CoSupport::SystemC::Event;
  using CoSupport::SystemC::RefCountEventPtr;
  class Task{
  public:
    Task(TaskPool * pool)
      : blockingCompute(NULL),
      timing(),
      pcb(),
      pool(pool),
      name("NN"),
      timingScale(1),
      scheduledTask(NULL)
    {
          this->instanceId = Task::globalInstanceId++;
    }

    // getter, setter
    std::string getName(){return name;}
    void setName(std::string name){this->name = name;}
    ProcessId  getProcessId()                    {return pid;}
    void       setProcessId(ProcessId pid)       {this->pid = pid;}
    FunctionIds getFunctionIds()                   {return fid;}
    void        setFunctionIds(FunctionIds fid)  {this->fid = fid;}
    EventPair  getBlockEvent()                   {return blockEvent;}
    void       setBlockEvent(EventPair p)        {blockEvent = p;}
    void       setPCB(ProcessControlBlockPtr pcb)  {this->pcb = pcb;}
    void       setTiming(FunctionTimingPtr timing) {this->timing = timing;}

    void       setExtraDelay(const sc_time & ed) { this->extraDelay = ed;}

    void       ackBlockingCompute(){
      blockAck = true;
      blockingCompute->notify();
    }
    void       abortBlockingCompute(){
      blockAck = false;
      blockingCompute->notify();
    }

    void       resetBlockingCompute(){this->setBlockingCompute(NULL);}
    void       setBlockingCompute(RefCountEventPtr blocker)
      { blockingCompute = blocker; }
    bool       isBlocking()
      { return blockingCompute != NULL; }
    bool       isAckedBlocking()
      { return blockAck; }
    void       setExec( bool exec ) {this->exec=exec;}
    bool       isExec(  ) { return this->exec;}
    void       setWrite( bool write ) {this->write=write;}
    bool       isWrite(  ) { return this->write;}


    void setDelay(const sc_time& delay)         {this->delay = delay;}
    sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_time& latency)     {this->latency = latency;}
    sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_time& delay){this->remainingDelay = delay;}
    sc_time getRemainingDelay() const           {return this->remainingDelay;}
    int getInstanceId()                         {return this->instanceId;}
    void setTimingScale( double scale )         {this->timingScale = scale;}
    double getTimingScale()                     {return this->timingScale;}

    void setScheduledTask(ScheduledTask * st)
      {this->scheduledTask = st;}
    ScheduledTask * getScheduledTask()
      {return this->scheduledTask;}
    bool hasScheduledTask()
      {return this->scheduledTask != NULL;}

    /**
     * 
     */
    void initDelays(){
      assert(pcb != NULL);
      FunctionIds fids = this->getFunctionIds();
      this->setDelay(this->timingScale * timing->getDelay(fids)
          + this->extraDelay);
      this->setRemainingDelay(this->getDelay());
      this->setLatency(this->timingScale * timing->getLatency(fids)
          + this->extraDelay);
    }

    // Adaptor setter / getter for ProcessControlBlock
    int getPriority()
      {assert(pcb != NULL); return pcb->getPriority();}
    sc_time getPeriod()
      {assert(pcb != NULL); return pcb->getPeriod();}

    void release() {
      //this->setBlockEvent(EventPair(NULL, NULL));
      this->getBlockEvent().dii     = NULL;
      this->getBlockEvent().latency = NULL;
      pool->free(this->getProcessId(), this);
    }

    void traceReleaseTask(){
#ifndef NO_VCD_TRACES
      if(this->getTraceSignal()!=0)
        this->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
      taskTracerTicket = pcb->taskTracer->releaseTask();
    }

    void traceFinishTaskLatency(){
      pcb->taskTracer->finishTaskLatency(taskTracerTicket);
    }

    void traceFinishTaskDii(){
#ifndef NO_VCD_TRACES
        if(this->getTraceSignal()!=0)
          this->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
    }

    void traceAssignTask(){
#ifndef NO_VCD_TRACES
      if(this->getTraceSignal()!=0)
        this->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES

    }

    void traceResignTask(){
#ifndef NO_VCD_TRACES
        if(this->getTraceSignal()!=0)
          this->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES

    }

    void traceBlockTask(){
#ifndef NO_VCD_TRACES
            if(this->getTraceSignal()!=0)
              this->getTraceSignal()->traceBlocking();
#endif //NO_VCD_TRACES

    }
  private:
    Tracing* getTraceSignal()
      {assert(pcb != NULL); return pcb->getTraceSignal();}

    friend class AssociativePrototypedPool<ProcessId, Task>;
    friend class PrototypedPool<Task>;

    Task(const Task &task) :
      pid(task.pid),
      fid(task.fid),
      blockEvent(task.blockEvent),
      blockingCompute(task.blockingCompute),
      write(task.write),
      delay(task.delay),
      latency(task.latency),
      remainingDelay(task.remainingDelay),
      extraDelay(task.extraDelay),
      timing(task.timing),
      pcb(task.pcb),
      pool(task.pool),
      name(task.name),
      timingScale(task.timingScale),
      scheduledTask(task.scheduledTask)
    {
          this->instanceId = Task::globalInstanceId++;
    }
        

    ProcessId        pid;
    FunctionIds      fid;
    EventPair        blockEvent;

    RefCountEventPtr blockingCompute;
    bool       blockAck;
    bool       exec;
    bool       write;

    sc_time delay;
    sc_time latency;
    sc_time remainingDelay;
    sc_time extraDelay;
    
    FunctionTimingPtr       timing;
    ProcessControlBlockPtr  pcb;
    TaskPool            *pool;

    static int globalInstanceId;
    int instanceId;
    std::string name;
    double timingScale;
    CoSupport::Tracing::TaskTracer::Ticket taskTracerTicket;
    ScheduledTask * scheduledTask;
  };

  typedef std::map<int, Task*>  TaskMap;

}
#endif // HSCD_VPC_TASK_H_
