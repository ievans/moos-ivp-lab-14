/************************************************************/
/*    NAME: robtruax                                              */
/*    ORGN: MIT                                             */
/*    FILE: HandleSensorData.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
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
	 it = _mines.find(label);
	 if (it == _mines.end()) {
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
	 // TODO:
	 // INTEGRATE NEW DATA WITH BAYES
	 Uuo newMine;
	 newMine.x = x;
	 newMine.y = y;
	 newMine.id = label;
	 map<int, Uuo>::iterator it;

	 it = _mines.find(label);
	 if (it == _mines.end()) {
	   // add new sighting to map
	   newMine.classifyCount = 1;
	   _mines.insert(pair<int,Uuo>(label,newMine));
	 }
	 else {
	   // do bayesian update
	   double haz = it->second.probHazard;
	   it->second.probHazard = (_Pd*haz) / (_Pd*haz + _Pfa*(1-haz));
	 }
       }
     }
   }
	
   return(true);
}

void HandleSensorData::parseStateMessage(string msg) {
  Uuo mine;
  map<int,Uuo>::iterator it;
  vector<string> strvect = parseString(msg,":");
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
      cout << msg << endl;
    }
  }
}

void HandleSensorData::classifyUuos() {

  //TODO
  /*
  map<int, Uuo>::iterator it, itother;

  // First, combine other_mine list into primary mine list
  for (itother = _mines_other.begin(); itother != _mines_other.end(); itother++) {
    it = _mines.find(itother->second.label);
    if (it == _mines.end()) {
      // add in
      _mines.insert(*itother);
    }
    else {
      // add in counts
      it->second.hazard_count += itother->second.hazard_count;
      it->second.neutral_count += itother->second.neutral_count;
    }
  }


  for (it = _mines.begin(); it != _mines.end(); it++) {
    int hazardcount = it->second.hazard_count;
    int neutralcount = it->second.neutral_count;
    if (hazardcount >= neutralcount) {
      it->second.isHazard = true;
    }
    else {
      it->second.isHazard = false;
    }
  }
  */
  return;
}

string HandleSensorData::printUuoList() {
  stringstream out;
  map<int, Uuo>::iterator it;
  for (it = _mines.begin(); it != _mines.end(); it++) {
    out << "Uuo[" << it->second.id <<"] = (" << it->second.x << ","
	<< it->second.y << ") hazard=" << it->second.isHazard() << endl;
  }
  return out.str();
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
}

//---------------------------------------------------------
// Procedure: Iterate()

bool HandleSensorData::Iterate()
{	
  //  cout << "Iterate Start" << endl;
  if (_iter_count == 200) {
    // Constantly send state messages to other vehicle
    stringstream msg;
    msg << "src_node=" << tolower(_vehicle_name) 
	<< ",dest_node=all,var_name=HANDLE_SENSOR_MESSAGE,string_val=\"" 
	<< printStateMessage() << "\"";
    m_Comms.Notify("NODE_MESSAGE_LOCAL", msg.str() );
    _msg_idx++;
    //    cout << "Trying to publish " << printStateMessage() << endl;

    // Post local copy
    m_Comms.Notify("LOCAL_SENSOR_MESSAGE", printStateMessage());

    // reset iteration counter
    _iter_count = 0;
  }
  else if (_isPrimary && !_lockout && MOOSTime() - _starttime > _endtime) {
    classifyUuos();
    generateHazardReport();
    _lockout = true;
    _iter_count = 0;
  }

  _iter_count++;
  return(true);
}

void HandleSensorData::generateHazardReport() {
  // Generate and publish a hazard report
  map<int, Uuo>::iterator it;
  XYHazardSet set;
  set.setSource(tolower(_vehicle_name));
  for (it = _mines.begin(); it != _mines.end(); it++) {
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

  // Default sensor settings
  _Pd = 1;
  _Pfa = 1;
  _Pc = 0.6;
  _width = 25;

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

    /*    else if(MOOSStrCmp(sVarName, "NUMTOCOLLECT")) {
      if(!strContains(sLine, " ")) {
	_numToCollect = atoi(stripBlankEnds(sLine).c_str());
	_total_unvisited = _numToCollect;
      }
      }   */
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

  if (_isPrimary) {
    m_Comms.Register("HANDLE_SENSOR_MESSAGE", 0);
  }
}

