#ifndef EChannels_cpp
#define EChannels_cpp

#include <iomanip>

#include "RegisterWords.h"
#include "EChannels.h"
#include "exit_codes.h"

/*********************************************************************************
 * Class for creating CROC-E channel objects for use with the 
 * MINERvA data acquisition system and associated software projects.
 *
 * Gabriel Perdue, The University of Rochester
 **********************************************************************************/

log4cpp::Category& EChannelLog = log4cpp::Category::getInstance(std::string("EChannel"));

//----------------------------------------
EChannels::EChannels( unsigned int vmeAddress, unsigned int number, log4cpp::Appender* appender, Controller* controller ) : 
  VMECommunicator( vmeAddress, appender, controller )
{
	/*! \fn 
	 * constructor takes the following arguments:
	 * \param vmeAddress  :  The channel base address (already bit-shifted)
	 * \param number      :  The channel number (0-3)
   * \param *appender   :  Pointer to the log4cpp appender.
   * \param *controller :  Pointer to the VME 2718 Controller servicing this device.
	 */
	channelNumber        = number;       //the channel number (0-3 here, 1-4 is stenciled on the cards themselves)
	channelDirectAddress = this->address + EChannelOffset * (unsigned int)(channelNumber);
  echanAppender        = appender;
  if ( echanAppender == 0 ) {
    std::cout << "EChannel Log Appender is NULL!" << std::endl;
    exit(EXIT_CROC_UNSPECIFIED_ERROR);
  }
  EChannelLog.setPriority(log4cpp::Priority::DEBUG);  // ERROR?


  receiveMemoryAddress             = channelDirectAddress + (unsigned int)ECROCReceiveMemory;
  sendMemoryAddress                = channelDirectAddress + (unsigned int)ECROCSendMemory;
  framePointersMemoryAddress       = channelDirectAddress + (unsigned int)ECROCFramePointersMemory;
  configurationAddress             = channelDirectAddress + (unsigned int)ECROCConfiguration;
  commandAddress                   = channelDirectAddress + (unsigned int)ECROCCommand;
  eventCounterAddress              = channelDirectAddress + (unsigned int)ECROCEventCounter;
  framesCounterAndLoopDelayAddress = channelDirectAddress + (unsigned int)ECROCFramesCounterAndLoopDelay;
  frameStatusAddress               = channelDirectAddress + (unsigned int)ECROCFrameStatus;
  txRxStatusAddress                = channelDirectAddress + (unsigned int)ECROCTxRxStatus;
  receiveMemoryPointerAddress      = channelDirectAddress + (unsigned int)ECROCReceiveMemoryPointer;

  dpmPointer    = 0;     // start pointing at zero

  EChannelLog.setPriority(log4cpp::Priority::DEBUG);
}

//----------------------------------------
EChannels::~EChannels() 
{
  for (std::vector<FEB*>::iterator p=FEBsVector.begin(); p!=FEBsVector.end(); p++) delete (*p);
  FEBsVector.clear();
}

//----------------------------------------
unsigned int EChannels::GetChannelNumber() 
{
  return channelNumber;
}

//----------------------------------------
unsigned int EChannels::GetParentECROCAddress() 
{
  return this->address;
}

//----------------------------------------
unsigned int EChannels::GetDirectAddress() 
{
  return channelDirectAddress;
}

//----------------------------------------
unsigned int EChannels::GetReceiveMemoryAddress()
{
  return receiveMemoryAddress;
}

//----------------------------------------
unsigned int EChannels::GetSendMemoryAddress()
{
  return sendMemoryAddress;
}

//----------------------------------------
unsigned int EChannels::GetFramePointersMemoryAddress() 
{
  return framePointersMemoryAddress;
}

//----------------------------------------
unsigned int EChannels::GetConfigurationAddress() 
{
  return configurationAddress;
}

//----------------------------------------
unsigned int EChannels::GetCommandAddress() 
{
  return commandAddress;
}

//----------------------------------------
unsigned int EChannels::GetEventCounterAddress() 
{
  return eventCounterAddress;
}

//----------------------------------------
unsigned int EChannels::GetFramesCounterAndLoopDelayAddress() 
{
  return framesCounterAndLoopDelayAddress;
}

//----------------------------------------
unsigned int EChannels::GetFrameStatusAddress() 
{
  return frameStatusAddress;
}

//----------------------------------------
unsigned int EChannels::GetTxRxStatusAddress() 
{
  return txRxStatusAddress;
}

//----------------------------------------
unsigned int EChannels::GetReceiveMemoryPointerAddress() 
{
  return receiveMemoryPointerAddress;
}

//----------------------------------------
unsigned int EChannels::GetDPMPointer() 
{
  return dpmPointer;
}

//----------------------------------------
void EChannels::SetDPMPointer( unsigned short pointer ) 
{
  dpmPointer = pointer;
}

//----------------------------------------
unsigned char* EChannels::GetBuffer() 
{
  return buffer;
}

//----------------------------------------
void EChannels::SetBuffer( unsigned char *data ) 
{
  /*! \fn 
   * Puts data into the data buffer assigned to this channel.
   * \param data the data buffer
   */

  EChannelLog.debugStream() << "     Setting Buffer for Chain " << this->GetChannelNumber();
  buffer = new unsigned char [(int)dpmPointer];
  for (int i=0;i<(int)dpmPointer;i++) {
    buffer[i]=data[i];
    EChannelLog.debugStream() << "       SetBuffer: buffer[" 
      << std::setfill('0') << std::setw( 3 ) << i  << "] = 0x" 
      << std::setfill('0') << std::setw( 2 ) << std::hex << buffer[i];
  }
  EChannelLog.debugStream() << "     Done with SetBuffer... Returning...";
  return; 
}

//----------------------------------------
void EChannels::DeleteBuffer() 
{
  delete [] buffer;
}

//----------------------------------------
int EChannels::DecodeStatusMessage() 
{
  /* TODO: Re-implement this correctly for new channels. */
  return 0;
}

//----------------------------------------
int EChannels::CheckHeaderErrors(int dataLength)
{                  
  /* TODO: Re-implement this correctly for new channels. */
  return 0;
}

//----------------------------------------
void EChannels::SetupNFEBs( int nFEBs )
{
  EChannelLog.debugStream() << "SetupNFEBs for " << nFEBs << " FEBs...";
  if ( ( nFEBs < 0 ) || (nFEBs > 10) ) {
    EChannelLog.fatalStream() << "Cannot have less than 0 or more than 10 FEBs on a Channel!";
    exit(EXIT_CONFIG_ERROR);
  }
  for ( int i=1; i<=nFEBs; ++i ) {
    EChannelLog.debugStream() << "Setting up FEB " << i << " ...";
    FEB *feb = new FEB( (febAddresses)i, echanAppender );
    if ( isAvailable( feb ) ) {
      FEBsVector.push_back( feb );
    } else {
      EChannelLog.fatalStream() << "Requested FEB with address " << i << " is not avialable!";
      exit(EXIT_CONFIG_ERROR);
    }
  }
}

//----------------------------------------
std::vector<FEB*>* EChannels::GetFEBVector() 
{
  return &FEBsVector;
}

//----------------------------------------
FEB* EChannels::GetFEBVector( int index /* should always equal FEB address */ ) 
{
  // TODO: add check for null here? or too slow? (i.e., live fast and dangerouss)
  // Check that address = index?
  // Maybe add a precompiler flag, SAFE_MODE or something, that makes these checks, 
  // but we wouldn't use it when optimizing for speed...
  return FEBsVector[index];
}

//----------------------------------------
bool EChannels::isAvailable( FEB* feb )
{
  EChannelLog.debugStream() << "isAvailable FEB with class address = " << feb->GetBoardNumber();
  bool available = false;
  this->ClearAndResetStatusRegister();

  unsigned short dataLength = this->ReadFPGAProgrammingRegistersToMemory( feb );
  unsigned char* dataBuffer = this->ReadMemory( dataLength ); 

  feb->message = dataBuffer;
  feb->DecodeRegisterValues(dataLength);
  /* feb->ShowValues(); */
  EChannelLog.debugStream() << "Decoded FEB address = " << (int)feb->GetBoardID();
  // Check to see if the readonly BoardID == Class value;
  if( (int)feb->GetBoardID() == feb->GetBoardNumber() ) available = true;

  feb->message = 0;
  delete [] dataBuffer;

  EChannelLog.debugStream() << "FEB " << feb->GetBoardNumber() << " isAvailable = " << available;
  return available;
}

//----------------------------------------
void EChannels::ClearAndResetStatusRegister()
{
  EChannelLog.debugStream() << "Command Address        = 0x" 
    << std::setfill('0') << std::setw( 8 ) << std::hex 
    << commandAddress;
  EChannelLog.debugStream() << "Address Modifier       = " 
    << (CVAddressModifier)addressModifier;
  EChannelLog.debugStream() << "Data Width (Registers) = " << dataWidthReg;

  int error = WriteCycle( 2,  RegisterWords::channelReset,  commandAddress, addressModifier, dataWidthReg ); 
  if( error ) exitIfError( error, "Failure clearing the status!");
}

//----------------------------------------
unsigned short EChannels::ReadFrameStatusRegister()
{
  unsigned char receivedMessage[] = {0x0,0x0};
  EChannelLog.debugStream() << "Frame Status Address = 0x" 
    << std::setfill('0') << std::setw( 8 ) << std::hex 
    << frameStatusAddress;

  int error = ReadCycle(receivedMessage, frameStatusAddress, addressModifier, dataWidthReg); 
  if( error ) exitIfError( error, "Failure reading Frame Status!");

  return ( (receivedMessage[1] << 8) | receivedMessage[0] );
}

//----------------------------------------
unsigned short EChannels::ReadTxRxStatusRegister()
{
  unsigned char receivedMessage[] = {0x0,0x0};
  EChannelLog.debugStream() << "Tx/Rx Status Address = 0x" 
    << std::setfill('0') << std::setw( 8 ) << std::hex 
    << txRxStatusAddress;

  int error = ReadCycle(receivedMessage, txRxStatusAddress, addressModifier, dataWidthReg); 
  if( error ) exitIfError( error, "Failure reading Tx/Rx Status!");

  return ( (receivedMessage[1] << 8) | receivedMessage[0] );
}


//----------------------------------------
void EChannels::SendMessage()
{
  //#ifndef GOFAST
  //#endif
  EChannelLog.debugStream() << "SendMessage Address = 0x" 
    << std::setfill('0') << std::setw( 8 ) << std::hex << commandAddress 
    << "; Message = 0x" << std::hex << RegisterWords::sendMessage[1] << RegisterWords::sendMessage[0];
  int error = WriteCycle( 2, RegisterWords::sendMessage, commandAddress, addressModifier, dataWidthReg); 
  if( error ) exitIfError( error, "Failure writing to CROC Send Message Register!"); 
}

//----------------------------------------
void EChannels::WaitForMessageReceived()
{
  unsigned short status = 0;
  do {
    status = this->ReadFrameStatusRegister();
  } while ( 
      (status & 0x1000) &&   // TODO: MAGIC NUMBERS MUST DIE
      !(status & 0x8000) &&  // send memory full
      !(status & 0x0080) &&  // receive memory full
      !(status & 0x0010) &&  // frame received
      !(status & 0x0008) &&  // timeout error
      !(status & 0x0004) &&  // crc error
      !(status & 0x0002)     // header error
      );
  // TODO decodeStatus(status); // maybe use this in the while also?
  EChannelLog.debugStream() << "Message was received with status = 0x" 
    << std::setfill('0') << std::setw( 4 ) << std::hex << status;
}

//----------------------------------------
unsigned short EChannels::ReadDPMPointer()
{
  unsigned short receiveMemoryPointer = 0;
  unsigned char pointer[] = {0x0,0x0};

  EChannelLog.debugStream() << "Read ReceiveMemoryPointer Address = 0x" << std::hex << address;
  int error = ReadCycle( pointer, receiveMemoryPointerAddress, addressModifier, dataWidthReg); 
  if( error ) exitIfError( error, "Failure reading the Receive Memory Pointer!"); 
  receiveMemoryPointer = pointer[1]<<0x08 | pointer[0];
  EChannelLog.debugStream() << "Pointer Length = " << receiveMemoryPointer;

  return receiveMemoryPointer;
}

//----------------------------------------
unsigned char* EChannels::ReadMemory( unsigned short dataLength )
{
  // -> possible shenanigans! -> 
  if (dataLength%2) {dataLength -= 1;} else {dataLength -= 2;} //must be even  //TODO: should this be in ReadDPMPointer?
  EChannelLog.debugStream() << "ReadMemory for buffer size = " << dataLength;
  unsigned char *dataBuffer = new unsigned char [dataLength];

  int error = ReadBLT( dataBuffer, dataLength, receiveMemoryAddress, bltAddressModifier, dataWidthSwapped );
  if( error ) exitIfError( error, "Error in BLT ReadCycle!");

  return dataBuffer;
}


void EChannels::WriteFPGAProgrammingRegistersReadFrameToMemory( FEB *feb )
{
  // Note: this function does not send the message! It only write the message to the CROC memory.
  Devices dev     = FPGA;
  Broadcasts b    = None;
  Directions d    = MasterToSlave;
  FPGAFunctions f = Read;
  feb->MakeDeviceFrameTransmit( dev, b, d, f, (unsigned int)feb->GetBoardNumber() );
  feb->MakeMessage(); 

  EChannelLog.debugStream() << "Send Memory Address   = 0x" << std::hex << sendMemoryAddress;
  EChannelLog.debugStream() << "Message Length        = " << feb->GetOutgoingMessageLength();
  int error = WriteCycle( feb->GetOutgoingMessageLength(), feb->GetOutgoingMessage(), 
      sendMemoryAddress, addressModifier, dataWidthSwappedReg );
  if( error ) exitIfError( error, "Failure writing to CROC FIFO!"); 
  feb->DeleteOutgoingMessage(); 
}


unsigned short EChannels::ReadFPGAProgrammingRegistersToMemory( FEB *feb )
{
  // Note: this function does not retrieve the data from memory! It only loads it and reads the pointer.
  this->ClearAndResetStatusRegister();
  this->WriteFPGAProgrammingRegistersReadFrameToMemory( feb );
  this->SendMessage();
  this->WaitForMessageReceived();
  unsigned short dataLength = this->ReadDPMPointer();

  return dataLength;
}




#endif