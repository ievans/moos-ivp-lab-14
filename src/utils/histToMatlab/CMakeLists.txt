#--------------------------------------------------------
# The CMakeLists.txt for:                       logsToMatlab
# Author(s):                              robtruax
#--------------------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(histToMatlab ${SRC})

TARGET_LINK_LIBRARIES(histToMatlab
   MOOS
   MOOSGen
   geometry
   mbutil
   contacts
   m
   pthread)

