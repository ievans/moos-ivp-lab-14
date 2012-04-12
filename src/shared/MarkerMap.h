/************************************************************/
/*    NAME: Isaac Evans, Robert Truax                        */
/*    ORGN: MIT                                             */
/*    FILE: MarkerMap.h                                     */
/*    DATE: April 5, 2012                                   */
/************************************************************/
#ifndef MARKERMAP_HEADER
#define MARKERMAP_HEADER

#include "UUO.h"
#include "PriorityNode.h"
#include <map>
#include <queue>

class MarkerMap {
public:
  // node id, node object
    std::map<int, Uuo> _map;

    MarkerMap() {
    };

    MarkerMap(string encodedMM) { this->fromString(encodedMM); };

    string toString() {
	// TODO:  Compress later
	// Message format:
	// : separated
	// id,xpos,ypos,pH,cc,mh

	stringstream msg;
	map<int, Uuo>::iterator it;

	if (_map.begin() == _map.end()) {
	    msg << "NONE";
	}
	else {
	    it = _map.begin();
	    msg << "id=" << it->second.id << "," 
		<< "x=" << it->second.x << "," 
		<< "y=" << it->second.y << ","
		<< "pH=" << it->second.probHazard << "," 
		<< "cc=" << it->second.classifyCount << ","
		<< "mh=" << it->second.m_hist;

	    for (it++; it != _map.end(); it++) {
		msg << ":" << "id=" << it->second.id << "," 
		    << "x=" << it->second.x << "," 
		    << "y=" << it->second.y << ","
		    << "pH=" << it->second.probHazard << "," 
		    << "cc=" << it->second.classifyCount << ","
		    << "mh=" << it->second.m_hist;
	    }
	}

	return msg.str();
    };
  
    void fromString(string encodedMM) {
      // WARNING!  THIS WILL OVERWRITE EXISTING MAP!
      _map.clear();
      if (encodedMM == "NONE") {
	return;
      }

      vector<string> strvect = parseString(encodedMM,":");
      for (int i = 0; i < strvect.size(); i++) {

	    Uuo newMine;
	    vector<string> in = parseString(strvect[i],",");
	    for (int j = 0; j < in.size(); j++) {
		vector<string> var = parseString(in[j],"=");

		if (var.size() != 2) { // crucial, prevents segfaults
		    continue;
		}
		if (var[0] == "id") { 
		    newMine.id = atoi(var[1].c_str());
		}
		else if(var[0] == "x") {
		    newMine.x = atof(var[1].c_str());
		}
		else if(var[0] == "y") {
		    newMine.y = atof(var[1].c_str());
		}
		else if(var[0] == "pH") {
		    newMine.probHazard = atof(var[1].c_str());
		}
		else if(var[0] == "cc") {
		    newMine.classifyCount = atoi(var[1].c_str());
		}
		else if (var[0] == "mh") {
		  newMine.m_hist = var[1];
		}
	    }

	    if (newMine.isInitialized()) {
	      _map.insert( pair<int,Uuo>(newMine.id,newMine) );
	    }
	    else {
		cout << "Invalid mine message received:" << endl;
		cout << encodedMM << endl;
		return;
	    }
	}
    };

    static double getPriority(Uuo& mine) {
	// Assume triangle priority.  zero at hazProb = {0,1} and
	// one at hazProb = PRIORITY_MAX
	// Biases towards points nearest uncertainty
	// However, also bias toward low label numbers
	double priority;
	if (mine.probHazard < PRIORITY_MAX) {
	    priority = 1.0/PRIORITY_MAX * mine.probHazard;
	}
	else {
	    // point slope formulation
	    priority = 1 + -1.0/(1-PRIORITY_MAX) * (mine.probHazard - PRIORITY_MAX);
	}

	return priority;
    };

    double getPriorityMineIndex() {
	// find highest priority point
	// TODO: What is highest priority???
	map<int, Uuo>::iterator it;
	double best_idx = -1;
	double best_priority = -1;

	// TODO: Linear search sux
	for (it = _map.begin(); it != _map.end(); it++) {
	    if (it->second.classifyCount > 0) {
		if (MarkerMap::getPriority(it->second) > best_priority) {
		    best_idx = it->second.id;
		    best_priority = MarkerMap::getPriority(it->second);
		}
	    }
	}
	return best_idx;
    };

    /*
     * Given a current position and the radius of a sensor that can detect
     * mines within SENSOR_RADIUS of that position, returns an priority queue
     * of the top priority nodes to visit. This list should then be run
     * through a path planning algorithm (such as a TSP approximation).
     */
    priority_queue<PriorityNode> getPriorityNodes(Point2D currentPos, double SENSOR_RADIUS) {
	priority_queue<PriorityNode> r;
	map<int, Uuo>::iterator it;

	// O(n^2), easily improvable for later (TODO)
	for (it = _map.begin(); it != _map.end(); it++) {
	    if (it->second.classifyCount > 0) {

		// for each point, examine all neighbors
		map<int, Uuo>::iterator neighbor_it;
		double utilityAccumulator = 0;
		Point2D this_point = it->second.getPoint();
		for (neighbor_it = _map.begin(); neighbor_it != _map.end(); neighbor_it++) {
		    // if the neighbor is within sensor distance, add its priority to this point
		    if (neighbor_it->second.getPoint().dist(this_point) <= SENSOR_RADIUS) {
			utilityAccumulator += MarkerMap::getPriority(neighbor_it->second);
		    }
		}
		
		// add the point to a sorted list with a value equal to the 
                // accumulated utility divided by the sqrt of the distance from us
#define EXPONENTIAL_DISTANCE_PENALTY 0.5
		r.push(PriorityNode(utilityAccumulator / 
				    pow(currentPos.dist(this_point), EXPONENTIAL_DISTANCE_PENALTY), 
				    this_point));
	    }
	}
    };

}; // end class definition

#endif
