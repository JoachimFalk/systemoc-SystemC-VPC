/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifdef WIN32

#include <systemcvpc/dynload/world.hpp>
#include <systemcvpc/dynload/dll.hpp>


DLLManager::DLLManager( const char *fname )
{
    // Try to open the library now and get any error message.
	
	h = LoadLibrary(fname);
	
	if (h == NULL)
	{
		DWORD m;
		m = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  GetLastError(),   /* Message Number */
					  0,   				/* Language */
					  err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */
		std::cout << err << std::endl;
	}
	else
	{
		err = NULL;
	}
}

DLLManager::~DLLManager()
{
	// Free error string if allocated
	LocalFree(err);
	
	// close the library if it isn't null
	if (h != NULL)
    	FreeLibrary(h);
    	
}


bool DLLManager::GetSymbol( 
			   void **v,
			   const char *sym_name
			   )
{
	// try extract a symbol from the library
	// get any error message is there is any
	
	if( h!=0 )
	{
		*v = GetProcAddress(h, sym_name);
	    if (v != NULL)
       	  return true;
    	else
    	{
    		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  GetLastError(),   /* Message Number */
					  0,   				/* Language */
					  err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */
    		return false;
    	}
	}
	else
	{	
        return false;
	}
	
}

const char * DLLManager::GetDLLExtension()
{
	return ".dll";
}

DLLFactoryBase::DLLFactoryBase(
			       const char *fname,
			       const char *factory
			       ) : DLLManager(fname)
{
	// try get the factory function if there is no error yet
	
	factory_func=0;
	
	if( LastError()==0 )
	{		
    	GetSymbol( (void **)&factory_func, factory ? factory : "factory0" );
	}
	
}


DLLFactoryBase::~DLLFactoryBase()
{
}


#endif /* WIN32 */
