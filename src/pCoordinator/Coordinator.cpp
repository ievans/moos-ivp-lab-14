/************************************************************/
/*    NAME: Isaac Evans                                     */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.cpp                                 */
/*    DATE: April 5, 2012                                   */
/************************************************************/

#define FUSE_COMPLETE_MESSAGE_NAME "FUSE_COMPLETE"

#include <iterator>
#include "Coordinator.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Coordinator::Coordinator()
{
    gameState = INITIAL;
}

//---------------------------------------------------------
// Destructor

Coordinator::~Coordinator()
{
}

vector<Uuo> getNMostValuableUuos(int n) {
    return null; // TODO
}

void Coordinator::stateTransition(GameState newState) {
    if (gameState == INITIAL && newState == ALL_LAWNMOW) {
	BehaviorOrder lawnmow = BehaviorOrder("lawnmower");
	sendOrdersToMaster(lawnmow);
	sendOrdersToSlaves(lawnmow);
    } else if (gameState == ALL_LAWNMOW && newState == LAWNMOW_AND_INSPECT) {
	// keep the master in lawmow mode
	BehaviorOrder lawnmow = BehaviorOrder("lawnmower");
	sendOrdersToMaster(lawnmow);
	// generate waypoint behaviors for the slave
	WaypointOrder wp = WaypointOrder();
	
	sendOrdersToSlaves(wp);
    }

    gameState == newState;
}

void Coordinator::sendOrdersToMaster(vector<Order> orders) {
    m_Comms.Notify("MASTER_ORDER", orders.toString());
}
void Coordinator::sendOrdersToSlaves(vector<Order> orders) {
    m_Comms.Notify("SLAVES_ORDER", orders.toString());
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Coordinator::OnNewMail(MOOSMSG_LIST &NewMail)
{
   MOOSMSG_LIST::iterator p;
   
   for(p=NewMail.begin(); p!=NewMail.end(); p++) {
      CMOOSMsg &msg = *p;

      if (msg.GetKey() == FUSE_COMPLETE_MESSAGE_NAME) {
	  string map = msg.GetString();
	  this->stateTransition(ALL_LAWNMOW);
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
    if (MOOSTime() > 500) {
	this->stateTransition(LAWNMOW_AND_INSPECT);
    }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool Coordinator::OnStartUp()
{
   // happens before connection is open

   RegisterVariables();	
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Coordinator::RegisterVariables()
{
    m_Comms.Register(FUSE_COMPLETE_MESSAGE_NAME, 0);
}

