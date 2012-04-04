#--------------------------------------------------------
# The CMakeLists.txt for:                       pHandleSensorData
# Author(s):                              robtruax
#--------------------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(pHandleSensorData ${SRC})

TARGET_LINK_LIBRARIES(pHandleSensorData
   MOOS
   MOOSGen
   geometry
   mbutil
   m
   pthread)

