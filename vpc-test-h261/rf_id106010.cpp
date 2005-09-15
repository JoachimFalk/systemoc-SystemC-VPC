#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "rf_id106010.h"

void rf_id106010::rf_id106010_process()
{
while(true){
bool var_c_in_rf2rf_id106556 = port_c_in_rf2rf_id106556->read();
bool var_rf2c_rf_bm_id106524 = 0;
AbstractComponent& r=Director::getInstance().getResource(RF_ID106010);
smoc_event *ev=new smoc_event(); 
r.compute(RF_ID106010,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "RF triggered!" << endl << endl;
port_rf2c_rf_bm_id106524->write(var_rf2c_rf_bm_id106524);
}
}