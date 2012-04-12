/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: FollowOrders.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "FollowOrders.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FollowOrders::FollowOrders()
{
}

//---------------------------------------------------------
// Destructor

FollowOrders::~FollowOrders()
{
}

#define MASTER_ORDERS_STRING "READ_MASTER_ORDERS"
#define SLAVE_ORDERS_STRING "READ_ORDERS"
#define WAYPOINTS_UPDATE_NAME "UP_WPT_PTS"
#define BEHAVIOR_UPDATE_NAME "COORDMODE"
//---------------------------------------------------------
// Procedure: OnNewMail

void FollowOrders::UpdateWaypoints() {
    XYSegList my_seglist;
    for (int i = 0; i < allWaypoints.size(); i++) {
	my_seglist.add_vertex((double)allWaypoints[i].x, (double)allWaypoints[i].y);
    }
    string updates = "points = ";
    updates += my_seglist.get_spec();
    cout << "sent: " << updates << " from " << this->GetAppName() << endl;
    m_Comms.Notify(WAYPOINTS_UPDATE_NAME, updates);
}

void FollowOrders::processOrderString(string orderS) { 
    OrderType ot = Order::getType(orderS);
    switch(ot.orderType) {
    case OrderType::WAYPOINT_ORDER: {
	WaypointOrder wpo = WaypointOrder(orderS);
	// if we are in waypoint behavior, add this to our list of waypoints
	allWaypoints.push_back(wpo.waypoint);
	FollowOrders::UpdateWaypoints();
	break;
    }
    case OrderType::BEHAVIOR_ORDER: {
	BehaviorOrder bo = BehaviorOrder(orderS);
	// set our behavior to be as follows
	m_Comms.Notify(BEHAVIOR_UPDATE_NAME, bo.newBehavior);
	break;
    }
    default: {
	cout << "Unrecognized order type! int type: " << ot.orderType << endl;
    }
    }

}

bool FollowOrders::OnNewMail(MOOSMSG_LIST &NewMail)
{
   MOOSMSG_LIST::iterator p;
   
   for(p=NewMail.begin(); p!=NewMail.end(); p++) {
      CMOOSMsg &msg = *p;

      if (msg.GetKey() == SLAVE_ORDERS_STRING || msg.GetKey() == MASTER_ORDERS_STRING) {
	  string order = msg.GetString();
	  this->processOrderString(order);
      }
      else if (msg.GetKey() == "NAV_X") {
          m_Comms.Notify("SLAVE_X", msg.GetDouble());
      }
      else if (msg.GetKey() == "NAV_Y") {
          m_Comms.Notify("SLAVE_Y", msg.GetDouble());
      }

   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool FollowOrders::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);
    m_Comms.Notify("SLAVE_SENSOR_RANGE", 25); // TODO PUT ACTUAL SENSOR RANGE

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool FollowOrders::Iterate()
{
   // happens AppTick times per second
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool FollowOrders::OnStartUp()
{
   // happens before connection is open

   RegisterVariables();	
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void FollowOrders::RegisterVariables()
{
    if (GetAppName() == "slave")
	m_Comms.Register(SLAVE_ORDERS_STRING, 0);
    else
	m_Comms.Register(MASTER_ORDERS_STRING, 0);

    m_Comms.Register("NAV_X", 0);
    m_Comms.Register("NAV_Y", 0);

}

