#ifndef HSCD_VPC_EDFCONFSCHEDULER_H_
#define HSCD_VPC_EDFCONFSCHEDULER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>


#include "hscd_vpc_Allocator.h"
#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_Director.h"

namespace SystemC_VPC{

  /**
   * Implementation of AbstactController which runs FIFO strategy without preempting or killing 
   * running configuraitons.
   * This means that task are served in their arriving order and completed
   * before other conflicting task, which need another configuration, may be
   * completed.
   */
  class EDFAllocator : public Allocator {

    private:
      // count of insertion order of elements
      int order_count;

      /**
       * Helper class to store configurations within EDF list of controller.
       * Contains pointer to actual configuration and fifo entry for secundary strategy.
       */
      template<class C>
        class EDFListElement{

          private:
            C contained;
            std::list<sc_time> deadlines;
            int fifo_order;

          public:
            EDFListElement(C contained, sc_time deadline, int degree) : contained(contained), fifo_order(degree){
              this->deadlines.push_back(deadline);            
            }

            void setContained(C contained){
              this->contained = contained;
            }

            C getContained(){
              return this->contained;
            }

            void setFifoOrder(int degree){
              this->fifo_order = degree;
            }

            int getFifoOrder() const{
              return this->fifo_order;
            }

            /**
             * \brief Adds a deadline to a EDFListElement
             * Used to add deadline of a running task on a element
             * to the deadline of the configuration to enable deadline schedule
             * \param p specifies the deadline to be added
             */
            void addDeadline(sc_time d){
              std::list<sc_time>::iterator iter;

              for(iter = this->deadlines.begin(); iter != this->deadlines.end() && *iter > d; iter++);

              this->deadlines.insert(iter, d);

            }

            /**
             * \brief Removes a deadline from a EDFListElement
             * Used to remove deadline of a finished task from the element,
             * the first occurence of the deadline will be removed.
             * \param p specifies the deadline value to be removed
             */
            void removeDeadline(sc_time d){
              std::list<sc_time>::iterator iter;

              for(iter = this->deadlines.begin(); iter != this->deadlines.end(); iter++){
                if(*iter == d){
                  this->deadlines.erase(iter);
                  break;
                }
              }

            }

            /**
             * \brief Access to current deadline of a EDFListElement 
             * Used to access the current deadline of a EDFListElement determined
             * by the task running on it. If there are no running tasks
             * a default value is returned.
             * \return current deadline of configuration or -1 if no task is running
             * on the configuration
             */
            sc_time getDeadline() const{
              assert(this->deadlines.size() != 0);
              return this->deadlines.front();
            }
                
            /**
             * \brief Tests if there is at minimum one deadline stored.
             */
            bool hasDeadline() const {
              return (this->deadlines.size() > 0);
            }
                    
            

            bool operator < (const EDFListElement& pe){
              //check if there is any deadline available
              //reconstruction of behaviour if default return value is -1
              if( !(this->hasDeadline()) && !(pe.hasDeadline()) ) return this->getFifoOrder() > pe.getFifoOrder();
              if( !(this->hasDeadline()) ) return false;
              if(      !pe.hasDeadline() ) return true;

              if(this->getDeadline() > pe.getDeadline()){
                return true;
              }else if(this->getDeadline() == pe.getDeadline()){
                return this->getFifoOrder() > pe.getFifoOrder();
              }

              return false;  
            }

            bool operator == (const C c){

              return this->contained == c;

            }
        };

      // queue of tasks ready to be processed
      std::queue<ProcessControlBlock* > tasksToProcess;

      // queue containing order of configuration to be loaded in next "rounds"
      //std::list<EDFListElement<Configuration* > > nextConfigurations;
      std::list<EDFListElement<unsigned int> > nextConfigurations;

    public:

      EDFAllocator(AbstractController* controller);

      virtual ~EDFAllocator();

      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void addProcessToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc);

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void performSchedule(ReconfigurableComponent* rc);

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller. It is used to initialize and set up all necessary data for a new "round" of
       * scheduling. 
       */    
      //    virtual void addProcessToSchedule(std::deque<ProcessControlBlock* >& newTasks);

      /**
       * \brief Returns next configuration to be loaded
       * Used to indicate if a new configuration should be loaded by the controller
       * component.
       * \return id of next configuration to be loaded or 0 if no configuration
       * is selected up to now.
       */
      virtual unsigned int getNextConfiguration(ReconfigurableComponent* rc);

      /**
       * \brief Indicates if controller still can forward tasks
       * \return TRUE if there are still task to be forwarded else FALSE
       */
      virtual bool hasProcessToDispatch(ReconfigurableComponent* rc);

      /**
       * \brief Returns next task to be forwarded
       * This method should only be called after calling hasProcessToDispatch
       * to ensure that there are still existing task to process.
       * \return pair containing ProcessControlBlock of task and requested function
       * to be simulated.
       */
      virtual ProcessControlBlock* getNextProcess(ReconfigurableComponent* rc);

      /**
       * \brief Used to signal finished tasks to the controller
       * \see ProcessEventListener::signalProcessEvent
       */
      virtual void signalProcessEvent(ProcessControlBlock* pcb, std::string compID);

    private:

      /**
       * \brief Helper method to retrieve deadline
       * Used to determine earliest deadline out of a set of
       * mapping possibilities assuming that this constraint has to
       * be fullfied by scheduling instance.
       * \param pid specifies the task to determine deadline for
       * \return earliest deadline
       */
      sc_time getEarliestDeadline(ProcessControlBlock* pcb, ReconfigurableComponent* rc);
  };

}
#endif /*HSCD_VPC_EDFCONFSCHEDULER_H_*/