/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Coordinator_HEADER
#define Coordinator_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

// Add "geometry" to src/pHandleSensorData/CMakeLists.txt
#include "GeomUtils.h"
#include "XYSegList.h"
#include "XYHazardSet.h"

// Local data structures
#include "Point2D.h"
#include "Order.h"

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
