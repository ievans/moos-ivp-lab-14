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

    void fuseFromString(string encodedMM) {
// TODO
    };

};

#endif
