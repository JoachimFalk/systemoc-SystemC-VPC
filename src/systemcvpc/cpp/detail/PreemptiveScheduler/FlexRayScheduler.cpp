// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

/* 
        FlexRay-Scheduler
        
        CMX-Format:
        
        z.B.
         <component name="Component1" type="threaded" scheduler="FlexRay">
         <attribute type="FlexRayParams">
              <parameter type="dualchannel" value="false"/>
              <attribute type="static">
                        <attribute type="slot0" value="20ns">
                          <attribute type="mapping" value="periodic.task1">
                            <parameter type="multiplex" value="0"/>
                            <parameter type="offset" value="0"/>
                          </attribute>
                        </attribute>
                        
                        <attribute type="slot1" value="20ns">
                          <attribute type="mapping" value="periodic.task2">
                            <parameter type="multiplex" value="4"/>
                            <parameter type="offset" value="2"/>
                          </attribute>
                        </attribute>
              </attribute>
                      
              <attribute type="dynamic" value="50ns">
                        <attribute type="slot3">
                          <parameter type="mapping" value="periodic.task4"/>
                        </attribute>
                        <attribute type="slot4" value="30ns">
                          <parameter type="mapping" value="periodic.task5"/>
                        </attribute>
                        
              </attribute>
        </attribute>
        </component>
        Sebastian Graf - Dezember 2007

*/

#include "FlexRayScheduler.hpp"
#include "PreemptiveComponent.hpp"

#include "../Director.hpp"
#include "../common.hpp"

namespace SystemC_VPC { namespace Detail {

  FlexRayScheduler::FlexRayScheduler()
    : _properties() {
    //neu FlexRay
    //    StartslotDynamic = 3;
    //    TimeDynamicSegment = Director::createSC_Time("50ns");
    //Standardmaessig beide Kanaele aktivieren!
    dualchannel=1; 
    //alt (TDMA)
    processcount=0;
    cyclecount=0;
    lastassign=sc_core::sc_time(0,sc_core::SC_NS);
    cycle_length = sc_core::sc_time(0,sc_core::SC_NS);
    this->remainingSlice = sc_core::sc_time(0,sc_core::SC_NS);
    slicecount=0;
    to_init=0;
   // firstrun=true;
    curr_slicecount=-1; 
    curr_slicecountA=0;
    curr_slicecountB=0; 
  }
  
  void FlexRayScheduler::setProperty(const char* key, const char* value){
    std::pair<std::string, std::string> toadd(key,value);
    _properties.push_back(toadd);
  }
  
  void FlexRayScheduler::initialize(){   
    for(std::deque< std::pair <std::string, std::string> >::const_iterator iter = this->_properties.begin();
        iter != this->_properties.end();
        ++iter){
      this->_setProperty(iter->first.c_str(), iter->second.c_str());
    }
#ifdef VPC_DEBUG
    std::cout << "------------ END Initialize ---------"<<std::endl;
#endif //VPC_DEBUG      
    this->_properties.clear();
  }
  
 
  void FlexRayScheduler::_setProperty(const char* key, const char* value){
    /*char *domain;
      int slot;
      //Herausfinden, welcher Slot genannt ist + welche Zeit ihm zugeordnet wird    
  
      if(0==strncmp(key,"slot",strlen("slot"))){
      //std::cout<<"hier darf ich - bei neuer Config - nichtmehr hin!!!!!!!"<<std::endl;
      domain=strstr(key,"slot");
      if(domain!=NULL){
      domain+=4*sizeof(char);
      sscanf(domain,"%d",&slot);
      domain=strstr(value,"ns");
      if(domain!=NULL){
      //Erstellen der TDMA-Struktur
      TDMASlot newSlot;
      newSlot.length = Director::createSC_Time(value);  
      newSlot.name = key;
      if(slicecount<StartslotDynamic){
      TDMA_slots.insert(TDMA_slots.end(), newSlot);
      }else{
      Dynamic_slots.insert(Dynamic_slots.end(), newSlot);
      //std::cout<<"new Dynamic One! " << newSlot.length <<std::endl;
      }
      slicecount++;
      //Erzeugen einer Info-Ausgabe             
      domain[0]='\0';
      sscanf(value,"%lf",&slottime);
      std::cout << "Datensatz fuer Slot Nr." << slot 
      <<"gefunden! TDMA-Slotdauer: " <<slottime << "ns"<<std::endl;
                                
      }
      }
      }else 
      if(0==strncmp(value,"slot",strlen("slot"))){
    */
    //std::cout<<"looking for "<< key << " and " << value<<std::endl;
    int i=-1;
    //schon Slots geadded? oder cmx-Syntax/Reihenfolge falsch?
    assert(0<TDMA_slots.size());
    //Betreffende SlotID in der Slotliste suchen
    do{
      //nichts zu tun.. da lediglich durchiteriert wird!
    }while(TDMA_slots[++i].name != value && (i+1)<(int)TDMA_slots.size());
    //std::cout<<"found at i= " <<i<<std::endl;
                
    if(TDMA_slots[i].name != value){
      i=-1;
      assert(0<Dynamic_slots.size());
      //std::cout<< "DynSize: " << Dynamic_slots.size()<<std::endl;
      do{
        //nichts zu tun.. da lediglich durchiteriert wird!
      }while(Dynamic_slots[++i].name != value && (i+1)<(int)Dynamic_slots.size());
      //auch wirklich etwas passendes gefunden?         
      assert(i<(int)Dynamic_slots.size());      
      i+=StartslotDynamic;      
    }
    //std::cout<<"PId-Map "<<i<< " to "<< Director::getInstance().getProcessId(key) <<std::endl;
    //Beziehung PId - SlotID herstellen
    ProcessId id=Director::getInstance().getProcessId(key);
    PIDmap[id]=i;   
    ProcessParams[id]=ProcessParams_string[key];
    //                 std::cout<<"registered function: "<< key<<" with ID: "<<id<<std::endl;
    //          std::cout<<"add Function " <<  key << " to " << value<<std::endl;
                
  }

  void FlexRayScheduler::setAttribute(Attribute const &attribute) {
    if (attribute.getType() != "FlexRayParams" )
      return;

    for (Attribute const &attr : attribute.getAttributes()) {
      if (attr.getType() == "dualchannel")
        dualchannel = attr.getValue() == "true";
      else if(attr.getType() == "static") {
        Attribute const &fr_static = attr;
        StartslotDynamic=0;
        for (Attribute const &attr2 : fr_static.getAttributes()) {
          //Slot einrichten
          StartslotDynamic++;
          slicecount++;
          std::pair<std::string, std::string > param;
          param.first = attr2.getType();
          param.second= attr2.getValue();

          //std::cout<<"found static Slot: "<<param.first <<" with value: "<<param.second<<std::endl;
          TDMASlot newSlot;
          //Werte aus dem Attribute auslesen und damit neuen Slot erzeugen
          newSlot.length = createSC_Time(param.second.c_str() );
          cycle_length += newSlot.length;
          newSlot.name = param.first;
          TDMA_slots.insert(TDMA_slots.end(), newSlot);

          //jetzt noch die Task-mappings!
          //für jeden Attribute-Eintrag Parameter verarbeiten
          for (Attribute const &attr3 : attr2.getAttributes()) {
            std::pair<std::string, std::string > param3;
            if (attr3.getType() == "mapping") {
              param3.first = attr3.getValue();
              param3.second= param.first;
              // std::cout<<"found static binding: "<<param3.second <<" with value: "<<param3.first<<std::endl;

              this->_properties.push_back(param3);
              ProcessParams_string[param3.first]=SlotParameters(0,0);
              for (Attribute const &attr4 : attr3.getAttributes()) {
                //parse parameters
                if(attr4.getType() == "offset") {
                  ProcessParams_string[param3.first].offset
                    = atoi(attr4.getValue().c_str());
                  // std::cout<<"found Offset-Setting for "<<param3.first<<" with value: "<<param4.second<<std::endl;
                }
                else if(attr4.getType() == "multiplex") {
                  ProcessParams_string[param3.first].multiplex
                    = atoi(attr4.getValue().c_str());
                  // std::cout<<"found Multiplex-Setting for "<<param3.first<<" with value: "<<param4.second<<std::endl;
                }
              }
            }
          }
          /*
            for(j=0;j<attribute2.second->getParameterSize();j++){
            std::pair<std::string, std::string > param2 =attribute2.second->getNextParameter(j);
            if(param2.first == "mapping"){
            param2.first=param2.second;
            param2.second=param.first;
            std::cout<<"found static binding: "<<param2.second <<" with value: "<<param2.first<<std::endl;
            this->_properties.push_back(param2);
            }

            }
          */
        }

        /*
          for(j=0;j<attribute2.second->getParameterSize();j++){
          std::pair<std::string, std::string > param=attribute2.second->getNextParameter(j);
          std::cout<<"found static Slot: "<<param.first <<" with value: "<<param.second <<std::endl;
          this->_properties.push_back(param);

          }
        */
      }
      else if(attr.getType() == "dynamic") {
        Attribute const &fr_dynamic = attr;
        this->TimeDynamicSegment = createSC_Time(fr_dynamic.getValue().c_str());
        cycle_length += this->TimeDynamicSegment;

        for (Attribute const &attr2 : fr_dynamic.getAttributes()) {
          //Slot einrichten
          slicecount++;
          std::pair<std::string, std::string > param;
          param.first = attr2.getType();
          param.second= attr2.getValue();
          // std::cout<<"found dynamic Slot: "<<param.first <<" with value: "<<param.second <<std::endl;

          TDMASlot newSlot;
          std::string temp = param.second;
          if(temp==""){
            //Default-Value fuer eine Dynamic-Slot-Laenge
            param.second="30us";
          }
          newSlot.length = createSC_Time(param.second.c_str() );
          newSlot.name = param.first;
          Dynamic_slots.insert(Dynamic_slots.end(), newSlot);
          //                      std::cout<<"new Dynamic One! " << newSlot.length <<std::endl;

          //jetzt noch die Task-mappings!
          for (Attribute const &paramx : attr2.getAttributes()) {
            if(paramx.getType() == "mapping"){
              std::pair<std::string, std::string > param2(paramx.getValue(), param.first);
              // std::cout<<"found dyn. binding: "<<param2.first <<"value: "<<param2.second<<std::endl;
              this->_properties.push_back(param2);
            }
          }
        }

        /*        std::pair<std::string, Attribute >attribute2=attribute.second.getNextAttribute(0);
                  for(j=0;j<attribute2.second->getParameterSize();j++){
                  std::pair<std::string, std::string > param=attribute2.second->getNextParameter(j);
                  std::cout<<"found dynamic Slot: "<<param.first <<" with value: "<<param.second<<std::endl;
                  this->_properties.push_back(param);
                  }
        */
      }
    }
  }
  
  bool FlexRayScheduler::getSchedulerTimeSlice( sc_core::sc_time& time,
                                                const TaskMap &ready_tasks,
                                                const TaskMap &running_tasks )
  {   
    // keine wartenden + keine aktiven Threads -> ende!
/*    if(processcount==0 && running_tasks.size()==0)   {std::cout<<"return 0"<<std::endl; return 0;} */
    //ansonsten: Restlaufzeit der Zeitscheibe
    if(curr_slicecount<StartslotDynamic){ //statisch
      if(curr_slicecount == -1){
        if(ready_tasks.size() == 0 && running_tasks.size() == 0 ){
          return false;
        }else{
          time=sc_core::sc_time(0,sc_core::SC_NS);
        }
      }else{
        time=this->remainingSlice;
      }
    }else{
      //Dynamisch
      //      std::cout<<"dynamic-timeSlice-request "<< curr_slicecount << "  " << sc_core::sc_time_stamp() << "  " << time << "running_tasks " << running_tasks.size() <<std::endl;
      if(running_tasks.size()<=1){
        //gerade kein (dynamic) Task aktiv.. -> naechster Schedulevorgang!
        //oder nur einer -> ein Kanal frei!
        time=sc_core::sc_time(1,sc_core::SC_US); //sofortiges Reschedule im nächsten Microtick bewirken!
      }else{
        //beide Kanaele voll.. -> naechster Schedulevorgang nach Ende von einem...
        if(remainingSliceA < remainingSliceB){
          time=remainingSliceA - (sc_core::sc_time_stamp() - this->lastassignA);
        }else{
          time=remainingSliceB - (sc_core::sc_time_stamp() - this->lastassignB);
        }
      }    
      if( TimeDynamicSegment == sc_core::sc_time(0,sc_core::SC_NS) )
        //Quick-FIX Falls KEIN dynamischer Teil benutzt wird..
        time=sc_core::sc_time(0,sc_core::SC_NS);

    }
    return true;   
  }
  
  
  void FlexRayScheduler::addedNewTask(TaskInstanceImpl *task){    
    int index = PIDmap[task->getProcessId()];
         // std::cerr<<"addedNewTask- index: "<<index<<" PID: "<<task->getProcessId()<<" instanceID: "<<task->getInstanceId()<<" name:"<<task->getName()<<std::endl;
    if(index<StartslotDynamic){
      //TDMA-Task
      TDMA_slots[ index ].pid_fifo.push_back(task->getInstanceId());
      ProcessParams[task->getInstanceId()]=ProcessParams[task->getProcessId()];
//#ifdef VPC_DEBUG     
      //std::cout<<"added Process " <<  task->getInstanceId() << " to Slot " << PIDmap[task->getProcessId()]  <<std::endl;
//#endif //VPC_DEBUG
    
      //std::cout << "added static Task" <<std::endl;
    }else{
        //std::cout << "added Dynamic Task at Slot " << index - StartslotDynamic  <<std::endl;
      Dynamic_slots[ index - StartslotDynamic ].pid_fifo.push_back(task->getInstanceId());
    }
    
    if(processcount==0 /*&& firstrun == false */){
      cyclecount = (int) (sc_core::sc_time_stamp() / cycle_length);
      to_init=true;
      // std::cout<<"new cyclecount: "<<cyclecount<<std::endl;
     }
     //firstrun=false;
     processcount++;
  }
  
  void FlexRayScheduler::removedTask(TaskInstanceImpl *task){ 
    int index = PIDmap[task->getProcessId()];
    
    // std::cout<<"Task entfernt! @ "<< sc_core::sc_time_stamp() << "  " << index << std::endl;
      
    std::deque<int>::iterator iter;
    if(index<StartslotDynamic){
      for(iter = TDMA_slots[ index ].pid_fifo.begin(); iter!=TDMA_slots[index].pid_fifo.end() ;iter++){
        if( *iter == task->getInstanceId()){
          TDMA_slots[index].pid_fifo.erase(iter);
          break;
        }
      }
    }else{
      index -= StartslotDynamic;
      for(iter = Dynamic_slots[index].pid_fifo.begin(); iter!=Dynamic_slots[index].pid_fifo.end() ;iter++){
        if( *iter == task->getInstanceId()){
          Dynamic_slots[index].pid_fifo.erase(iter);
          break;
        }
      }
    }
#ifdef VPC_DEBUG    
    std::cout<<"removed Task: " << task->getInstanceId()<<std::endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision
  FlexRayScheduler::schedulingDecision(int& task_to_resign,
                                       int& task_to_assign,
                                       const TaskMap &ready_tasks,
                                       const TaskMap &running_tasks )
  {
    scheduling_decision ret_decision = NOCHANGE;
    //statischer oder dynamischer Teil?
    if(curr_slicecount+1<StartslotDynamic){
      //     std::cout<<"Static! @ "<< sc_core::sc_time_stamp() << " curr slice: " << curr_slicecount+1 <<" cycle: "<< cyclecount<< std::endl;
      //TDMA-Scheduling: unveraendert aus TDMAScheduler verwendet.
      ret_decision=NOCHANGE;
      //Zeitscheibe abgelaufen?
      if(this->remainingSlice < (sc_core::sc_time_stamp() - this->lastassign)) this->remainingSlice=sc_core::SC_ZERO_TIME;
      else{
        this->remainingSlice = this->remainingSlice - (sc_core::sc_time_stamp() - this->lastassign);  
      }
      this->lastassign = sc_core::sc_time_stamp();
    
      if(this->remainingSlice <= sc_core::sc_time(0,sc_core::SC_NS)){//Zeitscheibe wirklich abgelaufen!
        curr_slicecount++; // Wechsel auf die naechste Zeitscheibe noetig!
        //neue Timeslice laden
        this->remainingSlice = TDMA_slots[curr_slicecount].length;
        
        //Korrekturfaktor falls mitten im Slot
        if(to_init == true && running_tasks.size()==0) {
          // std::cout<<"FIXME - curr_slicecount: "<< curr_slicecount<<" curr_remaining_slice="<< this->remainingSlice<< " @" <<sc_core::sc_time_stamp()<<std::endl;
          to_init=false;
          // std::cout<<"remainingSlice: " << this->remainingSlice <<"  "<<sc_core::sc_time_stamp()<<" - "<< cycle_length <<" * "<< cyclecount<<std::endl;

          if(sc_core::sc_time_stamp() < cycle_length * cyclecount){
            //Quick-FIX: Bug if last Slot is completely empty
            this->remainingSlice = sc_core::sc_time_stamp() - cycle_length * (cyclecount-1);
          }else{
            this->remainingSlice = sc_core::sc_time_stamp() - cycle_length * cyclecount;
          }
          // std::cout<<"new temp value: "<<this->remainingSlice<<std::endl;
          for(int i_fix = 0; i_fix < StartslotDynamic; i_fix++){
            if( this->remainingSlice > TDMA_slots[i_fix].length){
              this->remainingSlice = this->remainingSlice  - TDMA_slots[i_fix].length;
              // std::cout<<"new value: " << i_fix << "with: "<< TDMA_slots[i_fix].length <<" = "<<this->remainingSlice<<std::endl;
            }else{
              this->remainingSlice = TDMA_slots[i_fix].length - this->remainingSlice;
              curr_slicecount = i_fix;
              break;
            }
          }
          //std::cout<<"new value: "<<this->remainingSlice<<std::endl; 
        }
        if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){    // neuer Task da?
          unsigned int tempcount=0;
          bool found=false;
          //if not.. try the next one (if existing)
          while(!found && tempcount<TDMA_slots[curr_slicecount].pid_fifo.size()){
            task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];
            //is this task allowed to run in this slot?
            //            std::cout<<"testing "<<tempcount<<" @ "<<curr_slicecount<<" bei "<<task_to_assign<<" von gesamt: "<<TDMA_slots[curr_slicecount].pid_fifo.size()<<std::endl;
            if(ProcessParams[task_to_assign].offset<=cyclecount){
              //          std::cout<<"found task_to_assign: "<<task_to_assign<<std::endl;
              int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
              if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
                /* std::cout<<"Abfrage: mux= "<< ProcessParams[task_to_assign].multiplex << std::endl;
                   std::cout<<" count= "<<cyclecount <<" 2^ = "<< mux_value<<std::endl;
                   std::cout<< " MOD= " << cyclecount % mux_value <<" @ "<< sc_core::sc_time_stamp()<<std::endl;
                */
                found=true;
              }
            }
            tempcount++;
          }
        
          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          //         std::cout<<"here we are"<<std::endl;
          ret_decision= ONLY_ASSIGN;
        
          if(!found){ //keinen lauffaehigen gefunden! -> idle werden
            task_to_assign=0;
            if(running_tasks.size()!=0){  // alten Task entfernen, wenn noetig
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              TaskInstanceImpl *task=iter->second;

              task_to_resign=task->getInstanceId();
              ret_decision=RESIGNED;
            }else{
              //war keiner da... und ist auch kein Neuer da -> keine Aenderung  
              ret_decision=NOCHANGE;
            }  
          }else{
        
            if(running_tasks.size()!=0){  // alten Task entfernen
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              TaskInstanceImpl *task=iter->second;
              task_to_resign=task->getInstanceId();
              ret_decision= PREEMPT;  
            }
          }
          // else{}

          //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
        }else{
          //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
          if(running_tasks.size()!=0){  // alten Task entfernen
            TaskMap::const_iterator iter;
            iter=running_tasks.begin();
            TaskInstanceImpl *task=iter->second;
            task_to_resign=task->getInstanceId();
            ret_decision=RESIGNED;
          }else{
            //war keiner da... und ist auch kein Neuer da -> keine Aenderung    
            ret_decision=NOCHANGE;
          }             
        }    
      }else{
        //neuer Task hinzugefuegt -> nichts tun 
        //oder alter entfernt    -> neuen setzen
        //neuen setzen:
        if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
          //   std::cout<<"Task fertig!"<<std::endl;
          if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){            // ist da auch ein neuer da?        
            unsigned int tempcount=0;
            bool found=false;
            //if not.. try the next one (if existing)
            while(!found && tempcount<TDMA_slots[curr_slicecount].pid_fifo.size()){
              task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];
          

              //is this task allowed to run in this slot? or is an offset required?
              if(ProcessParams[task_to_assign].offset<cyclecount){
                //potenzieren des cycle-multiplex
                int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
                //no multiplex - run it  -- 
                if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
                    /*std::cout<<"Abfrage: mux= "<< ProcessParams[task_to_assign].multiplex << std::endl;
                      std::cout<<" count= "<<cyclecount <<" 2^ = "<< mux_value<<std::endl;
                      std::cout<< " MOD= " << cyclecount % mux_value <<" @ "<< sc_core::sc_time_stamp()<<std::endl;
                     */                  
                  found=true;
                }
              }
              tempcount++;
            }
        
            if(!found){
              ret_decision=NOCHANGE;
            }else{
              //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
              // -> kein preemption!
              ret_decision= ONLY_ASSIGN;
            }
          }     
        }
        //neuer Task hinzugefuegt, aber ein anderer laeuft noch -> nichts tun
      }
    }else{
      //Dynamischer Teil von FlexRay  
      if((curr_slicecount+1)==StartslotDynamic && this->remainingSlice > sc_core::sc_time(0,sc_core::SC_NS) ){
        //Restzeit des statischen Abschnitts "verbraten"
        ret_decision=NOCHANGE;
        if(this->remainingSlice < (sc_core::sc_time_stamp() - this->lastassign)) this->remainingSlice=sc_core::SC_ZERO_TIME;
        else{ this->remainingSlice = this->remainingSlice - (sc_core::sc_time_stamp() - this->lastassign);  }
        this->lastassign = sc_core::sc_time_stamp();
        
      }else{
    
        //      std::cout<<"Dynamic! "<< curr_slicecount << "  " << sc_core::sc_time_stamp()<< std::endl;
        if(curr_slicecount+1==StartslotDynamic){
          //Start des Dyn. Segments
          curr_slicecount++;
          this->remainingSlice=TimeDynamicSegment;
          curr_slicecountA=0;
          remainingSliceA=sc_core::sc_time(0,sc_core::SC_NS);
          curr_slicecountB=0;
          remainingSliceA=sc_core::sc_time(0,sc_core::SC_NS);
         
          //auf alle Faelle den letzten (periodischen) Task verdraengen!
          if(running_tasks.size()!=0){  // alten Task entfernen
            TaskMap::const_iterator iter;
            iter=running_tasks.begin();
            TaskInstanceImpl *task=iter->second;
            task_to_resign=task->getInstanceId();
            ret_decision=RESIGNED;
          }else{
            //war keiner da... und ist auch kein Neuer da -> keine Aenderung    
            ret_decision=NOCHANGE;
          }      
          this->lastassign = sc_core::sc_time_stamp();
        }else{  
          //Zeitscheibe abgelaufen? -> ende des dyn. Teils.. und Wechsel zurueck zum statischen
          if(this->remainingSlice <=(sc_core::sc_time_stamp() - this->lastassign)){
            if(running_tasks.size()!=0){  // alten Task entfernen
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              TaskInstanceImpl *task=iter->second;
              task_to_resign=task->getInstanceId();
              ret_decision=RESIGNED;
            }else{
              //war keiner da... und ist auch kein Neuer da -> keine Aenderung  
              ret_decision=NOCHANGE;
            }    
                
            if(running_tasks.size()<2){
              this->remainingSlice=sc_core::SC_ZERO_TIME;
              //dynamischer Teil "aufgeraeumt" -> statischer kann starten.
              curr_slicecount=-1;
              //naechster Zyklus beginnt
              cyclecount++;
            }
            this->lastassign=sc_core::sc_time_stamp();
            curr_slicecountA=0;
            curr_slicecountB=0;
            // std::cout<<"Dyn Ende! "<< this->lastassign <<std::endl;
                
          }else{ //Normalfall im dynamischen Teil.. -> ByteFlight-Protokoll!
            if(running_tasks.size() ==0){ //es laeuft grad keiner -> aktuellen auf Kanal A einlasten
              if(curr_slicecountA + StartslotDynamic < slicecount){
                if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){
                  task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
                  taskAssignedToA=task_to_assign;
                  ret_decision= ONLY_ASSIGN;
                  remainingSliceA=Dynamic_slots[curr_slicecountA].length;
                  lastassignA=sc_core::sc_time_stamp();
                  curr_slicecountA++;
                }else{//oder doch Kanal B nehmen? ;-)
                  if(curr_slicecountB + StartslotDynamic < slicecount){
                    if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){
                      task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
                      taskAssignedToB=task_to_assign;
                      ret_decision= ONLY_ASSIGN;        
                      remainingSliceB=Dynamic_slots[curr_slicecountB].length;
                      lastassignB=sc_core::sc_time_stamp();
                      curr_slicecountB++;
                    }else{
                      //hm, keiner da :-(  -> Zeit/Zaehler vorranschreiten lassen.
                      //bzw. Minislot ueberspringen
                      curr_slicecountB++;
                      curr_slicecountA++;
                      ret_decision=NOCHANGE;
                    }
                  }else{ret_decision=NOCHANGE;}
                }
              }else{ret_decision=NOCHANGE;}
            }else{
                
              if(running_tasks.size() ==1){ //es laeft ein Prozess -> pruefen auf welchem Kanal
                if(remainingSliceA > sc_core::sc_time(0,sc_core::SC_NS)){ //auf Kanal A =D
                  //und ob er noch weiterlaufen darf ;-) 
                
                  //-------------------------------------------         
                  if(this->remainingSliceA < (sc_core::sc_time_stamp() - this->lastassignA)){ 
                    this->remainingSliceA=sc_core::SC_ZERO_TIME;
                  }else{
                    this->remainingSliceA = this->remainingSliceA - (sc_core::sc_time_stamp() - this->lastassignA);  
                  }
                  this->lastassignA = sc_core::sc_time_stamp();
                  if(this->remainingSliceA <= sc_core::sc_time(0,sc_core::SC_NS)){//Zeitscheibe wirklich abgelaufen!
                    if(curr_slicecountA + StartslotDynamic +1 < slicecount){
                      // std::cout<<"next slicecount dynamic"<<std::endl;
          
                      //                curr_slicecountA++; // Wechsel auf die naechste Zeitscheibe noetig!
                      //neue Timeslice laden
                      this->remainingSliceA = Dynamic_slots[curr_slicecountA].length;
                      if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){    // neuer Task da?
                        task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
                        taskAssignedToA=task_to_assign;
                        TaskMap::const_iterator iter;
                        iter=running_tasks.begin();
                        TaskInstanceImpl *task=iter->second;
                        task_to_resign=task->getInstanceId();
                        ret_decision= PREEMPT;  
                      }else{
                        //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
                        // alten Task entfernen
                        TaskMap::const_iterator iter;
                        iter=running_tasks.begin();
                        TaskInstanceImpl *task=iter->second;
                        task_to_resign=task->getInstanceId();
                        ret_decision=RESIGNED;
                        taskAssignedToA=0;
                      }    
                    }else{
                      TaskMap::const_iterator iter;
                      iter=running_tasks.begin();
                      TaskInstanceImpl *task=iter->second;
                      task_to_resign=task->getInstanceId();
                      ret_decision=RESIGNED;
                      taskAssignedToA=0;          
                    }
                  }else{ 
        
                    if(dualchannel){
                      //Task auf Kanal A noch weiterhin aktiv ->  minislot auf B vergangen -> B schedulen
                      if(curr_slicecountB + StartslotDynamic +1< slicecount){           
                        curr_slicecountB++;
                        if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){            // ist da auch ein neuer da?
                          task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
                          taskAssignedToB=task_to_assign;
                          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
                          // -> kein preemption!
                          ret_decision= ONLY_ASSIGN;
                          if(task_to_assign == taskAssignedToA){ //d.h. es laeuft bereits ein Task dieser Node
                            if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>1){ // den naechsten nehmen
                              task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo[2];
                              taskAssignedToB=task_to_assign;
                              ret_decision= ONLY_ASSIGN;
                            }else{
                              ret_decision= NOCHANGE;
                            }
                          }else{
                            remainingSliceB=Dynamic_slots[curr_slicecountB].length;
                          }
                        }
                      }else{ret_decision=NOCHANGE;}
                    }else{
                      //wenn kein Dualchannel, dann darf man hier nichts machen ;)
                      ret_decision=NOCHANGE;
                    }
                  }             
                  //------------------------------------------- 
                }else{ //na, dann wohl das Ganze nochmal auf Kanal B ;-)
                  // hier duerfte man bei dualchannel=false normalerweise niemals hinkommen!
                  if(dualchannel){
                
                    //-------------------------------------------               
                    if(this->remainingSliceB < (sc_core::sc_time_stamp() - this->lastassignB)) 
                      this->remainingSliceB=sc_core::SC_ZERO_TIME;
                    else{
                      this->remainingSliceB = this->remainingSliceB - (sc_core::sc_time_stamp() - this->lastassignB);  
                    }
                    this->lastassignB = sc_core::sc_time_stamp();
                                
                    if(this->remainingSliceB <= sc_core::sc_time(0,sc_core::SC_NS)){//Zeitscheibe wirklich abgelaufen!
                      if(curr_slicecountB + StartslotDynamic +1 < slicecount){
                        curr_slicecountB++; // Wechsel auf die naechste Zeitscheibe noetig!
                        //neue Timeslice laden
                        this->remainingSliceB = Dynamic_slots[curr_slicecountB].length;
        
                        if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){    // neuer Task da?
                          task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
                          taskAssignedToB=task_to_assign;
                          TaskMap::const_iterator iter;
                          iter=running_tasks.begin();
                          TaskInstanceImpl *task=iter->second;
                          task_to_resign=task->getInstanceId();
                          ret_decision= PREEMPT;  
                        }else{
                          //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
                          // alten Task entfernen
                          TaskMap::const_iterator iter;
                          iter=running_tasks.begin();
                          TaskInstanceImpl *task=iter->second;
                          task_to_resign=task->getInstanceId();
                          ret_decision=RESIGNED;
                          taskAssignedToB=0;
                        }    
                      }else{ret_decision=NOCHANGE;}
                    }else{ //Task auf Kanal B noch weiterhin aktiv ->  minislot auf A vergangen -> A schedulen
                      if(curr_slicecountA + StartslotDynamic +1< slicecount){
                        curr_slicecountA++;
                        if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){            // ist da auch ein neuer da?
                          task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
                          taskAssignedToA=task_to_assign;
                          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
                          // -> kein preemption!
                          ret_decision= ONLY_ASSIGN;
                          if(task_to_assign == taskAssignedToB){ //d.h. es laeuft bereits der Task dieser Node
                            if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>1){ // den naechsten nehmen
                              task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo[2];
                              taskAssignedToA=task_to_assign;
                              ret_decision= ONLY_ASSIGN;
                            }else{
                              ret_decision= NOCHANGE;
                            }
                          }else{
                            remainingSliceA=Dynamic_slots[curr_slicecountA].length;     
                          }
                        }
                      }else{ret_decision=NOCHANGE;}
                    }           
                  }
                  ret_decision=NOCHANGE;
                }
                
                /*
                  TaskMap::const_iterator iter;
                  iter=running_tasks.begin();
                  Task *task=iter->second;
                  task->getInstanceId();
                */
                //              std::cout <<"Ende: "<< ret_decision << "  " << task_to_assign << "  " << running_tasks.size() << "  " <<  task->getInstanceId() << std::endl;
              }
              if(running_tasks.size() ==2){ // einer von beiden muss abgelaufen sein... welcher?
                //auch hier darf man bei dualchannel=false Niemals hinkommen!
                assert(dualchannel==true);
                if(this->remainingSliceA < (sc_core::sc_time_stamp() - this->lastassignA)){      
                  //na, es war wohl A
                  this->remainingSliceA=sc_core::SC_ZERO_TIME;
                  this->remainingSliceB = this->remainingSliceB - (sc_core::sc_time_stamp() - this->lastassignB);  
                  //-------------       
                  if(curr_slicecountA + StartslotDynamic +1 < slicecount){
                    curr_slicecountA++; // Wechsel auf die naechste Zeitscheibe noetig!
                    //neue Timeslice laden
                    this->remainingSliceA = Dynamic_slots[curr_slicecountA].length;
                
                    if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){    // neuer Task da?
                      task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
                      task_to_resign=taskAssignedToA;
                      taskAssignedToA=task_to_assign;
                      ret_decision= PREEMPT;  
                    }else{
                      //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!      
                      task_to_resign=taskAssignedToA;
                      ret_decision=RESIGNED;
                      taskAssignedToA=0;
                    }    
                  }else{
                    task_to_resign=taskAssignedToA;
                    ret_decision=RESIGNED;
                    taskAssignedToA=0;
                  }
                }else{
                  // na, dann wohl B
                  this->remainingSliceB=sc_core::SC_ZERO_TIME;
                  this->remainingSliceA = this->remainingSliceA - (sc_core::sc_time_stamp() - this->lastassignA);  
                        
                  if(curr_slicecountA + StartslotDynamic +1 < slicecount){
                    curr_slicecountB++; // Wechsel auf die naechste Zeitscheibe naeig!
                    //neue Timeslice laden
                    this->remainingSliceB = Dynamic_slots[curr_slicecountB].length;
                
                    if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){    // neuer Task da?
                      task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
                      task_to_resign=taskAssignedToB;
                      taskAssignedToB=task_to_assign;
                      ret_decision= PREEMPT;  
                      if(task_to_assign == taskAssignedToA){
                                
                        if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>1){ //den naechsten bitte
                          task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo[2];
                          taskAssignedToB=task_to_assign;
                          ret_decision= PREEMPT;
                        }else{
                          ret_decision= RESIGNED;
                          taskAssignedToB=0;
                        }       
                      }
                    }else{
                      //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
                      task_to_resign=taskAssignedToB;
                      ret_decision=RESIGNED;
                      taskAssignedToB=0;
                    }    
                  }else{
                    task_to_resign=taskAssignedToB;
                    ret_decision=RESIGNED;
                    taskAssignedToB=0;
                  }
                }               
                this->lastassignA = sc_core::sc_time_stamp();
                this->lastassignB = sc_core::sc_time_stamp();
              }
            }                           
          }
        }
      }
    } 
    
    /* if(ret_decision != NOCHANGE){
       std::cout << sc_core::sc_time_stamp() << " Decision: " << ret_decision << "newTask: " << task_to_assign  << " old task: " << task_to_resign << " Timeslice: " << this->remainingSlice << "  "<< remainingSliceA << "  " << remainingSliceB <<std::endl;
      }
    */

#ifdef VPC_DEBUG  
    std::cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
         << " old task: " << task_to_resign <<  "Timeslice: " << this->remainingSlice << " " << sc_core::sc_time_stamp() <<  std::endl;
#endif //VPC_DEBUG  
    return ret_decision;
  }


  /**
   *
   */
  sc_core::sc_time* FlexRayScheduler::schedulingOverhead(){
    return NULL; //new sc_core::sc_time(1,sc_core::SC_NS);
    
    //     return new sc_core::sc_time(1,sc_core::SC_NS);
  }

} } // namespace SystemC_VPC::Detail
