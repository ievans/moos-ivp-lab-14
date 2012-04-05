/************************************************************/
/*    NAME: Isaac Evans, Robin Truax                        */
/*    ORGN: MIT                                             */
/*    FILE: MarkerMap.h                                     */
/*    DATE: April 5, 2012                                   */
/************************************************************/
#ifndef MARKERMAP_HEADER
#define MARKERMAP_HEADER

#include "UUO.h"

class MarkerMap {
public:
    std::map<int, Uuo> _mines;
    std::map<int, Uuo> _mines_other;

    MarkerMap() {

    };

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

};

#endif
