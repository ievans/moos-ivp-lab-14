/************************************************************/
/*    NAME: robtruax                                              */
/*    ORGN: MIT                                             */
/*    FILE: HandleSensorData.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

//#include <iterator>
#include "HandleSensorData.h"

using namespace std;

//---------------------------------------------------------
// Constructor

HandleSensorData::HandleSensorData()
{
}

//---------------------------------------------------------
// Destructor

HandleSensorData::~HandleSensorData()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool HandleSensorData::OnNewMail(MOOSMSG_LIST &NewMail)
{

  //  cout << "OnNewMail start" << endl;

   MOOSMSG_LIST::iterator p;
   
   for(p=NewMail.begin(); p!=NewMail.end(); p++) {
     CMOOSMsg &msg = *p;

     string key = msg.GetKey();

     //     cout << "Got Mail : " << msg.GetKey() << endl;

     // This is a classification report
     if (key == "UHZ_HAZARD_REPORT") {
       // Parse String
       string str = msg.GetString();
       vector<string> strv = parseString(str,",");

       double x,y;
       bool hazard;
       int label, paramcount = 0;

       for (int i = 0; i < strv.size(); i++) {
	 vector<string> input = parseString(strv[i],"=");

	 if (input[0] == "x") {
	   x = atof(input[1].c_str());
	   paramcount++;
	 }
	 else if (input[0] == "y") {
	   y = atof(input[1].c_str());
	   paramcount++;
	 }
	 else if (input[0] == "type") {
	   if (input[1] == "hazard") {
	     hazard = true;
	     paramcount++;
	   }
	   else if (input[1] == "benign") {
	     hazard = false;
	     paramcount++;
	   }
	 }
	 else if (input[0] == "label") {
	   label = atoi(input[1].c_str());
	   cout << "set label to " << label << endl;
	   paramcount++;
	 }
       }

       // check if we got everything necessary
       if (paramcount == 4) {
	 // We will always have this in our database already for
	 // a classification:
	 map<int, Uuo>::iterator it;
	 it = _localMap._map.find(label);
	 if (it == _localMap._map.end()) {
	   cout << "ERROR!  classification uuo '" << label << "' not found! map:" << endl;
	   cout << _localMap.toString() << endl;
	 } else {
	     // Do bayesian update
	     double haz = it->second.probHazard;
	     if (hazard) {
		 it->second.probHazard = (_Pc*haz) / (_Pc*haz + (1-_Pc)*(1-haz));
		 it->second.m_hist = it->second.m_hist + "x";
	     }
	     else {
		 it->second.probHazard = ( (1-_Pc)*haz ) / ( (1-_Pc)*haz + _Pc*(1-haz) );
		 it->second.m_hist = it->second.m_hist + "o";
	     }
	 }
	 //m_Comms.Notify("LOCAL_SENSOR_MESSAGE", printStateMessage());
	 //	 cout << "Saw mine #" << label << endl;

	 // Report to all vehicles the detection (when they come
	 // within range

	 // publish hazard report directly
	 //	 stringstream msg;
	 //	 msg  << "src_node=" << _vehicle_name << ","
	 //	      << "dest_node=all,var_name=HANDLE_SENSOR_MESSAGE," 
	 //	      << "string_val=" << str;
	 //	 m_Comms.Notify("NODE_MESSAGE_LOCAL", msg.str());
       }
       else {
	 cout << "Invalid Message received: " << str << endl;
       }

       // Do classification and print current state
       //       classifyUuos();
       //              cout << printUuoList() << endl;

     }
     else if (key == "HANDLE_SENSOR_MESSAGE") {
       //      cout << "Got Message from other vehicle" << endl;
       parseStateMessage(msg.GetString());
     }

     else if (key == "UHZ_DETECTION_REPORT") {
	 
       // Parse String
       string str = msg.GetString();
       vector<string> strv = parseString(str,",");

       double x,y;
       bool hazard;
       int label, paramcount = 0;

       for (int i = 0; i < strv.size(); i++) {
	 vector<string> input = parseString(strv[i],"=");

	 if (input[0] == "x") {
	   x = atof(input[1].c_str());
	   paramcount++;
	 }
	 else if (input[0] == "y") {
	   y = atof(input[1].c_str());
	   paramcount++;
	 }
	 else if (input[0] == "label") {
	   label = atoi(input[1].c_str());
	   paramcount++;
	 }
       }

       // check if we got everything necessary
       if (paramcount == 3) {
	 Uuo newMine;
	 newMine.x = x;
	 newMine.y = y;
	 newMine.id = label;
	 newMine.m_hist = ".";
	 map<int, Uuo>::iterator it;

	 it = _localMap._map.find(label);
	 if (it == _localMap._map.end()) {
	   // add new sighting to map
	   newMine.classifyCount = 1;
	   _localMap._map[label] = newMine;
	   //	   cout << "inserted newMine " << " map is " << _localMap.toString() << endl;
	   // Apply the measurement with bayes
	   it = _localMap._map.find(label);
	   double haz = it->second.probHazard;
	   it->second.probHazard = (_Pd*haz) / (_Pd*haz + _Pfa*(1-haz));
	   _last_in_box[label] = true;
	   _last_time_in_box[label] = MOOSTime();
	 }
	 else {
	   it->second.classifyCount++;
	   it->second.m_hist = it->second.m_hist + ".";
	   // do bayesian update
	   double haz = it->second.probHazard;
	   it->second.probHazard = (_Pd*haz) / (_Pd*haz + _Pfa*(1-haz));
	 }
	 _last_detect[label] = MOOSTime();
       }

       // Publish new map to coordinator
       publishFuseComplete();
     }
     else if (key == "UHZ_CONFIG_ACK") {
       // Parse String
       string str = msg.GetString();
       vector<string> strv = parseString(str,",");

       int width = -1;
       double pd = -1;
       double pfa = -1;
       double pclass = -1;

       for (int i = 0; i < strv.size(); i++) {
	 vector<string> input = parseString(strv[i],"=");

	 if (input[0] == "width") {
	   width = atoi(input[1].c_str());
	 }
	 else if (input[0] == "pd") {
	   pd = atof(input[1].c_str());
	 }
	 else if (input[0] == "pfa") {
	   pfa = atof(input[1].c_str());
	 }
	 else if (input[0] == "pclass") {
	   pclass = atof(input[1].c_str());
	 }
       }

       if (width != -1 && pd != -1 && pfa != -1 && pclass != -1) {
	 installSensor(width,pd,pfa,pclass);
	 cout << "Installing sensor: " << width << "," << pd
	      << "," << pfa << "," << pclass << endl;
       }
     }
     else if (key == "NODE_REPORT_LOCAL") {
       NodeRecord new_node_record = string2NodeRecord(msg.GetString().c_str());

       if(!new_node_record.valid()) {
	 cout << "NodeRecord error " << endl;
       }
       //cout << "new_node_record: " << new_node_record.getSpec() << endl;

       _node_record = new_node_record;
     }

     else if (key == "DEPLOY") {
       if (msg.GetString() == "true"){
	 if (_deployed == false) {
	   _starttime = MOOSTime();
	   cout << "RESETTING STARTTIME" << endl;
	 }
	 _deployed = true;
       }
       else if (msg.GetString() == "false") {
	 _deployed = false;
       }
       else {
	 cout << "Invalid message" << endl;
       }
     }
   }
	
   return(true);
}

void HandleSensorData::negativeDetect(int id) {
  // do bayesian update
  map<int,Uuo>::iterator it;
  it = _localMap._map.find(id);
  double haz = it->second.probHazard;
  it->second.probHazard = ((1-_Pd)*haz) / ((1-_Pd)*haz + (1-_Pfa)*(1-haz));
  it->second.m_hist = it->second.m_hist + "^";
}

//------------------------------------------------------------
// Procedure: updateVehicleHazardStatus

bool HandleSensorData::updateVehicleHazardStatus(int hazard_int)
{

  bool previous_contained_status = _last_in_box[hazard_int];

  // Now figure out what the new status should be
  double haz_x  = _localMap._map[hazard_int].x;
  double haz_y  = _localMap._map[hazard_int].y;

  bool new_contained_status = _poly.contains(haz_x, haz_y);
  
  _last_in_box[hazard_int] = new_contained_status;

  if((new_contained_status == true) && (previous_contained_status == false)) {
      return(true);
  }

  return(false);
}

void HandleSensorData::calcSwathGeometry(double swath_wid, double& phi, double& hypot)
{
  double m_swath_len = 5.0;
  phi   = atan(m_swath_len / swath_wid);
  hypot = swath_wid / cos(phi);
  phi   = radToDegrees(phi);
}
//------------------------------------------------------------
// Procedure: updateNodePolygon

bool HandleSensorData::updateNodePolygon()
{

  double x     = _node_record.getX();
  double y     = _node_record.getY();
  double hdg   = _node_record.getHeading();
  string vname = _node_record.getName();

  double swath_width = _width;

  double phi, hypot;
  calcSwathGeometry(swath_width, phi, hypot);

  double x1,y1, hdg1 = angle360(hdg + (90-phi));
  double x2,y2, hdg2 = angle360(hdg + (90+phi));
  double x3,y3, hdg3 = angle360(hdg + (90-phi) + 180);
  double x4,y4, hdg4 = angle360(hdg + (90+phi) + 180);

  projectPoint(hdg1, hypot, x, y, x1, y1); 
  projectPoint(hdg2, hypot, x, y, x2, y2); 
  projectPoint(hdg3, hypot, x, y, x3, y3); 
  projectPoint(hdg4, hypot, x, y, x4, y4); 
  
  XYPolygon poly;
  poly.add_vertex(x1, y1);
  poly.add_vertex(x2, y2);
  poly.add_vertex(x3, y3);
  poly.add_vertex(x4, y4);

  poly.set_color("edge", "green");
  poly.set_color("fill", "white");
  poly.set_vertex_size(0);
  poly.set_edge_size(0);

  string label = "sensor_swath_" + vname;
  poly.set_label(label);
  poly.set_msg("_null_");
  
  poly.set_transparency(0.5);

  _poly = poly;
  return(true);
}

void HandleSensorData::runNegativeDetector() {
  updateNodePolygon();

  // For each hazard, determine if the hazard is newly within the sensor 
  // swath of the requesting vehicle.
  
  map<int,Uuo>::iterator p;
  for(p=_localMap._map.begin(); p!=_localMap._map.end(); p++) {
    int hlabel = p->first;
    bool new_report = updateVehicleHazardStatus(hlabel);  // detection dice
    if(new_report) {
      _last_time_in_box[p->first] = MOOSTime();
    }

    if (_last_time_in_box[p->first] - _last_detect[p->first] > 1.0 &&
	MOOSTime() -  _last_time_in_box[p->first] > 1.0) {
      // We missed it
      negativeDetect(p->first);
      _last_detect[p->first] = _last_time_in_box[p->first];
    }
  }
}


void HandleSensorData::parseStateMessage(string msg) {
  // cout << "Parsing: " << endl;
  //cout << msg << endl;
    _otherMap.fromString(msg);
}

MarkerMap HandleSensorData::fuseMaps() {
  // Ask Rob to type up the bayes math.

  map<int, Uuo>::iterator it_m, it_other;
  MarkerMap newmap = _localMap;
  for (it_other = _otherMap._map.begin(); it_other != _otherMap._map.end(); it_other++) {
    it_m = newmap._map.find(it_other->second.id);
    if (it_m == newmap._map.end()) {
      newmap._map[it_other->second.id] = it_other->second;
    }
    else {
      // integrate the two sets of measurements
      double Pa = it_m->second.probHazard;
      double Pb = it_other->second.probHazard;
      it_m->second.probHazard = (Pa*Pb/PRIOR_PROB) / ( (Pa*Pb/PRIOR_PROB) + ((1-Pa)*(1-Pb)/(1-PRIOR_PROB)) );
      it_m->second.m_hist = it_m->second.m_hist + it_other->second.m_hist;
    }
  }

  // Now newmap's _mines is a complete picture of the map state
  return newmap;
}

MarkerMap HandleSensorData::fuseLocalData() {
  // Ask Rob to type up the bayes math.

  map<int, Uuo>::iterator it_m, it_other;
  MarkerMap newmap = _localMap;
  for (it_other = _otherMap._map.begin(); it_other != _otherMap._map.end(); it_other++) {
    it_m = newmap._map.find(it_other->second.id);
    if (it_m == newmap._map.end()) {
      // do nothing, we can't measure this point anyway
    }
    else {
      // integrate the two sets of measurements
      double Pa = it_m->second.probHazard;
      double Pb = it_other->second.probHazard;
      it_m->second.probHazard = (Pa*Pb/PRIOR_PROB) / ( (Pa*Pb/PRIOR_PROB) + ((1-Pa)*(1-Pb)/(1-PRIOR_PROB)) );
      it_m->second.m_hist = it_m->second.m_hist + it_other->second.m_hist;
    }
  }

  // Now newmap's _mines is a complete picture of the map state
  return newmap;
}

void HandleSensorData::publishFuseComplete() {
  MarkerMap newmap = fuseMaps();
  string out = newmap.toString();
  //  newmap.fromString(out);
  m_Comms.Notify("FUSE_COMPLETE", out);
  //  cout << "FUSE message sent " << endl;;
}

void HandleSensorData::classifyUuos() {
  if (MOOSTime() - _classifyTime > _classify_min_time) {
    //    cout << "Best idx was " << best_idx << endl;
    MarkerMap fuseMap = fuseLocalData();
    double best_idx = fuseMap.getPriorityMineIndex();
    if (best_idx > -1) {
      if (_localMap._map[best_idx].classifyCount < 1) {
	cout << "ERROR!  BAD FUSE MAP IN CLASSIFY UUOS" << endl;
      }
      _localMap._map[best_idx].classifyCount--;
      // Post request
      stringstream s;
      s << "vname=" << tolower(_vehicle_name) << ",label="
	<< best_idx << endl;
      m_Comms.Notify("UHZ_CLASSIFY_REQUEST",s.str());
      _classifyTime = MOOSTime();
    }
    else {
      // Do nothing, wait unitl we get mines or more measurements
    }
  }
  return;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool HandleSensorData::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);
	
   RegisterVariables();
   return(true);
}

string HandleSensorData::printStateMessage() {
  return _localMap.toString();
}

void HandleSensorData::printHumanHazardFile(MarkerMap& inmap) {
  std::map<int, Uuo>::iterator it;
  ofstream outfile;
  string filename = "HazardOutput" + _vehicle_name + ".txt";
  outfile.open(filename.c_str());
  for (it = inmap._map.begin(); it != inmap._map.end(); it++) {
    outfile << "[" << it->second.id << "] = " 
	    << setprecision(4) << fixed << it->second.probHazard 
	    << "\t" << it->second.isHazard() << "\t"
	    << inmap.getPriority(it->second) << "\t" 
	    << it->second.m_hist << endl;
  }
  outfile.close();
  return;
}

void HandleSensorData::printHumanHazardFile(MarkerMap& inmap, string filename) {
  std::map<int, Uuo>::iterator it;
  ofstream outfile;
  outfile.open(filename.c_str());
  for (it = inmap._map.begin(); it != inmap._map.end(); it++) {
    outfile << "[" << it->second.id << "] = " 
	    << setprecision(4) << fixed << it->second.probHazard 
	    << "\t" << it->second.isHazard() << "\t" 
	    << inmap.getPriority(it->second) << "\t" 
	    << it->second.m_hist << endl;
  }
  outfile.close();
  return;
}

//---------------------------------------------------------
// Procedure: Iterate()

bool HandleSensorData::Iterate()
{	
  //  cout << "Iterate Start" << endl;

  // run classify subroutine.  Handles time internally
  classifyUuos();
  runNegativeDetector();

  if (_iter_count == 20) {
    // Constantly send state messages to other vehicle
    stringstream msg;
    msg << "src_node=" << tolower(_vehicle_name) 
    	<< ",dest_node=all,var_name=HANDLE_SENSOR_MESSAGE,string_val=\"" 
      	<< printStateMessage() << "\"";
      //	<< "" << _msg_idx;
      m_Comms.Notify("NODE_MESSAGE_LOCAL", msg.str() );
    _msg_idx++;

    // reset iteration counter
    _iter_count = 0;

    //    cout << "Mission Time: " << MOOSTime() - _starttime << endl;
  }

  if ( (int)(floor(MOOSTime() - _starttime)) % 200 == 0) {
    MarkerMap newmap = fuseMaps();
    printHumanHazardFile(newmap);
    string str = _vehicle_name + "localMap.txt";
    printHumanHazardFile(_localMap, str);
    str = _vehicle_name + "otherMap.txt";
    printHumanHazardFile(_otherMap, str);
  }

  if (_isPrimary && !_lockout && MOOSTime() - _starttime > _endtime) {
    cout << "Generating Report" << endl;
    generateHazardReport();
    _lockout = true;
  }

  _iter_count++;
  return(true);
}

void HandleSensorData::generateHazardReport() {
  // Generate and publish a hazard report

  // first fuse both copies of maps
  MarkerMap finalmap = fuseMaps();

  map<int, Uuo>::iterator it;
  XYHazardSet set;
  set.setSource(tolower(_vehicle_name));
  for (it = finalmap._map.begin(); it != finalmap._map.end(); it++) {
    string type;
    if (it->second.isHazard()) {
      type = "hazard";
    }
    else {
      type = "benign";
    }
    set.addHazard(it->second.x, it->second.y, type, intToString(it->second.id));
  }

  string out = set.getSpec();
  m_Comms.Notify("HAZARD_REPORT", out );

  printHumanHazardFile(finalmap);
}

void HandleSensorData::installSensor(int width, double pd) {
  // check for validity
  if (!(width == 50 || width == 25 || width == 10 || width == 5)) {
    cout << "Invalid width installed" << endl;
  }
  else if (pd < 0 || pd > 1) {
    cout << "Invalid pd installed" << endl;
  }
  else {
    if (width == 50) {
      _width = 50;
      _Pc = 0.6;
      _Pd = pd;
      _Pfa = pow(pd,2);
    }
    else if (width == 25) {
      _width = 25;
      _Pc = 0.8;
      _Pd = pd;
      _Pfa = pow(pd,4);
    }
    else if (width == 10) {
      _width = 10;
      _Pc = 0.93;
      _Pd = pd;
      _Pfa = pow(pd,6);
    }
    else if (width == 5) {
      _width = 5;
      _Pc = 0.98;
      _Pd = pd;
      _Pfa = pow(pd,8);
    }
    else {
      cout << "How the hell did you get here?" << endl;
    }
  }

  return;
}

void HandleSensorData::installSensor(int width, double pd, double pfa, double pclass) {
  // check for validity
  if (!(width == 50 || width == 25 || width == 10 || width == 5)) {
    cout << "Invalid width installed" << endl;
  }
  else if (pd < 0 || pd > 1) {
    cout << "Invalid pd installed" << endl;
  }
  else if (pfa < 0 || pfa > 1) {
    cout << "Invalid pfa installed" << endl;
  }
  else if (pclass < 0 || pclass > 1) {
    cout << "Invalid pclass installed" << endl;
  }
  else {
    _width = width;
    _Pc = pclass;
    _Pd = pd;
    _Pfa = pfa;
  }

  return;
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool HandleSensorData::OnStartUp()
{
  cout << "***************************" << endl;
  cout << "Starting pHandleSensorData" << endl;
  cout << "***************************" << endl;
  // Defaults
  _vehicle_name = "EDGAR_THE_WONDERFUL";
  _isPrimary = false;
  _msg_idx = 0;
  _iter_count = 0;
  //  _finished_survey = false;
  //  _rechecked = false;
  _lockout = false;
  //  _allDone = false;
  _report_count = 0;
  _starttime = MOOSTime();
  _classifyTime = _starttime;
  _endtime = 8500.0;
  _classify_min_time = 30.0;
  _deployed = false;

  // Default sensor settings
  installSensor(50,0.9); // Widest sensor, sensible Pd

  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  // Assign variables from mission configuration
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine     = *p;
    string sVarName  = MOOSChomp(sLine, "=");
    sLine = stripBlankEnds(sLine);
    
    if(MOOSStrCmp(sVarName, "VEHICLE_NAME")) {
      if(!strContains(sLine, " ")) {
	_vehicle_name = toupper(stripBlankEnds(sLine));
      }
    }
    
    else if(MOOSStrCmp(sVarName, "SET_PRIMARY")) {
      if(!strContains(sLine, " ")) {
	if (stripBlankEnds(sLine) == "TRUE"){
	  _isPrimary = true;
	}
	else if (stripBlankEnds(sLine) == "FALSE"){
	  _isPrimary = false;
	}
	else {
	  cout << "Invalid SET_PRIMARY configuration" << endl;
	}
      }
    }

    else if(MOOSStrCmp(sVarName, "CLASSIFY_MIN_TIME")) {
      if(!strContains(sLine, " ")) {
	_classify_min_time = atof(stripBlankEnds(sLine).c_str());
      }
    }

    else if(MOOSStrCmp(sVarName, "HAZARD_REPORT_TIME")) {
      if(!strContains(sLine, " ")) {
	_endtime = atof(stripBlankEnds(sLine).c_str());
      }
    }   
  }

   RegisterVariables();	
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void HandleSensorData::RegisterVariables()
{
  //m_Comms.Register("FOOBAR", 0);
  m_Comms.Register("UHZ_HAZARD_REPORT", 0);
  m_Comms.Register("UHZ_DETECTION_REPORT",0);
  //  m_Comms.Register("SURVEY", 0);
  //  m_Comms.Register("RECHECKED", 0);
  //  m_Comms.Register("ALL_DONE", 0);
  m_Comms.Register("UHZ_CONFIG_ACK", 0);

  //  if (_isPrimary) {
  m_Comms.Register("HANDLE_SENSOR_MESSAGE", 0);
  //  }
  m_Comms.Register("NODE_REPORT_LOCAL", 0);
  m_Comms.Register("DEPLOY",0);
}

