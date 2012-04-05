#ifndef UUO_HEADER
#define UUO_HEADER

#include <queue>

using namespace std;

class Uuo { // unidentified underwater vehicle
public:
    double x;
    double y;
    int id;
    double probHazard;
    int classifyCount; // remaining times to classify object
    // Todo: raw sightings from vehicles

    Uuo () {
	// Requires initialization
	x = -1000;
	y = -1000;
	id = -1;

	// default initialization
	probHazard = 0.1; // Maximum entropy?  Better prior?
	classifyCount = 0;
    }

    bool isHazard() {
	// Todo: Be smarter!
	if (probHazard > 0.75) {
	    return true;
	}
	else {
	    return false;
	}
    }

    bool isInitialized() {
	if (x == -1000 || y == -1000 || id == -1) {
	    return false;
	}
	else {
	    return true;
	}
    }

    bool operator <(const Uuo& u) const {
      // priority queue comparison
      if (u.classifyCount < 1) {
	return true;
      }
      else {
	return false;
      }
    }

    bool operator() (Uuo& u1, Uuo& u2) {
      if (u1.classifyCount < 1) {
	return true;
      }
      else {
	return false;
      }
    }
};

#endif
