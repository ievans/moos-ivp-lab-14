#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

class OrderType {
    int type;
    static const int WAYPOINT_ORDER = 0;
    static const int BEHAVIOR_ORDER = 1;
};

class Order {
    OrderType type;
};

class WaypointOrder : Order {
    Point2D waypoint;
    WaypointOrder(Point2D w) { waypoint = w; }
};

class BehaviorOrder : Order {
    string newBehavior;
    BehaviorOrder(string behaviorString) { newBehavior = behaviorString; }
};
