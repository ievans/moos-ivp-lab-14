/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: FollowOrders.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef FollowOrders_HEADER
#define FollowOrders_HEADER

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
#include "../shared/Point2D.h"
#include "../shared/Order.h"
#include "../shared/UUO.h"
#include "../shared/MarkerMap.h"

class FollowOrders : public CMOOSApp
{
 public:
   FollowOrders();
   ~FollowOrders();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   void processOrderString(std::string orders);
   void UpdateWaypoints();

private:
   vector<Point2D> allWaypoints;
};

#endif 
