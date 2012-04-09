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
	   paramcount++;
	 }
       }

       // check if we got everything necessary
       if (paramcount == 4) {
	 // We will always have this in our database already for
	 // a classification:
	 map<int, Uuo>::iterator it;
	 it = _map._mines.find(label);
	 if (it == _map._mines.end()) {
	   cout << "ERROR!  classification uuo not found!" << endl;
	 }

	 // Do bayesian update
	 double haz = it->second.probHazard;
	 if (hazard) {
	   it->second.probHazard = (_Pc*haz) / (_Pc*haz + (1-_Pc)*(1-haz));
	 }
	 else {
	   it->second.probHazard = ( (1-_Pc)*haz ) / ( (1-_Pc)*haz + _Pc*(1-haz) );
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
	 map<int, Uuo>::iterator it;

	 it = _map._mines.find(label);
	 if (it == _map._mines.end()) {
	   // add new sighting to map
	   newMine.classifyCount = 1;
	   _map._mines.insert(pair<int,Uuo>(label,newMine));
	 }
	 else {
	   it->second.classifyCount++;
	   // do bayesian update
	   double haz = it->second.probHazard;
	   it->second.probHazard = (_Pd*haz) / (_Pd*haz + _Pfa*(1-haz));
	 }
       }
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
   }
	
   return(true);
}

void HandleSensorData::parseStateMessage(string msg) {
  _map.fromStringToOther(msg);
}

double getPriority(Uuo& mine) {
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
}

void HandleSensorData::classifyUuos() {
  if (MOOSTime() - _classifyTime > _classify_min_time) {
    //    cout << "Trying to Classify Something" << endl;
    // find highest priority point
    // TODO: What is highest priority???
    map<int, Uuo>::iterator it;
    double best_idx = -1;
    double best_priority = -1;

    // TODO: Linear search sux
    for (it = _map._mines.begin(); it != _map._mines.end(); it++) {
      if (it->second.classifyCount > 0) {
	if (getPriority(it->second) > best_priority) {
	  best_idx = it->second.id;
	  best_priority = getPriority(it->second);
	}
      }
    }

    //    cout << "Best idx was " << best_idx << endl;
    if (best_idx > -1) {
      _map._mines[best_idx].classifyCount--;
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
  return _map.toString();
}

//---------------------------------------------------------
// Procedure: Iterate()

bool HandleSensorData::Iterate()
{	
  //  cout << "Iterate Start" << endl;

  // run classify subroutine.  Handles time internally
  classifyUuos();

  if (_iter_count == 200) {
    // Constantly send state messages to other vehicle
    stringstream msg;
    msg << "src_node=" << tolower(_vehicle_name) 
	<< ",dest_node=all,var_name=HANDLE_SENSOR_MESSAGE,string_val=\"" 
	<< printStateMessage() << "\"";
    //    m_Comms.Notify("NODE_MESSAGE_LOCAL", msg.str() );
    _msg_idx++;
    //    cout << "Trying to publish " << printStateMessage() << endl;

    // Post local copy
    //    m_Comms.Notify("LOCAL_SENSOR_MESSAGE", printStateMessage());

    // reset iteration counter
    _iter_count = 0;

    //    cout << "Mission Time: " << MOOSTime() - _starttime << endl;
    //    cout << "_isPrimary = " << _isPrimary << endl;
    //    cout << "_lockout = " << _lockout << endl;
    //    cout << "Timing: " << MOOSTime() << " - " << _starttime
    //	 << " <> " << _endtime << endl;
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
  map<int, Uuo>::iterator it;
  XYHazardSet set;
  set.setSource(tolower(_vehicle_name));
  for (it = _map._mines.begin(); it != _map._mines.end(); it++) {
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

  if (_isPrimary) {
    //    m_Comms.Register("HANDLE_SENSOR_MESSAGE", 0);
  }
}

