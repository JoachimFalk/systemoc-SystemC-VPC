/* 
	FlexRay-Scheduler
	
	CMX-Format:
	Slotkennzeichen: type="slotxxxxx"
	Funktionszuordnung: value="slotxxxxx"
	
	z.B.
	<component name="Component1" type="threaded" scheduler="TDMA">
   	<attribute type="slot0" value="20ns"/>
   	<attribute type="periodic.task1" value="slot0"/>
   	<attribute type="slot1" value="20ns"/>
   	<attribute type="periodic.task2" value="slot1"/>
  	</component>
	
	aktuelle Zuordnung: TDMA_slots bis [StartslotDynamic-1] festes TDMA, danach Dynamischer Teil
	
	Sebastian Graf - Oktober 2007

*/

#include <FlexRayScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>

namespace SystemC_VPC{

  FlexRayScheduler::FlexRayScheduler(const char *schedulername)
    : _properties() {
    //neu FlexRay
    StartslotDynamic = 2;
    TimeDynamicSegment = Director::createSC_Time("50ns");
        
    //alt (TDMA)
    processcount=0;
    lastassign=sc_time(0,SC_NS);
    this->remainingSlice = sc_time(0,SC_NS);
    slicecount=0;
    curr_slicecount=-1; 
    curr_slicecountA=0;
    curr_slicecountB=0; 
    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
     //':' finden -> ':' trennt key-value Paare 
    char *firstindex=strchr(schedulername,':');
    while(firstindex!=NULL){

      //':' überspringen und nächste ':' finden
      secondindex=strchr(firstindex+1,':');
      if(secondindex!=NULL)
        sublength=secondindex-firstindex;          //Länge bestimmen
      else
        sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     
     
      // key und value trennen und Property setzen
      char *key, *value;
      value=strstr(rest,"-");
      if(value!=NULL){
        value[0]='\0';
        value++;
        key=rest;
        setProperty(key,value);
      }
    }
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
	cout << "------------ END Initialize ---------"<<endl;
#endif //VPC_DEBUG 	
	this->_properties.clear();
  }
  
 
  void FlexRayScheduler::_setProperty(const char* key, const char* value){
  	char *domain;
	int slot;
    	//Herausfinden, welcher Slot genannt ist + welche Zeit ihm zugeordnet wird      
    	if(0==strncmp(key,"slot",strlen("slot"))){
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
				cout<<"new Dynamic One!"<<endl;
				}
				slicecount++;
				/*Erzeugen einer Info-Ausgabe		
				domain[0]='\0';
				sscanf(value,"%lf",&slottime);
				std::cout << "Datensatz für Slot Nr." << slot 
				<<"gefunden! TDMA-Slotdauer: " <<slottime << "ns"<<std::endl;
		*/		
			}
      		}
    	}else if(0==strncmp(value,"slot",strlen("slot"))){
    		int i=-1;
		//schon Slots geadded? oder cmx-Syntax/Reihenfolge falsch?
		assert(0<TDMA_slots.size());
		//Betreffende SlotID in der Slotliste suchen
    		do{
			//nichts zu tun.. da lediglich durchiteriert wird!
		}while(TDMA_slots[++i].name != value && (i+1)<TDMA_slots.size());
    	 	cout<<"found at i= " <<i<<endl;
		
		if(TDMA_slots[i].name != value){
		i=-1;
		assert(0<Dynamic_slots.size());
		cout<< "DynSize: " << Dynamic_slots.size()<<endl;
		do{
			//nichts zu tun.. da lediglich durchiteriert wird!
		}while(Dynamic_slots[++i].name != value && (i+1)<Dynamic_slots.size());
		//auch wirklich etwas passendes gefunden?		
		assert(i<Dynamic_slots.size());	
		i+=StartslotDynamic;	
		}
		cout<<"PId-Map "<<i<< " to "<< Director::getInstance().getProcessId(key) <<endl;
		//Beziehung PId - SlotID herstellen
  		PIDmap[Director::getInstance().getProcessId(key)]=i;   
// 		cout<<"add Function " <<  key << " to " << value<<endl;
    	}	
  }

  
  bool FlexRayScheduler::getSchedulerTimeSlice( sc_time& time,
  	 	const std::map<int,ProcessControlBlock*> &ready_tasks,
    		const  std::map<int,ProcessControlBlock*> &running_tasks )
  {   
  cout<<"moep "<< running_tasks.size() << endl;
    // keine wartenden + keine aktiven Threads -> ende!
    if(processcount==0 && running_tasks.size()==0) return 0;   
    //ansonsten: Restlaufzeit der Zeitscheibe
    if(curr_slicecount<StartslotDynamic){ //statisch
    cout<<"moep1 - "<< curr_slicecount<<endl;   
    	if(curr_slicecount == -1){
		time=sc_time(0,SC_NS);
	}else{
    	time=TDMA_slots[curr_slicecount].length -(sc_time_stamp() - this->lastassign);  
//     	cout<<"static-timeSlice-request "<< curr_slicecount << "  " << sc_time_stamp() << "  " << time  << "running_tasks " << running_tasks.size() <<endl;
	}
    }else{
    cout<<"moep2"<<endl;   
    //Dynamisch
//     cout<<"dynamic-timeSlice-request "<< curr_slicecount << "  " << sc_time_stamp() << "  " << time << "running_tasks " << running_tasks.size() <<endl;
    if(running_tasks.size()<=1){
    cout<<"moep2-1 " << running_tasks.size() <<endl;   
    //gerade kein (dynamic) Task aktiv.. -> nächster Schedulevorgang!
    //oder nur einer -> ein Kanal frei!
       time=sc_time(10,SC_NS); //sofortiges Reschedule bewirken!
    }else{
    	//beide Kanäle voll.. -> nächster Schedulevorgang nach Ende von einem...
	cout<<"moep2-2"<<endl;   
	if(remainingSliceA < remainingSliceB)
    	time=remainingSliceA - (sc_time_stamp() - this->lastassignA);
    	else 
	time=remainingSliceB - (sc_time_stamp() - this->lastassignB);
    }    
    }
    return true;   
  }
  
  
  void FlexRayScheduler::addedNewTask(ProcessControlBlock *pcb){    
     int index = PIDmap[pcb->getPid()];
     cout<< "added index " << index<<endl;
     if(index<StartslotDynamic){
     //TDMA-Task
     TDMA_slots[ index ].pid_fifo.push_back(pcb->getInstanceId());
#ifdef VPC_DEBUG     
     cout<<"added Process " <<  pcb->getInstanceId() << " to Slot " << PIDmap[pcb->getPid()]  <<endl;
#endif //VPC_DEBUG
    
	cout << "added static Task" <<endl;
     }else{
     cout << "added Dynamic Task" <<endl;
     Dynamic_slots[ index - StartslotDynamic ].pid_fifo.push_back(pcb->getInstanceId());
     }
     processcount++;
  }
  
  
  void FlexRayScheduler::removedTask(ProcessControlBlock *pcb){ 
  
  cout<<"Task entfernt! @ "<< sc_time_stamp() << endl;
  
    int index = PIDmap[pcb->getPid()];
    std::deque<ProcessId>::iterator iter;
    if(index<StartslotDynamic){
    for(iter = TDMA_slots[ index ].pid_fifo.begin(); iter!=TDMA_slots[index].pid_fifo.end() ;iter++){
      if( *iter == pcb->getInstanceId()){
        TDMA_slots[index].pid_fifo.erase(iter);
        break;
      }
    }
    }else{
    index -= StartslotDynamic;
    for(iter = Dynamic_slots[index].pid_fifo.begin(); iter!=Dynamic_slots[index].pid_fifo.end() ;iter++){
    	if( *iter == pcb->getInstanceId()){
        	Dynamic_slots[index].pid_fifo.erase(iter);
        	break;
      	}
      }
    }
#ifdef VPC_DEBUG    
    cout<<"removed Task: " << pcb->getInstanceId()<<endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision FlexRayScheduler::schedulingDecision(
    		int& task_to_resign,
    		int& task_to_assign,
    		const  std::map<int,ProcessControlBlock*> &ready_tasks,
    		const  std::map<int,ProcessControlBlock*> &running_tasks )
  {
    scheduling_decision ret_decision = NOCHANGE;;
    
    //statischer oder dynamischer Teil?
    if(curr_slicecount+1<StartslotDynamic){
    cout<<"Static! @ "<< sc_time_stamp() << "  " << curr_slicecount+1 << endl;
    //TDMA-Scheduleing: unverändert aus TDMAScheduler verwendet.
    ret_decision=NOCHANGE;
    //Zeitscheibe abgelaufen?
    if(this->remainingSlice < (sc_time_stamp() - this->lastassign)) this->remainingSlice=SC_ZERO_TIME;
    else{
    	this->remainingSlice = this->remainingSlice - (sc_time_stamp() - this->lastassign);  
    }
    this->lastassign = sc_time_stamp();
    
    if(this->remainingSlice <= sc_time(0,SC_NS)){//Zeitscheibe wirklich abgelaufen!
    	curr_slicecount++; // Wechsel auf die nächste Zeitscheibe nötig!
	//neue Timeslice laden
    	this->remainingSlice = TDMA_slots[curr_slicecount].length;

      if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){    // neuer Task da?
        task_to_assign = TDMA_slots[curr_slicecount].pid_fifo.front();
// 	cout<<"Scheduler:new task: " << task_to_assign << "..." <<endl;
        
	//alter wurde schon entfernt (freiwillige abgabe "BLOCK")
        // -> kein preemption!
        ret_decision= ONLY_ASSIGN;
	
        if(running_tasks.size()!=0){  // alten Task entfernen
          std::map<int,ProcessControlBlock*>::const_iterator iter;
          iter=running_tasks.begin();
          ProcessControlBlock *pcb=iter->second;
          task_to_resign=pcb->getInstanceId();
          ret_decision= PREEMPT;  
        }
        // else{}    ->
        //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
      }else{
      //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!
      if(running_tasks.size()!=0){  // alten Task entfernen
          std::map<int,ProcessControlBlock*>::const_iterator iter;
          iter=running_tasks.begin();
          ProcessControlBlock *pcb=iter->second;
          task_to_resign=pcb->getInstanceId();
          ret_decision=RESIGNED;
        }else{
	//war keiner da... und ist auch kein Neuer da -> keine Änderung	
      	ret_decision=NOCHANGE;
	}      	
      }    
    }else{
     	//neuer Task hinzugefügt -> nichts tun 
      	//oder alter entfernt    -> neuen setzen
      	//neuen setzen:
      if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
      cout<<"Task feddich!"<<endl;
        if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){            // ist da auch ein neuer da?
          task_to_assign = TDMA_slots[curr_slicecount].pid_fifo.front();

          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          ret_decision= ONLY_ASSIGN;
        }
      }
      //neuer Task hinzugefügt, aber ein anderer laeuft noch -> nichts tun
    }
    }else{
    	//Dynamischer Teil von FlexRay
	
	cout<<"Dynamic! "<< curr_slicecount << endl;
	
	if(curr_slicecount+1==StartslotDynamic){
	//Start des Dyn. Segments
	curr_slicecount++;
	 this->remainingSlice=TimeDynamicSegment;
	 curr_slicecountA=0;
	 remainingSliceA=sc_time(0,SC_NS);
	 curr_slicecountB=0;
	 remainingSliceA=sc_time(0,SC_NS);
	 
	 //auf alle Fälle den letzten (periodischen) Task verdrängen!
	 if(running_tasks.size()!=0){  // alten Task entfernen
          std::map<int,ProcessControlBlock*>::const_iterator iter;
          iter=running_tasks.begin();
          ProcessControlBlock *pcb=iter->second;
          task_to_resign=pcb->getInstanceId();
          ret_decision=RESIGNED;
         }else{
	  //war keiner da... und ist auch kein Neuer da -> keine Änderung	
      	  ret_decision=NOCHANGE;
	 }      
	 this->lastassign = sc_time_stamp();
	}
	
	 //Zeitscheibe abgelaufen? -> ende des dyn. Teils.. und Wechsel zurück zum statischen
	if(this->remainingSlice <(sc_time_stamp() - this->lastassign)){
	
		if(running_tasks.size()!=0){  // alten Task entfernen
         	 std::map<int,ProcessControlBlock*>::const_iterator iter;
         	 iter=running_tasks.begin();
          	ProcessControlBlock *pcb=iter->second;
         	 task_to_resign=pcb->getInstanceId();
         	 ret_decision=RESIGNED;
        	 }else{
	 	 //war keiner da... und ist auch kein Neuer da -> keine Änderung	
      	 	 ret_decision=NOCHANGE;
		 }    
		
		if(running_tasks.size()<2){
		this->remainingSlice=SC_ZERO_TIME;
		//dynamischer Teil "aufgeräumt" -> statischer kann starten.
		curr_slicecount=-1;
		}
		this->lastassign=sc_time_stamp();
		curr_slicecountA=0;
		curr_slicecountB=0;
		cout<<"Dyn Ende! "<< this->lastassign <<endl;
		
    	}else{ //Normalfall im dynamischen Teil.. -> ByteFlight-Protokoll!
		if(running_tasks.size() ==0){ //es l�uft grad keiner -> aktuellen auf Kanal A einlasten
		     if(curr_slicecountA + StartslotDynamic < slicecount){
			if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){
				task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
				taskAssignedToA=task_to_assign;
				curr_slicecountA++;
				ret_decision= ONLY_ASSIGN;
				remainingSliceA=Dynamic_slots[curr_slicecountA].length;
				remainingSliceA=sc_time_stamp();
			}else{//oder doch Kanal B nehmen? ;-)
			    if(curr_slicecountB + StartslotDynamic < slicecount){
				if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){
				task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
				taskAssignedToB=task_to_assign;
				curr_slicecountB++;
				ret_decision= ONLY_ASSIGN;	
				remainingSliceB=Dynamic_slots[curr_slicecountB].length;
				remainingSliceB=sc_time_stamp();
				}else{
				//hm, keiner da :-(  -> Zeit/Z�hler vorranschreiten lassen.
				//bzw. Minislot �berspringen
				curr_slicecountB++;
				curr_slicecountA++;
				ret_decision=NOCHANGE;
				}
			    }else{ret_decision=NOCHANGE;}
			}
		    }else{ret_decision=NOCHANGE;}
		}else{
		
		if(running_tasks.size() ==1){ //es l�uft ein Prozess -> pr�fen auf welchem Kanal
		if(remainingSliceA > sc_time(0,SC_NS)){ //auf Kanal A =D
		//und ob er noch weiterlaufen darf ;-) 
		
//-------------------------------------------		
	if(this->remainingSliceA < (sc_time_stamp() - this->lastassignA)){ 
			this->remainingSliceA=SC_ZERO_TIME;
		}else{
			this->remainingSliceA = this->remainingSliceA - (sc_time_stamp() - this->lastassignA);  
		}
		this->lastassignA = sc_time_stamp();
		if(this->remainingSliceA <= sc_time(0,SC_NS)){//Zeitscheibe wirklich abgelaufen!
	  if(curr_slicecountA + StartslotDynamic +1 < slicecount){		
		curr_slicecountA++; // Wechsel auf die nächste Zeitscheibe nötig!
		//neue Timeslice laden
		this->remainingSliceA = Dynamic_slots[curr_slicecountA].length;
		if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){    // neuer Task da?
			task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
			taskAssignedToA=task_to_assign;
			std::map<int,ProcessControlBlock*>::const_iterator iter;
			iter=running_tasks.begin();
			ProcessControlBlock *pcb=iter->second;
			task_to_resign=pcb->getInstanceId();
			ret_decision= PREEMPT;  
		}else{
		//kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!
		// alten Task entfernen
		std::map<int,ProcessControlBlock*>::const_iterator iter;
		iter=running_tasks.begin();
		ProcessControlBlock *pcb=iter->second;
		task_to_resign=pcb->getInstanceId();
		ret_decision=RESIGNED;
		taskAssignedToA=0;
		}    
	  }else{
	  	std::map<int,ProcessControlBlock*>::const_iterator iter;
		iter=running_tasks.begin();
		ProcessControlBlock *pcb=iter->second;
		task_to_resign=pcb->getInstanceId();
		ret_decision=RESIGNED;
		taskAssignedToA=0;	  
	  }
	}else{ //Task auf Kanal A noch weiterhin aktiv ->  minislot auf B vergangen -> B schedulen
	   if(curr_slicecountB + StartslotDynamic +1< slicecount){		
		curr_slicecountB++;
		if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){            // ist da auch ein neuer da?
		task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
		taskAssignedToB=task_to_assign;
		//alter wurde schon entfernt (freiwillige abgabe "BLOCK")
		// -> kein preemption!
		ret_decision= ONLY_ASSIGN;
		if(task_to_assign == taskAssignedToA){ //d.h. es l�uft bereits ein Task dieser Node
			if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>1){ // den n�chsten nehmen
			task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo[2];
			taskAssignedToB=task_to_assign;
			ret_decision= ONLY_ASSIGN;
			}else{
			ret_decision= NOCHANGE;
			}
		}
		}
	   }else{ret_decision=NOCHANGE;}
	}		
//-------------------------------------------	
	}else{ //na, dann wohl das Ganze nochmal auf Kanal B ;-)
		
//-------------------------------------------		
		if(this->remainingSliceB < (sc_time_stamp() - this->lastassignB)) 
			this->remainingSliceB=SC_ZERO_TIME;
		else{
			this->remainingSliceB = this->remainingSliceB - (sc_time_stamp() - this->lastassignB);  
		}
		this->lastassignB = sc_time_stamp();
				
		if(this->remainingSliceB <= sc_time(0,SC_NS)){//Zeitscheibe wirklich abgelaufen!
	   if(curr_slicecountB + StartslotDynamic +1 < slicecount){
		curr_slicecountB++; // Wechsel auf die nächste Zeitscheibe nötig!
		//neue Timeslice laden
		this->remainingSliceB = Dynamic_slots[curr_slicecountB].length;
	
		if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){    // neuer Task da?
			task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
			taskAssignedToB=task_to_assign;
			std::map<int,ProcessControlBlock*>::const_iterator iter;
			iter=running_tasks.begin();
			ProcessControlBlock *pcb=iter->second;
			task_to_resign=pcb->getInstanceId();
			ret_decision= PREEMPT;  
		}else{
	//kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!
		// alten Task entfernen
		std::map<int,ProcessControlBlock*>::const_iterator iter;
		iter=running_tasks.begin();
		ProcessControlBlock *pcb=iter->second;
		task_to_resign=pcb->getInstanceId();
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
		if(task_to_assign == taskAssignedToB){ //d.h. es l�uft bereits ein Task dieser Node
			if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>1){ // den n�chsten nehmen
			task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo[2];
			taskAssignedToA=task_to_assign;
			ret_decision= ONLY_ASSIGN;
			}else{
			ret_decision= NOCHANGE;
			}
		}
		}
	  }else{ret_decision=NOCHANGE;}
	}		
//-------------------------------------------	
		}
		
		std::map<int,ProcessControlBlock*>::const_iterator iter;
		iter=running_tasks.begin();
		ProcessControlBlock *pcb=iter->second;
		pcb->getInstanceId();
		
// 		cout <<"boing 1 - Ende: "<< ret_decision << "  " << task_to_assign << "  " << running_tasks.size() << "  " <<  pcb->getInstanceId() << endl;
		}
		if(running_tasks.size() ==2){ // einer von beiden muss abgelaufen sein... welcher?
		cout <<"boing 2"<<endl;
		if(this->remainingSliceA < (sc_time_stamp() - this->lastassignA)){ 	
		cout <<"boing 2-A"<<endl;
			//na, es war wohl A
			this->remainingSliceA=SC_ZERO_TIME;
			this->remainingSliceB = this->remainingSliceB - (sc_time_stamp() - this->lastassignB);  
		//-------------	
		   if(curr_slicecountA + StartslotDynamic +1 < slicecount){
			curr_slicecountA++; // Wechsel auf die nächste Zeitscheibe nötig!
			//neue Timeslice laden
			this->remainingSliceA = Dynamic_slots[curr_slicecountA].length;
		
		if(Dynamic_slots[curr_slicecountA].pid_fifo.size()>0){    // neuer Task da?
			task_to_assign = Dynamic_slots[curr_slicecountA].pid_fifo.front();
			task_to_resign=taskAssignedToA;
			taskAssignedToA=task_to_assign;
			ret_decision= PREEMPT;  
		}else{
		//kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!	
			task_to_resign=taskAssignedToA;
			ret_decision=RESIGNED;
			taskAssignedToA=0;
		}    
		}else{
			task_to_resign=taskAssignedToA;
			ret_decision=RESIGNED;
			taskAssignedToA=0;
		}
		//-------------	
    		}else{
		cout <<"boing 2-B"<<endl;
			// na, dann wohl B
			this->remainingSliceB=SC_ZERO_TIME;
    			this->remainingSliceA = this->remainingSliceA - (sc_time_stamp() - this->lastassignA);  
			
		//-------------	
		     if(curr_slicecountA + StartslotDynamic +1 < slicecount){
		     cout <<"boing 2-B-1"<<endl;
			curr_slicecountB++; // Wechsel auf die nächste Zeitscheibe nötig!
			//neue Timeslice laden
			this->remainingSliceB = Dynamic_slots[curr_slicecountB].length;
		
			if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>0){    // neuer Task da?
			cout <<"boing 2-B-1-1"<<endl;
				task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo.front();
				task_to_resign=taskAssignedToB;
				taskAssignedToB=task_to_assign;
				ret_decision= PREEMPT;  
				if(task_to_assign == taskAssignedToA){
				
				if(Dynamic_slots[curr_slicecountB].pid_fifo.size()>1){ // den n�chsten bitte
				task_to_assign = Dynamic_slots[curr_slicecountB].pid_fifo[2];
				taskAssignedToB=task_to_assign;
				ret_decision= PREEMPT;
				}else{
				ret_decision= RESIGNED;
				taskAssignedToB=0;
				}
				
				}
				
			}else{
			//kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!
				task_to_resign=taskAssignedToB;
				ret_decision=RESIGNED;
				taskAssignedToB=0;
			}    
		   }else{
		   cout <<"boing 2-B-2"<<endl;
			task_to_resign=taskAssignedToB;
			ret_decision=RESIGNED;
			taskAssignedToB=0;
		}
		cout <<"boing 2-C " << sc_time_stamp()<<endl;
		//-------------				
    		}    		
		this->lastassignA = sc_time_stamp();
    		this->lastassignB = sc_time_stamp();
		}
		}				
	}
	//Problem: 2 getrennte Kanäle..   
    } 
    
    cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
      << " old task: " << task_to_resign <<endl;

#ifdef VPC_DEBUG  
      cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
      << " old task: " << task_to_resign <<  "Timeslice: " << this->remainingSlice << endl;
#endif //VPC_DEBUG  
    return ret_decision;
  }


  /**
   *
   */
  sc_time* FlexRayScheduler::schedulingOverhead(){
     return NULL; //new sc_time(1,SC_NS);
    
//     return new sc_time(1,SC_NS);
  }

  /**
   * \brief Implementation of FlexRayScheduler::signalDeallocation
   */
  void FlexRayScheduler::signalDeallocation(bool kill){
  
  if(!kill){
    this->remainingSlice =
      this->remainingSlice - (sc_time_stamp() - this->lastassign);
      }else{
       
      //alle Prozesse aus den pid_fifos loeschen
          std::vector<TDMASlot>::iterator iter;
    		for(iter = TDMA_slots.begin(); iter!=TDMA_slots.end() ;iter++){
      		iter->pid_fifo.clear();        
      		}
      }
  }
  
  /**
   * \brief Implementation of FlexRayScheduler::signalAllocation
   */  
  void FlexRayScheduler::signalAllocation(){
    this->lastassign = sc_time_stamp();
  }
}
