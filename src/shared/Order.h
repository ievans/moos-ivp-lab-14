#ifndef ORDER_HEADER
#define ORDER_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

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
    WaypointOrder(Point2D w) { type = OrderType::WAYPOINT_ORDER; waypoint = w; };

    string toString() { 
	return Order::toTypeString() + SEPARATOR + waypoint.toString();
    };

    void fromString(string s) {
	vector<string> svector = parseString(s, SEPARATOR);
	waypoint.fromString(svector[1]);
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
