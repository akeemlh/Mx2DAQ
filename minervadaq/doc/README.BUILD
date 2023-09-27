This file is current as of Sep 29 2023 - ALH

How to Build the MINERvA DAQ.
-----------------------------

Quick Directions (software already installed):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: This assumes the local SQLite is built as well.

1) Log on to the DAQ machine you want to update as the "mnvonline" user.  On T977 machines, use the "tbonline" 
account instead.  In the $HOME are there will be a set of scripts for launching the Run and Slow Controls, 
configuring the environment, etc.  These scripts will vary from machine to machine, but they are hopefully 
titled in a useful way.  The primary distinction to be aware of is between single and mulit-machine DAQ builds.  
For most production machines, that choice is hidden, but the user should pay attention to the setup scripts just 
in case.

2) Source the setup script to set the DAQ environment variables.  On E938 machines, the script is likely named 
"setupdaqenv.sh."  On T977 machines, it will likely be named "singledaqenv.sh."

3) Change directories to the $DAQROOT.

4) Run the compiler script.

Example:
mnvtbonline0.fnal.gov> cd $DAQROOT
mnvtbonline0.fnal.gov> ./compiler.sh

5) If the build succeeded, you should see the following in the $DAQROOT/lib and $DAQROOT/bin directories:
        
perdue@minervatest04> ll lib/ bin/
bin/:
total 392K
-rwxrwxr-x 1 perdue e938  33K Mar 19 16:52 event_builder*
-rwxrwxr-x 1 perdue e938 105K Mar 19 16:52 minervadaq*
-rwxrwxr-x 1 perdue e938 237K Mar 19 16:52 tests*

lib/:
total 304K
-rwxrwxr-x 1 perdue e938  25K Mar 19 16:52 libevent_structure.so*
-rwxrwxr-x 1 perdue e938 181K Mar 19 16:52 libhardware.so*
-rwxrwxr-x 1 perdue e938  84K Mar 19 16:52 libminerva_workers.so*


Complete Directions (no software installed):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Before building the Mx2 DAQ you need to install the various required prerequisites. On Alma Linux 9 this 
can be done in one line with: 
`sudo dnf -y install wget tar nano git krb5-workstation krb5-libs libusb libusbx epel-release
libudev-devel bzip2 gcc-c++ glibc cmake sqlite-devel && sudo dnf -y install log4cpp-devel`

Then you will need to checkout the minervadaq code. Go to the location you'd like to checkout the 
minervadaq code to and make sure you have a valid kerberos ticket, then run;
`git clone ssh://p-minervadaq@cdcvs.fnal.gov/cvs/projects/minervadaq`

To build the MINERvA Production DAQ (mnvdaq) you need to install and build CAEN driver libraries 
to interface with the CAEN v2718 VME Controller and a2818 PCI Interface Card as well as the CAENVME library,
CAENVMELib. These are available on the CAEN website (http://www.caen.it/nuclear/index.php). The  most recent
CAENlib version as of September 29th 2023 is 3.4.4, a zip of this is available at 
misc/support/CAENVMELib-v3.4.4.tgz. The latest driver for the A2818 bridge (as of the writing of this 
document) is available at misc/support/A2818Drv-1.24.tar

Extract CAENVMELib-v3.4.4.tgz to ${DAQROOT}/CAENVMELib, then run the appropriate installation script within
${DAQROOT}/CAENVMELib/lib to install CAENVMELib. Note that even if you do not intend to run the DAQ for 
acquisition, you still need the CAEN libraries to build the code.

Note: It is worth mentioning that the drivers are set up a bit funny on the default DAQ machines.  We used 
the x86_64 version of the library, but ran the regular install script (which put the libraries in /usr/lib 
instead of /usr/lib64 as is technically proper).  The Makefiles for the DAQ reflect this, so if you put 
the CAEN libraries in the "correct" directory (/usr/lib64), you will need to also modify the default 
Makefiles in order to get the DAQ to build.

Inside the mnvdaq/ directory you will find a setup script, setupdaqenv.sh. It is a good idea to read this 
script  carefully before proceeding.  The "location" is set by a $LOCALE environment variable. For Mx2 
development and testing run setupMx2.sh to set your locale and run the setupdaqenv setup. You can also set
your own $LOCALE variable or mimic the directory structure of another $LOCALE and use that value for your
own $LOCALE.

As of 2010.July.14, the set-up scripts and run scripts are tuned in a fairly inflexible way for 
operation on Fermilab machines, set by hostnames in the options/ directory set of Make.options files.  
For set-up in a different environment, the default scripts will need to be edited carefully.

As of 2013.March.19, the different configuration options files are out of date. Basically we only build the 
DAQ one way right now, with updates to the build options required to get things working on the nearline 
monitoring stations. 

Once you have configured your setup scripts, build the DAQ with the following steps:

1) Go to $DAQROOT/

2) source setupdaqenv.sh. You will likely want to source the included setup.sh first if you want to 
  use the more modern version of ET.

3) Now build ET.  Go to the et_16.5 directory and follow the build instructions in README.md
  Either SCons or CMake can be used.
  E.g to use cmake: type
  `mkdir build && cd build && cmake –DCMAKE_BUILD_TYPE=Release ..
  make`

4) MINERvA DAQ uses the following set of ports:
	1090 : Queen-Soldier port (on Soldier)
	1098 : Run Control
	1110-1113, 1120-1123: Worker-Solider synchronization ports.
	1201-1250 : et port (on Queen)
It is a good idea to configure your firewall such that these ports are kept open. 

5) Check ${ET_LIBROOT}/lib and make sure you have libet.a, libet_remote.so, and libet.so.

6) Be sure that version of sqlite is what is in your $PATH and $LD_LIBRARY_PATH.
  (setupdaqenv.sh does this) 

7) Return to $DAQROOT and run the compiler.sh script. (Make sure you have the right 
  files in the options directory.)

8) Check ${DAQROOT}/bin/ for 
	event_builder
	minervadaq
  tests
And check ${DAQROOT}/lib/ for 
	libevent_structure.so
	libhardware.so
	libminerva_workers.so

9) If you are missing any of these, read the Makefile and try building each package one at a time and check 
for errors.  Most likely, an environment variable has been incorrectly set.

10) Finally, build the doxygen documentation:
  doxygen Doxyfile

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note about data sizes!  If you run the DAQ on an OS besides 64-bit SLF5.3, be sure to check the data sizes 
your compiler uses for variables.  The code currently assumes the following (check with sizeOfTest in the 
misc/ folder):

size of unsigned char      = 1
size of unsigned short int = 2
size of unsigned int       = 4
size of unsigned long      = 8
size of unsigned long long = 8

(Size is shown in bytes.)

