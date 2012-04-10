/************************************************************/
/*    NAME: Isaac Evans                                     */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.h                                   */
/*    DATE: April 5, 2012                                   */
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
#include "../shared/Point2D.h"
#include "../shared/Order.h"
#include "../shared/UUO.h"
#include "../shared/MarkerMap.h"

#define GS_INITIAL 0
#define GS_ALL_LAWNMOW 1
#define GS_LAWNMOW_AND_INSPECT 2

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

   void stateTransition(int newState);
   void sendOrdersTo(vector<string> orders, string to);

   vector<Uuo> getNMostValuableUuos(int n);

private:
   int gameState;
   double startMOOSTime;
};

#endif 
