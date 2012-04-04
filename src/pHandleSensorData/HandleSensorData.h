/************************************************************/
/*    NAME: robtruax                                              */
/*    ORGN: MIT                                             */
/*    FILE: HandleSensorData.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef HandleSensorData_HEADER
#define HandleSensorData_HEADER

#include "MOOSLib.h"
#include "MBUtils.h"
#include <stdlib.h>
#include "XYHazardSet.h"

using namespace std;

class Uuo { // unitenditfied underwater vehicle
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
    probHazard = 0.5; // Maximum entropy?  Better prior?
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
};

class HandleSensorData : public CMOOSApp
{
 public:
   HandleSensorData();
   ~HandleSensorData();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

private:

   string printUuoList();
   string printStateMessage();
   void classifyUuos();
   void generateHazardReport();
   void parseStateMessage(string);

   string _vehicle_name;
   map<int, Uuo> _mines;
   map<int, Uuo> _mines_other;
   bool _isPrimary;
   int _msg_idx;
   int _iter_count;
   //  bool _finished_survey;
   //  bool _rechecked;
   bool _lockout;
   //  bool _allDone;
   int _report_count;
   double _starttime;
   double _endtime;
};

#endif 
