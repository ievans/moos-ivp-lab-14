/****************************************************************/
/*   NAME:                                              */
/*   ORGN: MIT Cambridge MA                                     */
/*   FILE: Coordinator_ExampleConfig.cpp                               */
/*   DATE: Dec 29th 1963                                        */
/****************************************************************/

#include <iostream>
#include "Coordinator_ExampleConfig.h"
#include "ColorParse.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showExampleConfig                                  

void showExampleConfig()
{
  cout << "                                                   " << endl;
  cout << "===================================================" << endl;
  cout << "pCoordinator Example MOOS Configuration                " << endl;
  cout << "===================================================" << endl;
  cout << "                                                   " << endl;
  cout << "ProcessConfig = pCoordinator                           " << endl;
  cout << "{                                                  " << endl;
  cout << "  AppTick   = 4                                    " << endl;
  cout << "  CommsTick = 4                                    " << endl;
  cout << "                                                   " << endl;
  cout << "  Port = /dev/ttyUSB0                              " << endl;
  cout << "  Timeout = 6                                      " << endl;
  cout << "                                                   " << endl;
  cout << "  // If 1 then heading computed from magnetometers " << endl;
  cout << "  ComputeHeading = 0                               " << endl;
  cout << "                                                   " << endl;
}
