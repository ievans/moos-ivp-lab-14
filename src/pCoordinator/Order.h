class Point2D {
public:
    Point2D() {};
    Point2D(double px, double py) { x = px; y = py; };
    Point2D(double px, double py, int pid) { x = px; y = py; id = pid; };
    Point2D(double px, double py, string pid) { x = px; y = py; id = atoi(pid); };
    double x;
    double y;
    int id;
};

class OrderType {
    int type;
    const int WAYPOINT_ORDER = 0;
    const int BEHAVIOR_ORDER = 1;
}

class Order {
    OrderType type;
 }

class WaypointOrder : Order {
    Point2D waypoint;
    WaypointOrder(Point2D w) { waypoint = w; }
}

class BehaviorOrder : Order {
    string newBehavior;
    BehaviorOrder(string behaviorString) { newBehavior = behaviorString; }
}
