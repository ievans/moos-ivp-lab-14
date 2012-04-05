#ifndef ORDER_HEADER
#define ORDER_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

class OrderType {
public:
    int type;
    static const int WAYPOINT_ORDER = 0;
    static const int BEHAVIOR_ORDER = 1;
};

class Order {
public:
    OrderType type;
    virtual string toString();
};

class WaypointOrder : public Order {
public:
    Point2D waypoint;
    WaypointOrder(Point2D w) { waypoint = w; }
};

class BehaviorOrder : public Order {
public:
    string newBehavior;
    BehaviorOrder(string behaviorString) { newBehavior = behaviorString; }
};

#endif
