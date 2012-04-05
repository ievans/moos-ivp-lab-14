#include "MOOSLib.h"
#include "MBUtils.h"
#include <stdlib.h>
#include "XYHazardSet.h"
#include <math.h>

#include "UUO.h"

using namespace std;

class MarkerMap {
   map<int, Uuo> _mines;
   map<int, Uuo> _mines_other;

   MarkerMap(string encodedMM) { this->fromString(encodedMM); };

   string toString() {
// TODO
   };
   
   void fromString(string encodedMM) {
// TODO
   };

   void fuseFromString(string encodedMM) {
// TODO
   };

}
