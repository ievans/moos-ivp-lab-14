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
#include <math.h>
#include <queue>

#include "../shared/MarkerMap.h"
#include "../shared/UUO.h"

//#define CLASSIFY_MIN_TIME 30 // seconds

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
   void installSensor(int,double);
   void installSensor(int,double,double,double);
   MarkerMap fuseMaps();
   void printHumanHazardFile(MarkerMap&);
   void printHumanHazardFile(MarkerMap&, string);
   void publishFuseComplete();

   string _vehicle_name;
   double _classify_min_time;
   double _endtime;

   bool _isPrimary;
   int _msg_idx;
   int _iter_count;
   //  bool _finished_survey;
   //  bool _rechecked;
   bool _lockout;
   //  bool _allDone;
   int _report_count;
   double _starttime;

   MarkerMap _localMap, _otherMap;

   // Sensor settings for this vehicle
   double _Pd;   // prob detection
   double _Pfa;  // prob false detection
   double _Pc;   // prob classification
   int _width;   // swath width

   // classify variables
   double _classifyTime;

   // Do something like this later?
   // For use in doing classify priority
   //std::priority_queue<Uuo> _classifyQueue;
};

#endif 
