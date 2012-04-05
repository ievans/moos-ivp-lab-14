#--------------------------------------------------------
# The CMakeLists.txt for:                       pCoordinator
# Author(s):                              
#--------------------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(pCoordinator ${SRC})

TARGET_LINK_LIBRARIES(pCoordinator
   MOOS
   MOOSGen
   mbutil
   m
   pthread)

