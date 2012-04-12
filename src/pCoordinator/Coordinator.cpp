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
#define MIN(a, b) a < b ? a : b

using namespace std;

//---------------------------------------------------------
// Constructor

Coordinator::Coordinator()
{
    gameState = GS_INITIAL;
    myMap = MarkerMap();
    this->TestAll();
    slavePose = Point2D(0, 0, 5); // see note in .h file
    myPose = Point2D(0, 0, 0); 
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
    } 
    else if ((gameState == GS_ALL_LAWNMOW || gameState == GS_RENDEZVOUS)
	       && newState == GS_LAWNMOW_AND_INSPECT) {
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

	if (myMap._map.size() == 0) {
	    cout << "WARNING: Time to transition to LAWNMOW_AND_INSPECT, but no map data "  << endl;
	    return; // state will be left unchanged
	}

	// go to the most valuable UUOs
	// note: hack, slavePose.id = the sensor radius of the slave vehicle
	priority_queue<PriorityNode> pq = myMap.getPriorityNodes(slavePose, slavePose.id);
	int minePops = MAX_VISIT;
	if (pq.size() < MAX_VISIT)
	    minePops = pq.size();

	for (int i = 0; i < minePops; i++) {
	    PriorityNode mine = pq.top();
	    cout << "popped mine weight " << mine.getWeight() << endl;
	    pq.pop();
	    
            // create an order to go to this mine
	    WaypointOrder wp = WaypointOrder(mine.getPoint());
	    wps.push_back(wp);
	}

	// (TODO) meet back up where the master will be in the *future*
	rendezvous.waypoint = myPose;
	// optimally sort the waypoint orders
	cout << "beginning TSP..." << endl;
	wps = WaypointOrder::optimalPath(wps, slavePose, rendezvous.waypoint);
	cout << "TSP complete." << endl;
	// give slave final waypoint, it's the rendezvous
	wps.push_back(rendezvous);
	// set a time to meet
	rendezvousTime = MOOSTime() + WaypointOrder::getTimeToComplete(wps, VEHICLE_SPEED);

        // convert to a string message
	for (int i = 0; i < wps.size(); i++) {
	    slaveOrders.push_back(wps[i].toString());
	}
	sendOrdersTo(slaveOrders, SLAVE_ORDERS_STRING);
    } // end state transition to LAWNMOW_AND_INSPECT
    else if (newState == GS_RENDEZVOUS) {
	// set master behavior to be the rendezvous waypoint
	BehaviorOrder rv = BehaviorOrder(WAYPOINT_BEHAVIOR_STRING);
	vector<string> orders;
	orders.push_back(rv.toString());
	orders.push_back(rendezvous.toString());
	sendOrdersTo(orders, MASTER_ORDERS_STRING);
	// transition out of rendezvous state happens automatically when
	// a map fuse is completed (ie., we are in comm range of slave)
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

      if (msg.GetKey() == FUSE_COMPLETE_MESSAGE_NAME || msg.GetKey() == "HANDLE_SENSOR_MESSAGE") {
	  string mapString = msg.GetString();
	  cout << " got MAP " << mapString << endl;
	  myMap = MarkerMap(mapString);
	  // if we were planning on having a rendezvous...it just happened
	  this->stateTransition(GS_LAWNMOW_AND_INSPECT);
      } 
      else if (msg.GetKey() == SLAVE_UPDATE_MESSAGE_NAME) {
	  slavePose = Point2D(msg.GetString());
      }
      else if (msg.GetKey() == SLAVE_UPDATE_MESSAGE_NAME) {
	  slavePose = Point2D(msg.GetString());
      }
      else if (msg.GetKey() == "NAV_X") {
          myPose.x = msg.GetDouble();
      } 
      else if (msg.GetKey() == "NAV_Y") {
          myPose.y = msg.GetDouble();
      }
      else if (msg.GetKey() == "SLAVE_X") {
          slavePose.x = msg.GetDouble();
      }
      else if (msg.GetKey() == "SLAVE_Y") {
          slavePose.y = msg.GetDouble();
      }
      else if (msg.GetKey() == "SLAVE_SENSOR_RANGE") {
          slavePose.id = msg.GetDouble();
      }
      else {
	  cout << "unknown message: " << msg.GetKey() << " : " << msg.GetString() << endl;
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
    m_Comms.Register("NAV_X", 0);
    m_Comms.Register("NAV_Y", 0);
    m_Comms.Register("SLAVE_X", 0);
    m_Comms.Register("SLAVE_Y", 0);
    m_Comms.Register("SLAVE_SENSOR_RANGE", 0);
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

    cout << "Testing MarkerMap PriorityQueue...";
    MarkerMap m3 = MarkerMap("id=1,x=-10,y=-88,pH=0.0261963,cc=0,mh=.o:id=10,x=-88,y=-79,pH=0.0261963,cc=0,mh=.o:id=20,x=33,y=-145,pH=0.1,cc=1,mh=.:id=31,x=187,y=-104,pH=0.0261963,cc=0,mh=.o:id=38,x=362,y=-77,pH=0.0261963,cc=0,mh=.o:id=76,x=96,y=-116,pH=0.132275,cc=0,mh=.:id=84,x=-40,y=-81,pH=0.0261963,cc=0,mh=.o:id=95,x=383,y=-87,pH=0.463468,cc=0,mh=.x:id=144,x=338,y=-143,pH=0.0261963,cc=0,mh=.o:id=202,x=143,y=-122,pH=0.132275,cc=1,mh=.:id=204,x=282,y=-104,pH=0.0261963,cc=0,mh=.o:id=232,x=22,y=-86,pH=0.0261963,cc=0,mh=.o:id=245,x=382,y=-132,pH=0.0261963,cc=0,mh=.o:id=292,x=253,y=-148,pH=0.0261963,cc=0,mh=.o:id=322,x=70,y=-98,pH=0.463468,cc=0,mh=.x:id=362,x=287,y=-130,pH=0.132275,cc=1,mh=.:id=398,x=253,y=-76,pH=0.0261963,cc=0,mh=.o:id=424,x=341,y=-91,pH=0.132275,cc=1,mh=.:id=426,x=-51,y=-85,pH=0.132275,cc=1,mh=.:id=429,x=58,y=-140,pH=0.132275,cc=1,mh=.:id=449,x=327,y=-128,pH=0.132275,cc=1,mh=.:id=505,x=391,y=-155,pH=0.132275,cc=1,mh=.:id=510,x=52,y=-121,pH=0.132275,cc=1,mh=.:id=539,x=346,y=-106,pH=0.132275,cc=1,mh=.:id=553,x=212,y=-88,pH=0.132275,cc=1,mh=.:id=558,x=293,y=-139,pH=0.132275,cc=1,mh=.:id=575,x=362,y=-87,pH=0.132275,cc=1,mh=.:id=625,x=183,y=-123,pH=0.132275,cc=1,mh=.:id=626,x=392,y=-96,pH=0.132275,cc=1,mh=.:id=643,x=-131,y=-102,pH=0.0261963,cc=0,mh=.o:id=661,x=227,y=-109,pH=0.132275,cc=1,mh=.:id=672,x=-132,y=-93,pH=0.0261963,cc=0,mh=.o:id=684,x=389,y=-90,pH=0.132275,cc=1,mh=.:id=685,x=59,y=-138,pH=0.132275,cc=1,mh=.:id=727,x=-23,y=-92,pH=0.132275,cc=1,mh=.:id=738,x=-144,y=-84,pH=0.132275,cc=1,mh=.:id=744,x=277,y=-110,pH=0.132275,cc=1,mh=.:id=755,x=280,y=-81,pH=0.132275,cc=1,mh=.:id=767,x=329,y=-80,pH=0.132275,cc=1,mh=.:id=778,x=-135,y=-92,pH=0.0261963,cc=0,mh=.o:id=787,x=301,y=-132,pH=0.132275,cc=1,mh=.:id=794,x=40,y=-84,pH=0.132275,cc=1,mh=.:id=829,x=391,y=-93,pH=0.132275,cc=1,mh=.:id=836,x=302,y=-113,pH=0.132275,cc=1,mh=.:id=847,x=252,y=-90,pH=0.132275,cc=1,mh=.:id=850,x=-108,y=-88,pH=0.132275,cc=1,mh=.:id=918,x=245,y=-146,pH=0.132275,cc=1,mh=.:id=929,x=321,y=-102,pH=0.132275,cc=1,mh=.:id=930,x=92,y=-105,pH=0.132275,cc=1,mh=.:id=968,x=276,y=-75,pH=0.132275,cc=1,mh=.:id=970,x=42,y=-123,pH=0.132275,cc=1,mh=.:id=8888,x=0,y=-20,pH=0.0261963,cc=0,mh=.o");
    priority_queue<PriorityNode> pq = m3.getPriorityNodes(slavePose, slavePose.id);
    cout << "pq.size(): " << pq.size() << endl;
    PriorityNode pn1 = pq.top();
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
