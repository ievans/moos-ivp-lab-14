#include "MOOSLib.h"
#include "MBUtils.h"
#include <iterator>
#include <iostream>
#include <string>

using namespace std;

class Point2D {
public:
    double x;
    double y;
    int id;

    Point2D() {};
    Point2D(double px, double py) { x = px; y = py; };
    Point2D(double px, double py, int pid) { x = px; y = py; id = pid; };
    Point2D(double px, double py, string pid) { x = px; y = py; id = atoi(pid.c_str()); };
    Point2D(string encodedPoint) { this->fromString(encodedPoint); }

    string toString() { 
	stringstream s;
	s << this->id << "," << this->x << "," << this->y << endl;
	return s.str();
    }

    void fromString(string encodedPoint) {
	vector<string> svector = parseString(encodedPoint, ',');
	id = atoi(svector[0].c_str());
	x  = atof(svector[1].c_str());
	y  = atof(svector[2].c_str());
    }
};
