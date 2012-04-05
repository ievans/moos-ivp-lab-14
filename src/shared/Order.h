#ifndef ORDER_HEADER
#define ORDER_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

class OrderType {
public:
    static const int WAYPOINT_ORDER = 0;
    static const int BEHAVIOR_ORDER = 1;
};

class Order {
public:
    int type;
    string toString() {
	return "jk";
    }
};

class WaypointOrder : public Order {
public:
    Point2D waypoint;
    WaypointOrder(Point2D w) { type = OrderType::WAYPOINT_ORDER; waypoint = w; };
    string toString() { return ""; }
};

class BehaviorOrder : public Order {
public:
    string newBehavior;
    BehaviorOrder(string behaviorString) { type = OrderType::BEHAVIOR_ORDER; newBehavior = behaviorString; }
    string toString() { return ""; }
};

#endif
