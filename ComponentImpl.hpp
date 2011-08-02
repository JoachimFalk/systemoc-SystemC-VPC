/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <systemc.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/vpc_config.h>

#include "scheduler/Scheduler.hpp"
#include "timetriggered/tt_support.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_COMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC{

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class Component : public AbstractComponent{
    
    SC_HAS_PROCESS(Component);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);

    /**
     *
     */
    bool setAttribute(AttributePtr attribute);

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption();
      
    /**
     * \brief An implementation of AbstractComponent.
     */
    Component( Config::Component::Ptr component)
      : AbstractComponent(component),
        blockMutex(0)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      SC_METHOD(releaseActorsMethod);
      dont_initialize();
      sensitive << releaseActors;
      this->setPowerMode(this->translatePowerMode("SLOW"));
      setScheduler(component);

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM
    }
      
    virtual ~Component();
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

  protected:
    virtual void schedule_thread() = 0;
    virtual void remainingPipelineStages() = 0;
    virtual void moveToRemainingPipelineStages(Task* task) = 0;

    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;

    // time last task started
    sc_time startTime;

    void fireStateChanged(const ComponentState &state);

  private:
    sc_event releaseActors;
    TT::TimedQueue ttReleaseQueue;

#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    void initialize(const Director* d);

    void setScheduler(Config::Component::Ptr component);

    void releaseActorsMethod();
  };

  template<class TASKTRACER>
  class ComponentImpl : public Component{
  public:
    /**
     *
     */
    ComponentImpl( Config::Component::Ptr component)
      : Component(component), taskTracer_(component){}

    /**
     *
     */
    virtual ~ComponentImpl() {}

    /**
     *
     */
    virtual Trace::Tracing * getOrCreateTraceSignal(std::string name)
    {
      return taskTracer_.getOrCreateTraceSignal(name);
    }

    /**
     *
     */
    void addTasks(){
      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        Task *newTask;
        newTask=newTasks.front();
        newTasks.pop_front();
        DBG_OUT(this->getName() << " received new Task: "
                << newTask->getName() << " at: "
                << sc_time_stamp().to_default_time_units() << std::endl);
        this->taskTracer_.release(newTask);
        //insert new task in read list
        assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                /* A task can call compute only one time! */);
        assert( runningTasks.find(newTask->getInstanceId()) ==
                runningTasks.end()
                /* A task can call compute only one time! */);

        readyTasks[newTask->getInstanceId()]=newTask;
        scheduler->addedNewTask(newTask);
      }

    }

    /**
     *
     */
    void moveToRemainingPipelineStages(Task* task){
      sc_time now                 = sc_time_stamp();
      sc_time restOfLatency       = task->getLatency()  - task->getDelay();
      sc_time end                 = now + restOfLatency;
      if(end <= now){
        //early exit if (Latency-DII) <= 0
        //std::cerr << "Early exit: " << task->getName() << std::endl;
        this->taskTracer_.finishLatency(task);
        Director::getInstance().signalLatencyEvent(task);
        return;
      }
      timePcbPair pair;
      pair.time = end;
      pair.task  = task;
      //std::cerr << "Rest of pipeline added: " << task->getName()
      //<< " (EndTime: " << pair.time << ") " << std::endl;
      pqueue.push(pair);
      remainingPipelineStages_WakeUp.notify();
    }

    /**
     *
     */
    virtual void remainingPipelineStages(){
      while(1){
        if(pqueue.size() == 0){
          wait(remainingPipelineStages_WakeUp);
        }else{
          timePcbPair front = pqueue.top();

          //cerr << "Pop from list: " << front.time << " : "
          //<< front.pcb->getBlockEvent().latency << endl;
          sc_time waitFor = front.time-sc_time_stamp();
          assert(front.time >= sc_time_stamp());
          //cerr << "Pipeline> Wait till " << front.time
          //<< " (" << waitFor << ") at: " << sc_time_stamp() << endl;
          wait( waitFor, remainingPipelineStages_WakeUp );

          sc_time rest = front.time-sc_time_stamp();
          assert(rest >= SC_ZERO_TIME);
          if(rest > SC_ZERO_TIME){
            //cerr << "------------------------------" << endl;
          }else{
            assert(rest == SC_ZERO_TIME);
            //cerr << "Ready! releasing task (" <<  front.time <<") at: "
            //<< sc_time_stamp() << endl;

            this->taskTracer_.finishLatency(front.task);

            // Latency over -> remove Task
            Director::getInstance().signalLatencyEvent(front.task);

            //wait(SC_ZERO_TIME);
            pqueue.pop();
          }
        }

      }
    }

    /**
     *
     */
    void schedule_thread(){
      sc_time timeslice;
      sc_time actualRemainingDelay;
      sc_time *overhead = new sc_time( SC_ZERO_TIME );
      int actualRunningIID;
      bool newTaskDuringOverhead=false;
      //wait(SC_ZERO_TIME);

      scheduler->initialize();
      fireStateChanged(ComponentState::IDLE);

      //QUICKFIX solve thread initialization: actors are released before schedule_thread is called
      newTaskDuringOverhead=(newTasks.size()>0);

      while(1){
        //determine the time slice for next scheduling decision and wait for
        bool hasTimeSlice= scheduler->getSchedulerTimeSlice( timeslice,
                                                             readyTasks,
                                                             runningTasks );
        startTime = sc_time_stamp();
        if(!newTaskDuringOverhead){
          if(runningTasks.size()<=0){                    // no running task
            if(hasTimeSlice){
              wait( timeslice - (*overhead),
                    notify_scheduler_thread );
            }else{
              wait( notify_scheduler_thread );
            }
          }else{                                        // a task already runs
            if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){
              wait( timeslice - (*overhead),
                    notify_scheduler_thread );
            }else{
              wait( actualRemainingDelay,
                    notify_scheduler_thread );
            }
            sc_time runTime=sc_time_stamp()-startTime;
            assert(runTime.value()>=0);
            actualRemainingDelay-=runTime;

            assert(actualRemainingDelay.value()>=0);

            DBG_OUT("Component " << this->getName()
                      << "> actualRemainingDelay= "
                      << actualRemainingDelay.value() << " for iid="
                      << actualRunningIID << " at: "
                      << sc_time_stamp().to_default_time_units()
                      << std::endl);

            if(actualRemainingDelay.value()==0){
              // all execution time simulated -> BLOCK running task.
              Task *task=runningTasks[actualRunningIID];

            DBG_OUT(this->getName() << " IID: " << actualRunningIID<< " > ");
            DBG_OUT(this->getName() << " removed Task: " << task->getName()
                    << " at: " << sc_time_stamp().to_default_time_units()
                    << std::endl);

              //notify(*(task->blockEvent));
              scheduler->removedTask(task);
              fireStateChanged(ComponentState::IDLE);
              this->taskTracer_.finishDii(task);
              runningTasks.erase(actualRunningIID);

              task->getBlockEvent().dii->notify();
              if(task->hasScheduledTask()){
                assert(Director::canExecute(task->getProcessId()));
                Director::execute(task->getProcessId());
              }
              moveToRemainingPipelineStages(task);
              //wait(SC_ZERO_TIME);
            }else{

              // store remainingDelay
              runningTasks[actualRunningIID]->setRemainingDelay(
                actualRemainingDelay );
            }
          }
        }else{
          newTaskDuringOverhead=false;
        }

        this->addTasks();

        int taskToResign,taskToAssign;
        scheduling_decision decision =
          scheduler->schedulingDecision(taskToResign,
                                        taskToAssign,
                                        readyTasks,
                                        runningTasks);

        //resign task
        if(decision==RESIGNED || decision==PREEMPT){
          readyTasks[taskToResign]=runningTasks[taskToResign];
          runningTasks.erase(taskToResign);
          actualRunningIID=-1;
          readyTasks[taskToResign]->setRemainingDelay(actualRemainingDelay);
          fireStateChanged(ComponentState::IDLE);
          this->taskTracer_.resign(readyTasks[taskToResign]);
        }

        sc_time timestamp=sc_time_stamp();
        if( overhead != NULL ) delete overhead;
        overhead=scheduler->schedulingOverhead();

        if( overhead != NULL ){
          //    actual time    < endtime
          while( (sc_time_stamp() < timestamp + (*overhead)) ){

            wait( (timestamp+(*overhead))-sc_time_stamp(),
                  notify_scheduler_thread );

          }

          // true if some task becames ready during overhead waiting
          newTaskDuringOverhead=(newTasks.size()>0);
        }else {
          // avoid failures
          overhead=new sc_time(SC_ZERO_TIME);
        }


        //assign task
        if(decision==ONLY_ASSIGN || decision==PREEMPT){
          runningTasks[taskToAssign]=readyTasks[taskToAssign];
          this->taskTracer_.assign(runningTasks[taskToAssign]);
          readyTasks.erase(taskToAssign);
          actualRunningIID=taskToAssign;
          DBG_OUT("IID: " << taskToAssign << "> remaining delay for "
               << runningTasks[taskToAssign]->getName());
          actualRemainingDelay
            = sc_time(runningTasks[taskToAssign]->getRemainingDelay());
          DBG_OUT(" is " << runningTasks[taskToAssign]->getRemainingDelay()
               << endl);
          fireStateChanged(ComponentState::RUNNING);

          /* */
          Task * assignedTask = runningTasks[taskToAssign];
          if(assignedTask->isBlocking() /* && !assignedTask->isExec() */) {
            blockMutex++;
            if(blockMutex == 1) {
              DBG_OUT(this->getName() << " scheduled blocking task: "
                      << assignedTask->getName() << std::endl);
              assignedTask->ackBlockingCompute();
              DBG_OUT(this->getName() << " enter wait: " << std::endl);
              fireStateChanged(ComponentState::STALLED);
              this->taskTracer_.block(assignedTask);
              while(!assignedTask->isExec()){
                blockCompute.reset();
                CoSupport::SystemC::wait(blockCompute);
                this->addTasks();
              }
              DBG_OUT(this->getName() << " exit wait: " << std::endl);
              fireStateChanged(ComponentState::RUNNING);
              this->taskTracer_.assign(assignedTask);
              if(assignedTask->isBlocking()){
                DBG_OUT(this->getName() << " exec Task: "
                        << assignedTask->getName() << " @  " << sc_time_stamp()
                        << std::endl);
                // task is still blocking: exec task
              } else {
                DBG_OUT(this->getName() << " abort Task: "
                        << assignedTask->getName() << " @  " << sc_time_stamp()
                        << std::endl);

                //notify(*(task->blockEvent));
                scheduler->removedTask(assignedTask);
                fireStateChanged(ComponentState::IDLE);
                this->taskTracer_.finishDii(assignedTask);
                //FIXME: notify latency ??
                //assignedTask->traceFinishTaskLatency();
                runningTasks.erase(actualRunningIID);

              }
            }else{
              assert(blockMutex>1);
              scheduler->removedTask(assignedTask);
              fireStateChanged(ComponentState::IDLE);
              this->taskTracer_.finishDii(assignedTask);
              //FIXME: notify latency ??
              //assignedTask->traceFinishTaskLatency();
              runningTasks.erase(actualRunningIID);
              assignedTask->abortBlockingCompute();
            }
            blockMutex--;
          }
          /* */
        }
      }

    }

private:
    TASKTRACER taskTracer_;

  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
