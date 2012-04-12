/************************************************************/
/*    NAME: Isaac Evans                                     */
/*    ORGN: MIT                                             */
/*    FILE: Coordinator.cpp                                 */
/*    DATE: April 5, 2012                                   */
/************************************************************/


#include <iterator>
#include "Coordinator.h"

#include <assert.h>

#define FUSE_COMPLETE_MESSAGE_NAME "FUSE_COMPLETE"
#define LAWNMOW_BEHAVIOR_STRING "lawmow"
#define WAYPOINT_BEHAVIOR_STRING "waypoint"
#define MASTER_ORDERS_STRING "READ_MASTER_ORDERS"
#define SLAVE_ORDERS_STRING "READ_ORDERS"
#define SLAVE_UPDATE_MESSAGE_NAME "SLAVE_POSE"
#define VEHICLE_SPEED 2 // xy units per second = meters / sec
#define MAX_VISIT 8 // max number of nodes a slave will visit before rendesvous
		    // note that these are these are supernodes (see the
		    // priority queue method in MarkerMap.h)

using namespace std;

//---------------------------------------------------------
// Constructor

Coordinator::Coordinator()
{
    gameState = GS_INITIAL;
    myMap = MarkerMap();
    this->TestAll();
    slavePose = Point2D(0, 0, 5); // see note in .h file
    myPose = Point2D(0, 0, 0); // TODO actually update
    rendezvousTime = -1;
    rendezvous = WaypointOrder();
}

//---------------------------------------------------------
// Destructor

Coordinator::~Coordinator()
{
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
	BehaviorOrder wpb = BehaviorOrder(WAYPOINT_BEHAVIOR_STRING);
	slaveOrders.push_back(wpb.toString());

	// generate waypoint orders for the slave
	vector<WaypointOrder> wps;

	// go to the most valuable UUOs
	// note: hack, slavePose.id = the sensor radius of the slave vehicle
	priority_queue<PriorityNode> pq = myMap.getPriorityNodes(slavePose, slavePose.id);
#define MIN(a, b) a < b ? a : b
	for (int i = 0; i < MIN(MAX_VISIT, pq.size()); i++) {
	    PriorityNode mine = pq.top();
	    pq.pop();
	    
            // create an order to go to this mine
	    WaypointOrder wp = WaypointOrder(mine.getPoint());
	    wps.push_back(wp);
	}

	// (TODO) meet back up where the master will be in the future 
	rendezvous.waypoint = myPose;
	// optimally sort the waypoint orders
	wps = WaypointOrder::optimalPath(wps, slavePose, rendezvous.waypoint);
	// give slave final waypoint, it's the rendezvous
	wps.push_back(rendezvous);
	// set a time to meet
	rendezvousTime = MOOSTime() + WaypointOrder::getTimeToComplete(wps, VEHICLE_SPEED);

        // convert to a string message
	for (int i = 0; i < wps.size(); i++) {
	    slaveOrders.push_back(wps[i].toString());
	}
	sendOrdersTo(slaveOrders, SLAVE_ORDERS_STRING);
    // end state transition to LAWNMOW_AND_INSPECT
    } else if (newState == GS_RENDEZVOUS) {
	// set master behavior to be the rendezvous waypoint
	BehaviorOrder rv = BehaviorOrder(WAYPOINT_BEHAVIOR_STRING);
	vector<string> orders;
	orders.push_back(rv.toString());
	orders.push_back(rendezvous.toString());
	sendOrdersTo(orders, MASTER_ORDERS_STRING);
	// TODO: need transition out of rendezvous state
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
      if (msg.GetKey() == SLAVE_UPDATE_MESSAGE_NAME) {
	  slavePose = Point2D(msg.GetString());
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

    if (MOOSTime() - rendezvousTime > 0 && rendezvousTime != -1) {
	cout << "making state transition to rendezvous" << endl;
	this->stateTransition(GS_RENDEZVOUS);
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

void Coordinator::TestAll() {
    cout << "Testing Point2D...";
    Point2D p = Point2D(03235, 23281, 2372);
    Point2D q = Point2D(p.toString());
    assert(p.x == q.x && p.y == q.y && p.id == q.id);
    q = Point2D(03235, 23281, "2372");
    assert(p.x == q.x && p.y == q.y && p.id == q.id);
    cout << "ok" << endl;

//    cout << "Testing UUO" << endl;
//    Uuo u = new Uuo();

    cout << "Testing BehaviorOrder...";
    BehaviorOrder bo = BehaviorOrder("foobar");
    BehaviorOrder bo2 = BehaviorOrder("nadda");
    bo2.fromString(bo.toString());
    assert(bo2.toString() == bo.toString());
    cout << "ok" << endl;

    cout << "Testing WaypointOrder...";
    WaypointOrder wo = WaypointOrder(Point2D(32, 23, 101));
    WaypointOrder wo2 = WaypointOrder(Point2D(-1, -1, -1));
    wo2.fromString(wo.toString());
    assert(wo.toString() == wo2.toString());
    cout << "ok" << endl;

    cout << "Testing MarkerMap...";
    MarkerMap m = MarkerMap();
    //assert(m.toString() == "");
    Uuo a = Uuo(3, 38.83, 1, 0.238);
    Uuo b = Uuo(2, -28.28, 328.2, 0.0001);
    Uuo c = Uuo(-13823, 0, 0, 0.00002);
    Uuo d = Uuo(23811, 32.282, 382, 32.0);
    m._map.insert(pair<int,Uuo>(3, a));
    m._map.insert(pair<int,Uuo>(2, b));
    m._map.insert(pair<int,Uuo>(-13823, c));
    m._map.insert(pair<int,Uuo>(3282, d));
    m._map.insert(pair<int,Uuo>(0, Uuo(328, 23, 2383, 1.0)));
    MarkerMap m2 = MarkerMap(m.toString());
//    cout << m.toString() << endl;
//    cout << "A: " << m2._map[3].toString() << endl; cout << "B: " << a.toString() << endl;
    assert(m2._map[3].toString() == a.toString());
    assert(m2._map[2].toString() == b.toString());
    assert(m2._map[-13823].toString() == c.toString());

    // should !=, because ID of Uuo didn't match inserted ID
    assert(m2._map[3282].toString() != d.toString());
    cout << "ok" << endl;

    cout << "Testing TSP...";
    vector<WaypointOrder> wps;
    wps.push_back(WaypointOrder(Point2D(10, 10)));
    wps.push_back(WaypointOrder(Point2D(9, 9)));
    wps.push_back(WaypointOrder(Point2D(6, 7)));
    wps.push_back(WaypointOrder(Point2D(6, 6)));
    wps.push_back(WaypointOrder(Point2D(3, 63)));
    wps.push_back(WaypointOrder(Point2D(163, 163)));
    wps.push_back(WaypointOrder(Point2D(63, 363)));
    wps.push_back(WaypointOrder(Point2D(663, 663)));

//    wps.push_back(WaypointOrder(Point2D(1, 1)));
//    wps.push_back(WaypointOrder(Point2D(6, 99)));
//    wps.push_back(WaypointOrder(Point2D(1337, 63)));
//    wps.push_back(WaypointOrder(Point2D(163, 163)));

    wps = WaypointOrder::optimalPath(wps, Point2D(0, 0), Point2D(30, 30));
    cout << "returned min path cost " << WaypointOrder::pathLength(wps, Point2D(0, 0), Point2D(30, 30)) << endl;
    for (int i = 0; i < wps.size(); i++) {
	cout << wps[i].toString() << endl;
    }
    cout << "ok" << endl;
}
