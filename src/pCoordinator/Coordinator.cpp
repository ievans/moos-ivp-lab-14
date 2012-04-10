/************************************************************/
/*    NAME: Isaac Evans                                     */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.cpp                                 */
/*    DATE: April 5, 2012                                   */
/************************************************************/


#include <iterator>
#include "Coordinator.h"

#define FUSE_COMPLETE_MESSAGE_NAME "FUSE_COMPLETE"
#define LAWNMOW_BEHAVIOR_STRING "lawmow"
#define WAYPOINT_BEHVIOR_STRING "waypoint"
#define MASTER_ORDERS_STRING "READ_MASTER_ORDERS"
#define SLAVE_ORDERS_STRING "READ_ORDERS"

using namespace std;

//---------------------------------------------------------
// Constructor

Coordinator::Coordinator()
{
    gameState = GS_INITIAL;
    myMap = MarkerMap();
}

//---------------------------------------------------------
// Destructor

Coordinator::~Coordinator()
{
}

vector<Uuo> getNMostValuableUuos(int n) {
    vector<Uuo> topN;
    return topN;
}

void Coordinator::stateTransition(int newState) {
    cout << "moving to " << newState << endl;

    if (gameState == GS_INITIAL && newState == GS_ALL_LAWNMOW) {
	BehaviorOrder lawnmow = BehaviorOrder(LAWNMOW_BEHAVIOR_STRING);
	vector<string> orders;
	orders.push_back(lawnmow.toString());
	sendOrdersTo(orders, MASTER_ORDERS_STRING);
    } else if (gameState == GS_ALL_LAWNMOW && newState == GS_LAWNMOW_AND_INSPECT) {
	// keep the master in lawmow mode
	BehaviorOrder lawnmow = BehaviorOrder(LAWNMOW_BEHAVIOR_STRING);
	vector<string> orders;
	orders.push_back(lawnmow.toString());
	sendOrdersTo(orders, MASTER_ORDERS_STRING);

	vector<string> slaveOrders;
	BehaviorOrder wpb = BehaviorOrder(WAYPOINT_BEHVIOR_STRING);
	slaveOrders.push_back(wpb.toString());

	// generate waypoint behaviors for the slave
	WaypointOrder wp = WaypointOrder(Point2D(0, 0));
	slaveOrders.push_back(wp.toString());

	sendOrdersTo(slaveOrders, SLAVE_ORDERS_STRING);
    }
    gameState = newState;
}

void Coordinator::sendOrdersTo(vector<string> orders, string target) {
    for (int i = 0; i < orders.size(); i++) {
	cout << "sent orders to " << target << " , orders: " <<  orders[i] << endl;
	m_Comms.Notify(target, orders[i]);
    }
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Coordinator::OnNewMail(MOOSMSG_LIST &NewMail)
{
   MOOSMSG_LIST::iterator p;
   
   for(p=NewMail.begin(); p!=NewMail.end(); p++) {
      CMOOSMsg &msg = *p;

      if (msg.GetKey() == FUSE_COMPLETE_MESSAGE_NAME) {
	  string mapString = msg.GetString();
	  cout << " got MAP " << mapString << endl;
	  myMap = MarkerMap(mapString);
      }
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Coordinator::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);
	
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool Coordinator::Iterate()
{
   // happens AppTick times per second

   // check the marker map. how many blocks have been found? 
   // should we change out of lawnmow behavior to the waypoint 
    // search + rendezvous state?
    if (MOOSTime() - startMOOSTime > 0) {
	if (gameState == GS_INITIAL) {
	    cout << "making state transition to all lawnmow" << endl;
	    this->stateTransition(GS_ALL_LAWNMOW);
	}
    }

    if (MOOSTime() - startMOOSTime > 500) {
	if (gameState == GS_ALL_LAWNMOW) {
	    cout << "making state transition to lawnmow and inspect" << endl;
	    this->stateTransition(GS_LAWNMOW_AND_INSPECT);
	}
    }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool Coordinator::OnStartUp()
{
   // happens before connection is open. important: CANNOT work in constructor
   startMOOSTime = MOOSTime();

   RegisterVariables();	
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Coordinator::RegisterVariables()
{
    m_Comms.Register(FUSE_COMPLETE_MESSAGE_NAME, 0);
}

