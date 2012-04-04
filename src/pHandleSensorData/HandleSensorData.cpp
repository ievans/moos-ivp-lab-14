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
	 map<int, Mine>::iterator it = _mines.find(label);
	 if (it == _mines.end()) {

	   Mine temp_mine;
	   temp_mine.x = x;
	   temp_mine.y = y;
	   temp_mine.label = label;

	   // Don't hold onto noisy first pass data
	   if (_finished_survey) {
	     if (hazard) {
	       temp_mine.hazard_count = 1;
	       temp_mine.neutral_count = 0;
	     }
	     else {
	       temp_mine.hazard_count = 0;
	       temp_mine.neutral_count = 1;
	     }
	   }
	   else {
	     temp_mine.hazard_count = 0;
	     temp_mine.neutral_count = 0;
	   }

	   _mines.insert( pair<int,Mine>(label,temp_mine) );

	 }
	 // don't count noisy first pass survey data
	 else if (_finished_survey) {
	   if (hazard) {
	     it->second.hazard_count++;
	   }
	   else {
	     it->second.neutral_count++;
	   }
	 }
	 m_Comms.Notify("LOCAL_SENSOR_MESSAGE", printStateMessage());
	 cout << "Saw mine #" << label << endl;

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
       //       classifyMines();
       //              cout << printMineList() << endl;

     }
     else if (key == "HANDLE_SENSOR_MESSAGE") {
       // Parse state of other vehicle and make copy
       Mine mine;
       map<int,Mine>::iterator it;
       string instr = msg.GetString();
       vector<string> strvect = parseString(instr,",");
       for (int i = 0; i < strvect.size(); i++) {
	 vector<string> in = parseString(strvect[i],":");
	 if (in.size() != 5) {
	   cout << "Invalid Mine format found at " << i << endl;
	 }
	 else {
	   mine.label = atoi(in[0].c_str());
	   mine.x = atof(in[1].c_str());
	   mine.y = atof(in[2].c_str());
	   mine.hazard_count = atoi(in[3].c_str());
	   mine.neutral_count = atoi(in[4].c_str());

	   it = _mines_other.find(mine.label);
	   if (it == _mines_other.end()) {
	     _mines_other.insert( pair<int,Mine>(mine.label,mine) );
	   }
	   else {
	     it->second.hazard_count = mine.hazard_count;
	     it->second.neutral_count = mine.neutral_count;
	   }
	 }
       }
     }

     else if (key == "SURVEY") {
       if (msg.GetString() == "COMPLETE") {
	 _finished_survey = true;
	 cout << "Survey Completed" << endl;
       }
       else {
	 cout << "SURVEY not completed" << endl;
       }
     }

     else if (key == "RECHECKED") {
       if (msg.GetString() == "true") {
	 _rechecked = true;
	 cout << "Rechecked" << endl;
       }
       else {
	 cout << "Rechecked message not true" << endl;
       }
     }

     else if (key == "ALL_DONE") {
       cout << "All_DONE received" << endl;
       _allDone = true;
     }
   }
	
   return(true);
}

void HandleSensorData::classifyMines() {
  map<int, Mine>::iterator it, itother;

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
}

string HandleSensorData::printMineList() {
  stringstream out;
  map<int, Mine>::iterator it;
  for (it = _mines.begin(); it != _mines.end(); it++) {
    out << "Mine[" << it->second.label <<"] = (" << it->second.x << ","
	<< it->second.y << ") hazard=" << it->second.isHazard << endl;
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
  // comma separated
  // index:xpos:ypos:hazard-count:neutral-count

  stringstream msg;
  map<int, Mine>::iterator it;

  if (_mines.begin() == _mines.end()) {
    msg << "NONE";
  }
  else {
    it = _mines.begin();
    msg << it->second.label << ":" << it->second.x << ":" << it->second.y << ":"
	<< it->second.hazard_count << ":" << it->second.neutral_count;

    for (it++; it != _mines.end(); it++) {
      msg << "," << it->second.label << ":" << it->second.x 
	  << ":" << it->second.y << ":" << it->second.hazard_count 
	  << ":" << it->second.neutral_count;
    }
  }

  return msg.str();
}

//---------------------------------------------------------
// Procedure: Iterate()

bool HandleSensorData::Iterate()
{	
  //  cout << "Iterate Start" << endl;
  // if (_finished_survey && _iter_count % 400 == 0) {
  if (_iter_count == 200) {
    // Constantly send state messages to other vehicle
    stringstream msg;
    msg << "src_node=" << tolower(_vehicle_name) 
	<< ",dest_node=all,var_name=HANDLE_SENSOR_MESSAGE,string_val=\"" 
      //     << _msg_idx << "  " << printStateMessage() << "\"";
	<< printStateMessage() << "\"";
    m_Comms.Notify("NODE_MESSAGE_LOCAL", msg.str() );
    _msg_idx++;
    //    cout << "Trying to publish " << printStateMessage() << endl;

    // Post local copy
    m_Comms.Notify("LOCAL_SENSOR_MESSAGE", printStateMessage());
  }
  else if (_iter_count == 400) {
    if (_rechecked) {
      stringstream out;
      out << "src_node=" << tolower(_vehicle_name) 
	  << ",dest_node=all,var_name=ALL_DONE,string_val=true";
      m_Comms.Notify("NODE_MESSAGE_LOCAL", out.str());
    }
  }
  else if (_iter_count == 401) {
    if (_isPrimary && _rechecked && _allDone && !_lockout) {
      if (_report_count > 1) {
	// send final message 
	classifyMines();
	generateHazardReport();
	_lockout = true;
      }
      _report_count++;
    }
    _iter_count = 0;
  }

  _iter_count++;
  return(true);
}

void HandleSensorData::generateHazardReport() {
  // Generate and publish a hazard report
  map<int, Mine>::iterator it;
  XYHazardSet set;
  set.setSource(tolower(_vehicle_name));
  for (it = _mines.begin(); it != _mines.end(); it++) {
    string type;
    if (it->second.isHazard) {
      type = "hazard";
    }
    else {
      type = "benign";
    }
    set.addHazard(it->second.x, it->second.y, type, intToString(it->second.label));
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
  _finished_survey = false;
  _rechecked = false;
  _lockout = false;
  _allDone = false;
  _report_count = 0;

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
  m_Comms.Register("SURVEY", 0);
  m_Comms.Register("RECHECKED", 0);
  m_Comms.Register("ALL_DONE", 0);

  if (_isPrimary) {
    m_Comms.Register("HANDLE_SENSOR_MESSAGE", 0);
  }
}

