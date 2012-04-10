/************************************************************/
/*    NAME: Isaac Evans, Robert Truax                        */
/*    ORGN: MIT                                             */
/*    FILE: MarkerMap.h                                     */
/*    DATE: April 5, 2012                                   */
/************************************************************/
#ifndef MARKERMAP_HEADER
#define MARKERMAP_HEADER

#include "UUO.h"
#include <map>

class MarkerMap {
public:
  std::map<int, Uuo> _mines;
  std::map<int, Uuo> _mines_other;

  MarkerMap() {
  };

  MarkerMap(string encodedMM) { this->fromString(encodedMM); };

  

  string toString() {
    // TODO:  Compress later
    // Message format:
    // : separated
    // id,xpos,ypos,pH,cc

    stringstream msg;
    map<int, Uuo>::iterator it;

    if (_mines.begin() == _mines.end()) {
      msg << "NONE";
    }
    else {
      it = _mines.begin();
      msg << "id=" << it->second.id << "," 
	  << "x=" << it->second.x << "," 
	  << "y=" << it->second.y << ","
	  << "pH=" << it->second.probHazard << "," 
	  << "cc=" << it->second.classifyCount;

      for (it++; it != _mines.end(); it++) {
	msg << ":" << "id=" << it->second.id << "," 
	    << "x=" << it->second.x << "," 
	    << "y=" << it->second.y << ","
	    << "pH=" << it->second.probHazard << "," 
	    << "cc=" << it->second.classifyCount;
      }
    }

    return msg.str();
  };
   
  void fromStringToOther(string encodedMM) {
    Uuo mine;
    map<int,Uuo>::iterator it;
    vector<string> strvect = parseString(encodedMM,":");
    for (int i = 0; i < strvect.size(); i++) {

      Uuo newMine;
      vector<string> in = parseString(strvect[i],",");
      for (int j = 0; j < in.size(); j++) {
	vector<string> var = parseString(in[j],"=");

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
      }

      if (newMine.isInitialized()) {
	// integrate new mine
	it = _mines_other.find(newMine.id);
	if (it == _mines_other.end()) {
	  _mines_other.insert( pair<int,Uuo>(newMine.id,newMine) );
	}
	else {
	  it->second = newMine;
	}
      }
      else {
	cout << "Invalid mine message received:" << endl;
	cout << encodedMM << endl;
      }
    }
  };

  void fromString(string encodedMM) {
    Uuo mine;
    map<int,Uuo>::iterator it;
    vector<string> strvect = parseString(encodedMM,":");
    for (int i = 0; i < strvect.size(); i++) {

      Uuo newMine;
      vector<string> in = parseString(strvect[i],",");
      for (int j = 0; j < in.size(); j++) {
	vector<string> var = parseString(in[j],"=");

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
      }

      if (newMine.isInitialized()) {
	// integrate new mine
	it = _mines.find(newMine.id);
	if (it == _mines.end()) {
	  _mines.insert( pair<int,Uuo>(newMine.id,newMine) );
	}
	else {
	  it->second = newMine;
	}
      }
      else {
	cout << "Invalid mine message received:" << endl;
	cout << encodedMM << endl;
      }
    }
  };

  static double getPriority(Uuo& mine) {
      // Assume triangle priority.  zero at hazProb = {0,1} and
      // one at hazProb = {PRIOR_PROB}
      // Biases towards points nearest default
      // However, also bias toward low label numbers
      double priority;
      if (mine.probHazard < PRIOR_PROB) {
	  priority = 1.0/PRIOR_PROB * mine.probHazard;
      }
      else {
	  // point slope formulation
	  priority = 1 + -1.0/(1-PRIOR_PROB) * (mine.probHazard - PRIOR_PROB);
      }

      return priority;
  };

  double getPriorityMineIndex() {
      //    cout << "Trying to Classify Something" << endl;
      // find highest priority point
      // TODO: What is highest priority???
      map<int, Uuo>::iterator it;
      double best_idx = -1;
      double best_priority = -1;

      // TODO: Linear search sux
      for (it = _mines.begin(); it != _mines.end(); it++) {
	  if (it->second.classifyCount > 0) {
	      if (MarkerMap::getPriority(it->second) > best_priority) {
		  best_idx = it->second.id;
		  best_priority = MarkerMap::getPriority(it->second);
	      }
	  }
      }
      return best_idx;
  }

}; // end class definition

#endif
