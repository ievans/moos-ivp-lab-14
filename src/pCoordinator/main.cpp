/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: main.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <string>
#include "MOOSLib.h"
#include "MOOSGenLib.h"
#include "MBUtils.h"
#include "Coordinator_ExampleConfig.h"
#include "Coordinator.h"

using namespace std;

int main(int argc, char *argv[])
{
  string mission_file = "";
  string run_command  = argv[0];
  bool   help_requested = false;
  bool   vers_requested = false;
  bool   exam_requested = false;

  int    i;
  for(i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-v") || (argi=="--version") || (argi=="-version"))
      vers_requested = true;
    else if((argi=="-e") || (argi=="--example") || (argi=="-example"))
      exam_requested = true;
    else if((argi == "--help")||(argi=="-h"))
      help_requested = true;
    else if(strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
      mission_file = argv[i];
    else if(strBegins(argi, "--alias="))
      run_command = argi.substr(8);
    else if(i==2)
     run_command = argi;
  }
  
  if(vers_requested) {
    cout << "pCoordinator Version 0.1" << endl;
    return(0);
  }

  if(exam_requested) {
    showExampleConfig();
    return(0);
  }

  if((mission_file == "") || help_requested) {
    cout << "Usage: pCoordinator file.moos [OPTIONS]                 " << endl;
    cout << "                                                    " << endl;
    cout << "Options:                                            " << endl;
    cout << "  --alias=<ProcessName>                             " << endl;
    cout << "      Launch pCoordinator with the given process  " << endl;
    cout << "      name rather than pCoordinator.               " << endl;
    cout << "  --example, -e                                     " << endl;
    cout << "      Display example MOOS configuration block.     " << endl;
    cout << "  --help, -h                                        " << endl;
    cout << "      Display this help message.                    " << endl;
    cout << "  --version,-v                                      " << endl;
    cout << "      Display the release version of pCoordinator. " << endl;
    return(0);
  }

  cout << "pCoordinator running as: " << run_command << endl;

  Coordinator Coordinator;

  Coordinator.Run(run_command.c_str(), mission_file.c_str());
  
  return(0);
}

