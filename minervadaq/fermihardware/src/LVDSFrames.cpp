#ifndef LVDSFrames_cpp
#define LVDSFrames_cpp

#include <iomanip>
#include "LVDSFrames.h"
#include "exit_codes.h"

/*********************************************************************************
 * Class for creating FPGA Frame header objects for use with the 
 * MINERvA data acquisition system and associated software projects.
 *
 * Elaine Schulte, Rutgers University
 * Gabriel Perdue, The University of Rochester
 **********************************************************************************/

/* const int LVDSFrames::MinHeaderLength=9; // renamed: FrameHeaderLengthOutgoing */

log4cpp::Category& lvdsLog = log4cpp::Category::getInstance(std::string("frames"));

//------------------------------------------
LVDSFrames::LVDSFrames() 
{ 
  FrameID[0] = 0x00; 
  FrameID[1] = 0x00; 
  lvdsLog.setPriority(log4cpp::Priority::DEBUG);
}

//------------------------------------------
void LVDSFrames::MakeDeviceFrameTransmit( Devices dev, Broadcasts b, Directions d, 
    unsigned int f, unsigned int feb ) 
{
  /*! \fn********************************************************************************
   * a function which makes up an FPGA frame for transmitting information from
   * the data acquisition routines to the FPGA on the front end board (FEB) and
   * on to the requested device.
   *
   * Inputs:
   *
   * \param dev:  The device to which the message is destined
   * \param b: whether or not this is a broadcast request
   * \param d: the direction of the message:  either master-to-slave (true for transmit) or
   *    slave-to-master (receive)
   * \param f: the device function.  This is specific to the device (dev) receiving the message
   * \param feb: the number of the FEB to which this frame is destined
   *********************************************************************************/

  broadcastCommand[0] = (unsigned char)b;
  messageDirection[0] = (unsigned char)d;
  targetDevice[0]     = (unsigned char)dev; 
  deviceFunction[0]   = (unsigned char)f;
  febNumber[0]        = (unsigned char)feb;

  MakeHeader();
}

//------------------------------------------
void LVDSFrames::MakeHeader() 
{
  /*! \fn********************************************************************************
   * a function which packs outgoing frame header data for transmitting information from
   * the data acquisition routines to the FPGA on the front end board (FEB) and
   * on to the requested device.
   *********************************************************************************/

  /* we've done all the conversion & stuff so we can make up the frame header now! */
  /* word 1: the broadcast direction, command, and feb number */
  frameHeader[0] = (messageDirection[0] & 0x80 ); // The direction bit is in bit 7 of word 1
  frameHeader[0] |= (broadcastCommand[0] & 0xF0); // The broadcast command is in bits 4-6
  frameHeader[0] |= (febNumber[0] & 0x0F);        // The feb number is bits 0-3

  /* word 2:  target device & its function */
  frameHeader[1] = (targetDevice[0] & 0xF0);     // The target device is in bits 4-7
  frameHeader[1] |= (deviceFunction[0] & 0x0F);  // The function is in bits 0-3

  /* word 3:  reserved for response information */
  frameHeader[2] = 0x00; //initialize to null

  /* word 4 & 5:  frame ID (whatever that does) */
  frameHeader[3] = FrameID[0]; frameHeader[4] = FrameID[1];

  /* words 5 - 8 are reserved for response information */
  frameHeader[5] = frameHeader[6] = frameHeader[7] = frameHeader[8] = 0x00; //initialize them to null
}

//------------------------------------------
// Each class which inherits frames makes its own messages!
void LVDSFrames::MakeMessage() { std::cout << "Hi Elaine!" << std::endl; } 

//------------------------------------------
bool LVDSFrames::CheckForErrors() 
{
  /*! \fn bool LVDSFrames::CheckForErrors()
   * Check incoming frame header data for errors.
   */
  bool errors[10] = {false};
  bool error = false; //initialize error 

  ResponseWords word; //which word of the message do we check
  ResponseFlags flag; //what status bits are we checking

  /* we are looking to see if any of the various bits are set */
  unsigned char messageLength[2];
  word = ResponseLength0;
  messageLength[0] = message[word];
  word = ResponseLength1;
  messageLength[1] = message[word];

  word = FrameStart; flag = Direction; //check direction
  errors[0] = !(message[word] & flag); 
  if (errors[0]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: Direction: "<<errors[0];
  }

  word = DeviceStatus; flag = DeviceOK; //check status
  errors[1] = !(message[word] & flag);
  if (errors[1]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: DeviceOK: "<<errors[1];
  }

  word = DeviceStatus; flag = FunctionOK; //check execution status
  errors[2] = !(message[word]&flag);
  if (errors[2]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: FunctionOK: "<<errors[2];
  }

  word = FrameStatus; flag = CRCOK; //check CRC error bit
  errors[3] = !(message[word] & flag);
  if (errors[3]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: CRCOK: "<<errors[3];
  }

  word = FrameStatus; flag = EndHeader; //message ended properly
  errors[4] = !(message[word] & flag);
  if (errors[4]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: EndHeader: "<<errors[4];
  }

  word = FrameStatus; flag = MaxLen; //message exceeded maximum message length
  errors[5] = (message[word] & flag);
  if (errors[5]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: MaxLen: "<<errors[5];
  }

  word = FrameStatus; flag = SecondStart;
  errors[6] = (message[word] & flag);
  if (errors[6]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: SecondStart: "<<errors[6];
  }

  word = FrameStatus; flag = NAHeader;
  errors[7] = (message[word] & flag);
  if (errors[7]) {
    error = true; 
    lvdsLog.errorStream()<<"CheckForErrors: NAHeader: "<<errors[7];
  }

  return error; // true if *any* error was found!
}

//------------------------------------------
void LVDSFrames::DecodeHeader() 
{
  /*! \fn 
   * extract device information from the FPGA header sent back from
   * the electronics by a read request.
   *
   * note:  these should be decoded using the enumerated types in 
   * FrameTypes.h 
   */
#if DEBUG_VERBOSE
  std::cout << " Entering LVDSFrames::DecodeHeader..." << std::endl;
#endif
  ResponseWords word;

  word = FrameStart; 
  febNumber[0] = (message[word]&0x0F); //extract the feb board number from which this message came
#if DEBUG_VERBOSE
  std::cout << "  message at framestart: " << (int)message[word] << std::endl;
#endif
  broadcastCommand[0] = (message[word]&0xF0); //extract the broadcast command
  messageDirection[0] = (message[word]&0x80); //get the message direction
#if DEBUG_VERBOSE
  std::cout << "  direction: " << (int)(message[word]&0x80) << std::endl;
#endif
  word = DeviceStatus;
  deviceFunction[0] = (message[word]&0x0F); // extract the device function executed 
  targetDevice[0]   = (message[word]&0xF0); // extract the device which responded
}

//------------------------------------------
void LVDSFrames::printMessageBufferToLog( int buffersize )
{
  lvdsLog.debugStream() << "Printing message buffer of size = " << buffersize;
  if (buffersize > 0) 
    for (int i=0; i<buffersize; i+=2 ) {
      int j = i + 1;
      lvdsLog.debugStream() 
        << std::setfill('0') << std::setw( 2 ) << std::hex << (int)message[i] << " " 
        << std::setfill('0') << std::setw( 2 ) << std::hex << (int)message[j] << " " 
        << "\t" 
        << std::setfill('0') << std::setw( 4 ) << std::dec << i << " " 
        << std::setfill('0') << std::setw( 4 ) << std::dec << j;

    }
}
#endif
