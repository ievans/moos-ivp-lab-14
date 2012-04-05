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
