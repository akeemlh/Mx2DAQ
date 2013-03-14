#ifndef DAQWorker_cxx
#define DAQWorker_cxx

#include <fstream>

#include "EventHandler.h"
#include "DAQHeader.h"
#include "DAQWorker.h"

#include "exit_codes.h"

log4cpp::Category& daqWorker = 
log4cpp::Category::getInstance(std::string("daqWorker"));

//---------------------------------------------------------
DAQWorker::DAQWorker( const DAQWorkerArgs* theArgs, 
    log4cpp::Priority::Value priority,
    bool *theStatus ) :
  args(theArgs),
  status(theStatus),
  declareEventsToET(false)
{
  daqWorker.setPriority(priority);

  daqWorker.infoStream() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  daqWorker.infoStream() << "Arguments to MINERvA DAQ Worker: ";
  daqWorker.infoStream() << "  Run Number             = " << args->runNumber;
  daqWorker.infoStream() << "  Subrun Number          = " << args->subRunNumber;
  daqWorker.infoStream() << "  Total Gates            = " << args->numberOfGates;
  daqWorker.infoStream() << "  Running Mode (encoded) = " << args->runMode;
  daqWorker.infoStream() << "  Detector (encoded)     = " << args->detector;
  daqWorker.infoStream() << "  DetectorConfiguration  = " << args->detectorConfigCode;
  daqWorker.infoStream() << "  LED Level (encoded)    = " << (int)args->ledLevel;
  daqWorker.infoStream() << "  LED Group (encoded)    = " << (int)args->ledGroup;
  daqWorker.infoStream() << "  ET Filename            = " << args->etFileName;
  daqWorker.infoStream() << "  SAM Filename           = " << args->samFileName;
  daqWorker.infoStream() << "  LOG Filename           = " << args->logFileName;
  daqWorker.infoStream() << "  Configuration File     = " << args->hardwareConfigFileName;
  daqWorker.infoStream() << "  VME Card Init. Level   = " << args->hardwareInitLevel;	
  daqWorker.infoStream() << "  ET System Port         = " << args->networkPort;	
  daqWorker.infoStream() << "See Event/MinervaEvent/xml/DAQHeader.xml for codes.";
  daqWorker.infoStream() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

  readoutWorker = new ReadoutWorker( priority, theStatus, (bool)args->hardwareInitLevel );
  stateRecorder = new ReadoutStateRecorder( args, priority );
}

//---------------------------------------------------------
DAQWorker::~DAQWorker()
{
  delete readoutWorker;
  delete stateRecorder;
}

//---------------------------------------------------------
void DAQWorker::InitializeHardware()
{
  daqWorker.infoStream() << "Initializing Hardware for DAQWorker...";

  // Read in hardware config here. For now, hard code...

  readoutWorker->AddCrate(0);
  readoutWorker->GetVMECrateVector(0)->AddECROC( 2, 0, 5, 0, 0 );
  readoutWorker->GetVMECrateVector(0)->AddCRIM( 224 );
  readoutWorker->InitializeCrates( args->runMode );
}

//---------------------------------------------------------
int DAQWorker::SetUpET()  
{
  daqWorker.infoStream() << "Setting up ET...";
  declareEventsToET = true;

  et_openconfig  openconfig;

  // Configuring the ET system is the first thing we must do.
  et_open_config_init(&openconfig);

  // Set the remote host. We operate the DAQ exclusively 
  // in "remote" mode even when running on only one PC.
  et_open_config_setmode(openconfig, ET_HOST_AS_REMOTE);  

  // Set to the host machine name. 
  et_open_config_sethost(openconfig, (args->hostName).c_str());
  daqWorker.infoStream() << "Setting ET host to " << args->hostName;

  // Set direct connection.
  et_open_config_setcast(openconfig, ET_DIRECT);  // Remote mode only.

  // Set the server port.
  et_open_config_setserverport(openconfig, args->networkPort); // Remote mode only.
  daqWorker.infoStream() << "Set ET server port to " << args->networkPort; 

  // Open it.
  daqWorker.infoStream() << "Trying to open ET system...";   
  if (et_open(&sys_id, (args->etFileName).c_str(), openconfig) != ET_OK) {
    daqWorker.fatalStream() << "et_producer: et_open problems!";
    return EXIT_UNSPECIFIED_ERROR;
  }
  daqWorker.infoStream() << "...Opened ET system!";  

  // Clean up.
  et_open_config_destroy(openconfig);

  // Set the debug level for output (everything).
  et_system_setdebug(sys_id, ET_DEBUG_INFO);

  // Attach to GRANDCENTRAL station since we are producing events.
  int etattstat = et_station_attach(sys_id, ET_GRANDCENTRAL, &attach); 
  if (etattstat < 0) {
    daqWorker.fatalStream() << "et_producer: error in station attach!";
    daqWorker.fatalStream() << " error code = " << etattstat;
    return etattstat;
  } 
  daqWorker.infoStream() << "Successfully attached to GRANDCENTRAL Station.";        

  return 0;
}

//---------------------------------------------------------
bool DAQWorker::ContactEventBuilder( EventHandler *handler )
{
  daqWorker.infoStream() << "Contacting Event Builder...";
  if (!declareEventsToET) return true;

  unsigned short length = handler->dataLength;
  daqWorker.debugStream() << "EventHandler dataLength = " << length;

  while (et_alive(sys_id)) {
    daqWorker.debugStream() << "  ->ET is Alive!";
    et_event *pe;         
    EventHandler *pdata;  
    int status = et_event_new(sys_id, attach, &pe, ET_SLEEP, NULL,
        sizeof(struct EventHandler)); 
    if (status == ET_ERROR_DEAD) {
      daqWorker.crit("ET system is dead in ContactEventBuilder!");
      break;
    } else if (status == ET_ERROR_TIMEOUT) {
      daqWorker.crit("Got an ET timeout in ContactEventBuilder!");
      break;
    } else if (status == ET_ERROR_EMPTY) {
      daqWorker.crit("No ET events in ContactEventBuilder!");
      break;
    } else if (status == ET_ERROR_BUSY) {
      daqWorker.crit("ET Grandcentral is busy in ContactEventBuilder!");
      break;
    } else if (status == ET_ERROR_WAKEUP) {
      daqWorker.crit("ET wakeup error in ContactEventBuilder!");
      break;
    } else if ((status == ET_ERROR_WRITE) || (status == ET_ERROR_READ)) {
      daqWorker.crit("ET socket communication error in ContactEventBuilder!");
      break;
    } if (status != ET_OK) {
      daqWorker.fatal("ET et_producer: error in et_event_new in ContactEventBuilder!");
      return false;
    }
    // Put data into the event.
    if (status == ET_OK) {
      daqWorker.debugStream() << "Putting Event into ET System...";
      et_event_getdata(pe, (void **)&pdata); 
#ifndef GOFAST
      { 
        daqWorker.debugStream() << "-----------------------------------------------";
        daqWorker.debugStream() << "EventHandler_size: " << sizeof(struct EventHandler);
        daqWorker.debugStream() << "Finished Processing Event Data:";
        for (int index = 0; index < length; index++) {
          daqWorker.debug("     Data Byte %02d = 0x%02X",
              index,(unsigned int)handler->data[index]);
        }
      }
#endif
      // TODO : memmove?
      // Also TODO : statically sized EventHandler is typically far too big. Dynamic?
      memcpy(pdata, handler, sizeof(struct EventHandler));
      et_event_setlength(pe, sizeof(struct EventHandler));
    } 

    // Put the event into the ET system.
    status = et_event_put(sys_id, attach, pe); 
    if (status != ET_OK) {
      daqWorker.fatal("et_producer: put error in ContactEventBuilder!");
      return false;
    }
    if (!et_alive(sys_id)) {
      et_wait_for_alive(sys_id);
    }
    break; // Done processing the event. 
  } // while alive 

  daqWorker.debugStream() << "  Exiting ContactEventBuilder...";
  return true;
}

//---------------------------------------------------------
bool DAQWorker::CloseDownET()
{
  daqWorker.infoStream() << "Closing down ET...";
  if (!declareEventsToET) return true;

  if (et_station_detach(sys_id, attach) < 0) {
    daqWorker.fatal("et_producer: error in station detach\n");
    return false;
  }     

  return true;
}

//---------------------------------------------------------
void DAQWorker::TakeData()
{
  daqWorker.infoStream() << "Beginning Data Acquisition...";

  while ( BeginNextGate() ) {

    do {
      unsigned short blockSize = readoutWorker->GetNextDataBlockSize();  
      daqWorker.debugStream() << "Next data block size is: " << blockSize;
      std::tr1::shared_ptr<SequencerReadoutBlock> block = 
        readoutWorker->GetNextDataBlock( blockSize );
      if (declareEventsToET) {
        DeclareDataBlock<SequencerReadoutBlock>( block.get() );
      }
      else {
        DissolveDataBlock( block );
      }
    } while ( readoutWorker->MoveToNextChannel() );

    FinishGate();
    DeclareDAQHeaderToET();
  }

  daqWorker.infoStream() << "Finished Data Acquisition...";
}

//---------------------------------------------------------
bool DAQWorker::BeginNextGate()
{
  daqWorker.debugStream() << "DAQWorker::BeginNextGate...";
  daqWorker.debugStream() << "Continue Running Status = " << (*status);
  if (!(*status)) return false;

  unsigned long long triggerTime = 0;
  readoutWorker->ResetCurrentChannel();
  Triggers::TriggerType triggerType = stateRecorder->GetNextTriggerType();
  triggerTime = readoutWorker->Trigger( triggerType );
  stateRecorder->SetGateStartTime( triggerTime );

  return stateRecorder->BeginNextGate();
}

//---------------------------------------------------------
bool DAQWorker::FinishGate()
{
  daqWorker.debugStream() << "DAQWorker::FinishGate...";
  stateRecorder->SetGateFinishTime( readoutWorker->GetNowInMicrosec() );
  stateRecorder->SetMINOSSGATE( readoutWorker->GetMINOSSGATE() );

  return stateRecorder->FinishGate();
}

//---------------------------------------------------------
void DAQWorker::DissolveDataBlock( std::tr1::shared_ptr<SequencerReadoutBlock> block )
{
  daqWorker.debugStream() << "Got data, destructively processing into frames...";
  block->ProcessDataIntoFrames();
  while (block->FrameCount()) {
    LVDSFrame * frame = block->PopOffFrame();
    daqWorker.debugStream() << (*frame);
    frame->printReceivedMessageToLog();
    delete frame;
  }
}

//---------------------------------------------------------
void DAQWorker::DeclareDAQHeaderToET( HeaderData::BankType bankType )
{
  daqWorker.debugStream() << "Declaring Header to ET for bank type " << bankType;
  std::tr1::shared_ptr<DAQHeader> daqhead = stateRecorder->GetDAQHeader( bankType );
  DeclareDataBlock<DAQHeader>( daqhead.get() );
}

//---------------------------------------------------------
bool DAQWorker::SendSentinel()
{
  daqWorker.debugStream() << "Sending Sentinel Frame...";
  DeclareDAQHeaderToET( HeaderData::SentinelBank );
  return true;
}

//---------------------------------------------------------
template <class X> void DAQWorker::DeclareDataBlock( X *dataBlock )
{
  struct EventHandler * handler = NULL;
  handler = CreateEventHandler<X>( dataBlock );
  ContactEventBuilder( handler );
  DestroyEventHandler( handler );
}

//---------------------------------------------------------
template <class X> struct EventHandler * DAQWorker::CreateEventHandler( X *dataBlock )
{
  struct EventHandler * handler = (struct EventHandler *)malloc( sizeof(struct EventHandler) );
  assert( NULL != handler );

  handler->dataLength = dataBlock->GetDataLength();
  // If data ever becomes dynamic, we need to new it here.
  memcpy( handler->data, dataBlock->GetData(), handler->dataLength ); // dest, src, length

  return handler;
}

//---------------------------------------------------------
void DAQWorker::DestroyEventHandler( struct EventHandler * handler )
{
  assert( NULL != handler );
  // If data ever becomes dynamic, we need to free it here.
  free( handler );
}


#endif
