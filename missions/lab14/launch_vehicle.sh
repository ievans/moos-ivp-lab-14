#!/bin/bash 

WARP=1
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
HOSTNAME=$(hostname -s)
VNAME=$(id -un)
MOOS_PORT="9201"
UDP_LISTEN_PORT="9301"

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
let COUNT=0
for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI:0:6}" = "--warp" ] ; then
	WARP="${ARGI#--warp=*}"
	UNDEFINED_ARG=""
    fi
    if [ "${ARGI:0:7}" = "--shore" ] ; then
	SHOREIP="${ARGI#--shore=*}"
	UNDEFINED_ARG=""
    fi
    if [ "${ARGI:0:7}" = "--mport" ] ; then
	MOOS_PORT="${ARGI#--mport=*}"
	UNDEFINED_ARG=""
    fi
    if [ "${ARGI:0:7}" = "--lport" ] ; then
	UDP_LISTEN_PORT="${ARGI#--lport=*}"
	UNDEFINED_ARG=""
    fi
    if [ "${ARGI:0:7}" = "--vname" ] ; then
	VNAME="${ARGI#--vname=*}"
	UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	HELP="yes"
	UNDEFINED_ARG=""
    fi
    # Handle Warp shortcut
    if [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$COUNT" = 0 ]; then 
        WARP=$ARGI
        let "COUNT=$COUNT+1"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
	JUST_BUILD="yes"
	UNDEFINED_ARG=""
    fi
    if [ "${UNDEFINED_ARG}" != "" ] ; then
	BAD_ARGS=$UNDEFINED_ARG
    fi
done

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

if [ "${HELP}" = "yes" ]; then
    printf "%s [SWITCHES]                           \n" $0
    printf "Switches:                               \n" 
    printf "  --warp=WARP_VALUE                     \n" 
    printf "  --vname=VEHICLE_NAME                  \n" 
    printf "  --mport=MOOSDB Port #                 \n" 
    printf "  --lport=MOOSBridge UDPListen Port #   \n" 
    printf "  --just_build, -j                      \n" 
    printf "  --help, -h                            \n" 
    exit 0;
fi

# Second check that the warp argument is numerical
if [ "${WARP//[^0-9]/}" != "$WARP" ]; then 
    printf "Warp values must be numerical. Exiting now."
    exit 127
fi

#-------------------------------------------------------
#  Part 2: Create the .moos and .bhv files. 
#-------------------------------------------------------

GROUP="GROUP12"
FULL_VNAME=$VNAME"@"$HOSTNAME
WPT_COLOR="light_blue"

VNAME1="archie@jleonard"  # The first vehicle Community
VPORT1="9201"
LPORT1="9301"

VNAME9="betty@jleonard"
VPORT9="9209"
LPORT9="9309"

START_POS1="0,0"   # Vehicle 1 Behavior configurations
START_POS9="0,0"   # Vehicle 9 UUV configuration

nsplug meta_archie.moos targ_archie.moos -f WARP=$WARP       \
   VNAME=$VNAME1 VPORT=$VPORT1 LPORT=$LPORT1                 \
   START_POS=$START_POS1

nsplug meta_betty.moos targ_betty.moos -f WARP=$WARP         \
   VNAME=$VNAME9 VPORT=$VPORT9 LPORT=$LPORT9                 \ 
   
nsplug meta_archie.bhv targ_archie.bhv -f VNAME=$VNAME1      \
    START_POS=$START_POS1 

nsplug meta_betty.bhv targ_betty.bhv -f VNAME=$VNAME9        \
    START_POS=$START_POS9    
    
if [ ${JUST_BUILD} = "yes" ] ; then
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Launch the processes
#-------------------------------------------------------


printf "Launching $VNAME1 MOOS Community (WARP=%s) \n" $WARP
pAntler targ_archie.moos >& /dev/null &
sleep 1
printf "Launching $VNAME9 MOOS Community (WARP=%s) \n" $WARP
pAntler targ_betty.moos >& /dev/null &
sleep 1



#-------------------------------------------------------
#  Part 4: Exiting and/or killing the simulation
#-------------------------------------------------------

ANSWER="0"
while [ "${ANSWER}" != "1" -a "${ANSWER}" != "2" ]; do
    printf "Now what? (1) Exit script (2) Exit and Kill Simulation \n"
    printf "> "
    read ANSWER
done

# %1, %2, %3 matches the PID of the first three jobs in the active
# jobs list, namely the three pAntler jobs launched in Part 3.
if [ "${ANSWER}" = "2" ]; then
    printf "Killing all processes ... \n"
    kill %1 
    printf "Done killing processes.   \n"
fi
