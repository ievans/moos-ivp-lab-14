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

class Mine {
 public:
  double x;
  double y;
  int hazard_count;
  int neutral_count;
  int label;
  bool isHazard;
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

   string printMineList();
   string printStateMessage();
   void classifyMines();
   void generateHazardReport();

   string _vehicle_name;
   map<int, Mine> _mines;
   map<int, Mine> _mines_other;
   bool _isPrimary;
   int _msg_idx;
   int _iter_count;
   bool _finished_survey;
   bool _rechecked;
   bool _lockout;
   bool _allDone;
   int _report_count;
};

#endif 
