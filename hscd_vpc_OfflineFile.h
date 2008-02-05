#ifndef HSCD_VPC_OFFLINEFILE_H_
#define HSCD_VPC_OFFLINEFILE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <systemc.h>

namespace SystemC_VPC{
  
  /**
   * OfflineFile realizes file access to the schedulerfile for settings of OfflineBinder and OfflineAllocator
   */
   
  class OfflineFile{
  
  private:
    std::ifstream file;
    const char *filename;
  
  public:
    OfflineFile::OfflineFile(const char*);
    OfflineFile::~OfflineFile();
    bool OfflineFile::open();
    void OfflineFile::close();
    int OfflineFile::getlength();
    std::string OfflineFile::getbuffer();
    void OfflineFile::print();
    
    void OfflineFile::cleanstring(std::string*);
    sc_time OfflineFile::generate_sctime(std::string);
  };
}
#endif //HSCD_VPC_OFFLINEFILE_H_