#ifndef daqmain_cxx
#define daqmain_cxx

#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

#include "log4cppHeaders.h"
#include "DAQWorker.h"
#include "DAQArgs.h"
#include "ReadoutTypes.h"
#include "exit_codes.h"
#include "daqmain.h"

// log4cpp message levels:
//	emergStream(), fatalStream(), alert..., crit..., error..., warn..., notice..., info..., debug...
log4cpp::Appender* baseAppender;
log4cpp::Category& rootCat = log4cpp::Category::getRoot();
log4cpp::Category& daqmain = log4cpp::Category::getInstance(std::string("daqmain"));

//---------------------------------------------------------------
//---------------------------------------------------------------
int main( int argc, char * argv[] ) 
{
  struct DAQWorkerArgs * args = DAQArgs::ParseArgs( argc, argv, "0" );

  baseAppender = new log4cpp::FileAppender("default", args->logFileName, false);
  baseAppender->setLayout(new log4cpp::BasicLayout());
  rootCat.addAppender(baseAppender);
  rootCat.setPriority(log4cpp::Priority::DEBUG);
  daqmain.setPriority(log4cpp::Priority::DEBUG);
  daqmain.infoStream() << "Starting MinervaDAQ...";

  continueRunning = true;
  DAQWorker * worker = new DAQWorker( args, log4cpp::Priority::DEBUG, &continueRunning );

  bool sentSentinel = false;
  int error = worker->SetUpET(); 
  if (0 == error) {
    worker->TakeData();
    if (worker->CloseDownET())
      daqmain.infoStream() << "Detached from ET station..."; 
    sentSentinel = worker->SendSentinel();
  }
  else {
    daqmain.fatalStream() << "Failed to establish ET connection!";
  }

  daqmain.infoStream() << "Finished MinervaDAQ...";

  log4cpp::Category::shutdown();
  delete worker;
  delete args;

  return (sentSentinel) ? EXIT_CLEAN_SENTINEL : EXIT_CLEAN_NOSENTINEL;
}

//---------------------------------------------------------------
void SetUpSigAction()
{
  // Set up the signal handler so we can always exit cleanly 

  struct sigaction quit_action;
  quit_action.sa_handler = quitsignal_handler;
  sigemptyset (&quit_action.sa_mask);
  // restart interrupted system calls instead of failing with EINTR
  quit_action.sa_flags = SA_RESTART;    

  sigaction(SIGINT,  &quit_action, NULL);
  sigaction(SIGTERM, &quit_action, NULL);
}

//---------------------------------------------------------------
void quitsignal_handler(int signum)
  /*! \fn void quitsignal_handler(int signum)
   *
   * Handles the SIGINT & SIGNUM signals (both of which should exit the process).
   */
{
  continueRunning = false;
}


#endif
