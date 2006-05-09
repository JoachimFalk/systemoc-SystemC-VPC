#ifndef HSCD_VPC_MIMAPPER_H_
#define HSCD_VPC_MIMAPPER_H_

#include <map>
#include <set>
#include <string>

#include "hscd_vpc_MappingInformation.h"

namespace SystemC_VPC {

  /**
   * \brief Enables access to MappingInformations associated with a target component
   */
  class MappingInformationIterator {

    private:

      std::set<MappingInformation* >* mInfos;
      std::set<MappingInformation* >::iterator _iter;

    public:

      MappingInformationIterator(std::set<MappingInformation* >* mInfos) : mInfos(mInfos) {
        if(mInfos != NULL){
          _iter = mInfos->begin();
        }
      }

      ~MappingInformationIterator() {}
        
      bool hasNext(){
        return (_iter != mInfos->end());
      }

      MappingInformation* getNext(){
        MappingInformation* mI = *_iter;
        _iter++;
        return mI;
      }

      void reset() {
        _iter = mInfos->begin();
      }

  };

  /**
   * \brief Simple management class for managing MappingInformation instances
   */
  class MIMapper {

    private:

      std::map<std::string, std::set<MappingInformation* > > mInfos;

    public:

      MIMapper();

      ~MIMapper();

      void addMappingInformation(std::string key, MappingInformation* mInfo);

      /**
       * \brief Gets iterator over all MappingInformations associated with component of Binding
       * \param key specifies the successing component to get information for
       */
      MappingInformationIterator* getMappingInformationIterator(std::string& key);

  };

}


#endif //HSCD_VPC_MIMAPPER_H_