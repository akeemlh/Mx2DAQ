echo ---------------------------------------------------------------------------
echo Welcome to the MINERvA DAQ Software Environment.
echo
echo Your LOCALE is $LOCALE
echo ---------------------------------------------------------------------------

export DAQROOT=/work/software/croce/minervadaq/minervadaq
export CAEN_DIR=/work/software/CAENVMElib
export ET_HOME=$DAQROOT/et_9.0/Linux-x86_64-64
export ET_LIBROOT=$ET_HOME/Linux-x86_64-64
# Add $ET_LIBROOT/lib & $CAEN_DIR/lib for ET & CAEN libraries.
export LD_LIBRARY_PATH=$DAQROOT/lib:$ET_LIBROOT/lib:$CAEN_DIR/lib/x86_64/:$LD_LIBRARY_PATH
# Add local SQLite
export LD_LIBRARY_PATH=$DAQROOT/sqlite/lib:$LD_LIBRARY_PATH
export PATH=$DAQROOT/sqlite/bin:$PATH
# Add /usr/local/lib for log4cpp support.
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib

export BMS_HOME=${DAQROOT}/et_9.0/BMS
export INSTALL_DIR=$ET_HOME
export ET_USE64BITS=1

echo Your DAQROOT is $DAQROOT
echo Your CAEN_DIR is $CAEN_DIR
echo Your BMS_HOME is $BMS_HOME
echo Your ET INSTALL_DIR is $INSTALL_DIR
echo Your ET_HOME is $ET_HOME
echo Your ET_LIBROOT is $ET_LIBROOT
echo Your ET_USE64BITS is $ET_USE64BITS
echo Your LD_LIBRARY_PATH is $LD_LIBRARY_PATH

echo ---------------------------------------------------------------------------

