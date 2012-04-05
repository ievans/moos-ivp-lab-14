/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Coordinator_HEADER
#define Coordinator_HEADER

#include "MOOSLib.h"

class Coordinator : public CMOOSApp
{
 public:
   Coordinator();
   ~Coordinator();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

private:
	// insert local vars here
};

#endif 
