#--------------------------------------------------------
# The CMakeLists.txt for:                       pFollowOrders
# Author(s):                              
#--------------------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(pFollowOrders ${SRC})

TARGET_LINK_LIBRARIES(pFollowOrders
   MOOS
   MOOSGen
   mbutil
   m
   pthread)

