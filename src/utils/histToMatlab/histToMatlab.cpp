
#include "MOOSLib.h"
#include "MBUtils.h"
#include <stdlib.h>
#include <math.h>

using namespace std;

int main(int argc, char** argv) {

  if (argc != 3) {
    cout << "Please provide a file to parse and a destination" << endl;
  }

  string line;
  ifstream readfile;
  ofstream outfile;
  outfile.open(argv[2]);
  readfile.open(argv[1]);
  if (readfile.is_open()) {
    while(readfile.good()) {
      getline(readfile,line);
    
      char c = line[0];
      int idx = 0;
      string sub = "";
      while(c != ']') {
	sub = sub + c;
	idx++;
	c = line[idx];
      }

      int id = atoi(line.c_str());
 
    }
  }

  readfile.close();
  outfile.close();

  return 0;
}
