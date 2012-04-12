#ifndef ORDER_HEADER
#define ORDER_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>
#include <algorithm>

#define SEPARATOR ":"

class OrderType {
public:
    static const int WAYPOINT_ORDER = 0;
    static const int BEHAVIOR_ORDER = 1;
    int orderType;

    OrderType(int orderInt) {
	orderType = orderInt;
    };

};

class Order {
public:
    int type;
    string toTypeString() {
	stringstream s;
	s << type;
	return s.str();
    }

    static OrderType getType(string order) {
	vector<string> svector = parseString(order, SEPARATOR);
	return (OrderType)atoi(svector[0].c_str());
    }
};

class WaypointOrder : public Order {
public:
    Point2D waypoint;
    WaypointOrder() {};
    WaypointOrder(Point2D w) { type = OrderType::WAYPOINT_ORDER; waypoint = w; };

    string toString() { 
	return Order::toTypeString() + SEPARATOR + waypoint.toString();
    };

    void fromString(string s) {
	vector<string> svector = parseString(s, SEPARATOR);
	waypoint.fromString(svector[1]);
    };

    /*
     * O(N!) running time. Returns TSP solution.
     */
    static vector<WaypointOrder> optimalPath(vector<WaypointOrder> wps, Point2D initialPoint, Point2D finalPoint) {
	// exact TSP

        // next_permutation stops when we have reached the last lexicographical
	// ordering. To see all permutations, we must sort lexicographically
	// ahead of time.
	sort(wps.begin(), wps.end());

	double minP = -1;
	vector<WaypointOrder> minPath = wps;
	double P = 0;
	int c = 0;
	do {
	    c++;
	    P = WaypointOrder::pathLength(wps, initialPoint, finalPoint);
/*
	    cout << "P : " << P << endl;
	    for (int i = 0; i < wps.size(); i++) {
		cout << "[" << i << "] : " << wps[i].toString() << endl;
	    }
*/

	    if (P < minP || minP == -1) {
		minP = P;
		minPath = wps;
	    }
	} while (next_permutation(wps.begin(), wps.end()));
	cout << "TSP iterations: " << c;
	return minPath;
    };

    const bool operator <(const WaypointOrder& wpo) const {
	return waypoint < wpo.waypoint;
    };

    /*
     * assumes the inputted waypoints are already in the exact order we're
     * planning to use.
     * TODO: compensate for min turning radius
     */ 
    static double pathLength(vector <WaypointOrder>& wps) {
	double distAcc = 0;
	for (int i = 1; i < wps.size(); i++) {
	    distAcc += wps[i].waypoint.dist(wps[i -1].waypoint);
	}
	return distAcc;
    };

    static double pathLength(vector <WaypointOrder>& wps, Point2D initialPoint, Point2D finalPoint) {
	double distAcc = 0;
	for (int i = 0; i <= wps.size(); i++) {
	    if (i == 0) {
		distAcc += wps[i].waypoint.dist(initialPoint);
	    } 
	    else if (i == wps.size()) {
		distAcc += wps[i - 1].waypoint.dist(finalPoint);
	    }
	    else {
		distAcc += wps[i].waypoint.dist(wps[i -1].waypoint);
	    } 
	}
	return distAcc;
    };

    static double getTimeToComplete(vector<WaypointOrder> wps, double VEHICLE_SPEED_IN_UNITS_PER_SEC) {
	return WaypointOrder::pathLength(wps) / VEHICLE_SPEED_IN_UNITS_PER_SEC;
    };

};

class BehaviorOrder : public Order {
public:
    string newBehavior;
    BehaviorOrder(string behaviorString) { type = OrderType::BEHAVIOR_ORDER; newBehavior = behaviorString; }

    string toString() {
	return Order::toTypeString() + SEPARATOR + newBehavior;
    };

    void fromString(string s) {
	vector<string> svector = parseString(s, SEPARATOR);
	newBehavior = svector[1];
    }

};

#endif
