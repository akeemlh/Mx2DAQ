#ifndef ADCFrame_cpp
#define ADCFrame_cpp
/*! \file ADCFrame.cpp
*/

#include <iomanip>
#include "ADCFrame.h"

log4cpp::Category& ADCFrameLog = log4cpp::Category::getInstance(std::string("ADCFrame"));

//----------------------------------------------------
/*! 
  \param a The address (number) of the FEB.
  \param theChannelAddress The VME address of the handling VMECommunicator.
  \param theCrateNumber The VME crate ID.
  \param f The "RAM Function" which describes the hit of number to be read off
  */
ADCFrame::ADCFrame(
        FrameTypes::FEBAddresses a,
        unsigned int theChannelAddress, 
        int theCrateNumber,
        FrameTypes::RAMFunctionsHit f) : LVDSFrame()
{
  using namespace FrameTypes;

  febNumber[0]     = (unsigned char)a; 
  channelAddress   = theChannelAddress;
  crateNumber      = theCrateNumber;
  Devices dev      = RAM;               // device to be addressed
  Broadcasts broad = None;              // we don't broadcast
  Directions dir   = MasterToSlave;     // ALL outgoing messages are master-to-slave
  MakeDeviceFrameTransmit(dev, broad, dir, (unsigned int)f, (unsigned int)febNumber[0]); 

#ifndef GOFAST
  ADCFrameLog.setPriority(log4cpp::Priority::DEBUG);  
#else
  ADCFrameLog.setPriority(log4cpp::Priority::INFO);  
#endif
  ADCFrameLog.debugStream() << "Made ADCFrame " << f << " for FEB " << a; 
}


//----------------------------------------------------
/*!
  MakeMessage is the local implimentation of a virtual function of the same
  name inherited from LVDSFrame.  This function bit-packs the data into an OUTGOING
  message from values set using the get/set functions assigned to this class.
  */
void ADCFrame::MakeMessage() 
{
  if (NULL != outgoingMessage) this->DeleteOutgoingMessage();
  outgoingMessage = new unsigned char [this->GetOutgoingMessageLength()];
  for (unsigned int i = 0; i < MinervaDAQSizes::FrameHeaderLengthOutgoing; ++i) {
    outgoingMessage[i]=frameHeader[i];
  }
  for (unsigned int i = MinervaDAQSizes::FrameHeaderLengthOutgoing; i < this->GetOutgoingMessageLength(); ++i) {
    outgoingMessage[i]=0;
  }
#ifndef GOFAST
  ADCFrameLog.debugStream() << "Made ADCFrame Message";
#endif
}

//----------------------------------------------------
/*! 
  Based on C. Gingu's ParseInpFrameAsAnaBRAM function (from the FermiDAQ).
  Decode the input frame data as Hit data type from all Analog BRAMs.
  */
void ADCFrame::DecodeRegisterValues()
{	
  // Check to see if the frame is right length...
  unsigned short ml = (receivedMessage[FrameTypes::ResponseLength1]) | 
    (receivedMessage[FrameTypes::ResponseLength0] << 8);
  if ( ml == 0 ) {
    std::string errstring = "Can't parse an empty InpFrame in ADCFrame";
    ADCFrameLog.fatalStream() << errstring;
    FrameThrow( errstring ); 
  }
  if ( ml != MinervaDAQSizes::ADCFrameMaxSize) { 
    std::string errstring = "ADCFrame length mismatch";
    ADCFrameLog.fatalStream() << errstring;
    ADCFrameLog.fatalStream() << "  Message Length = " << ml;
    ADCFrameLog.fatalStream() << "  Expected       = " << MinervaDAQSizes::ADCFrameMaxSize;
    this->printReceivedMessageToLog(); 
    FrameThrow( errstring ); 
  }
  // Check that the dummy byte is zero...
  if ( receivedMessage[FrameTypes::Data] != 0 ) {
    std::string errstring = "Dummy byte is non-zero in ADCFrame";
    ADCFrameLog.fatalStream() << errstring;
    FrameThrow( errstring ); 
  }

  // Show header in 16-bit word format...
  int hword = 0;
  for (int i=0; i<(int)FrameTypes::Data; i+=2) {
    unsigned short int val = receivedMessage[i+1] | (receivedMessage[i]<< 8) ;
    ADCFrameLog.debug("Header Word %08d = %08d (%04X)", hword, val, val);
    hword++;
  }

  // The number of bytes per row is a function of firmware!!
  // v90+ uses 2 bytes per row, older firmware used 4.
  // We assume the firmware version is v90+ now.
  int bytes_per_row = 2; 

  // First, show the adc in simple sequential order...
  int AmplVal     = 0;
  int ChIndx      = 0;
  int TripIndx    = 0;
  int NTimeAmplCh = MinervaDAQSizes::nChannelsPerTrip; 
  int nrows       = 215; //really 216, but starting just past row 1...
  int start_byte  = 21;  // was 15? - this is an *index*
  int max_show    = start_byte + bytes_per_row * nrows; 
  for (int i = start_byte; i <= max_show; i += bytes_per_row/*4*/) {
    AmplVal = ((receivedMessage[i] << 8) + receivedMessage[i - 1]);
    // Show.  Recall that no *hit index* is shown because each analog bank *is* a "hit" index.
    ADCFrameLog.debug("ChIndx = %d, TripIndx = %d", ChIndx,TripIndx);
    // Apply "data mask" (pick out 12 bit adc) and shift two bits to the right (lower).
    ADCFrameLog.debug("  Masked AmplVal = %d", (AmplVal & 0x3FFC)>>2); // 0x3FFC = 0011 1111 1111 1100, 12-bit adc 
    ChIndx++;
    if (ChIndx == NTimeAmplCh) { TripIndx++; ChIndx = 0; }
  } //end for - show frame sequential

  // Show the adc keyed by pixel value...
  int lowMap[] = {26,34,25,33,24,32,23,31,22,30,21,29,20,28,19,27,
    11,3,12,4,13,5,14,6,15,7,16,8,17,9,18,10,
    11,3,12,4,13,5,14,6,15,7,16,8,17,9,18,10,
    26,34,25,33,24,32,23,31,22,30,21,29,20,28,19,27};

  for (unsigned int pixel = 0; pixel < MinervaDAQSizes::nPixelsPerFEB; pixel++) {
    // locate the pixel
    int headerLength = 20; // number of bytes 8 for the Minerva Frame & 10 for the Device & 2 blanks?
    int side = pixel / (MinervaDAQSizes::nPixelsPerSide);
    int hiMedEvenOdd = (pixel / (MinervaDAQSizes::nPixelsPerSide / 2)) % 2;
    int hiMedTrip = 2 * side + hiMedEvenOdd;  // 0...3
    int loTrip = MinervaDAQSizes::nHiMedTripsPerFEB + side;    // 4...5
    int hiChannel = MinervaDAQSizes::nSkipChannelsPerTrip + (pixel % MinervaDAQSizes::nPixelsPerTrip);
    int medChannel = hiChannel + MinervaDAQSizes::nPixelsPerTrip;
    int loChannel = lowMap[pixel];
    ADCFrameLog.debug("Pixel = %d, HiMedTrip = %d, hiChannel = %d, medChannel = %d, loTrip = %d, lowChannel = %d",
        pixel, hiMedTrip, hiChannel, medChannel, loTrip, loChannel);
    // Calculate the offsets (in bytes_per_row increments).
    int hiOffset = hiMedTrip * MinervaDAQSizes::nChannelsPerTrip + hiChannel;
    int medOffset = hiMedTrip * MinervaDAQSizes::nChannelsPerTrip + medChannel;
    int loOffset = loTrip * MinervaDAQSizes::nChannelsPerTrip + loChannel;
    ADCFrameLog.debug("hiOffset = %d, medOffset = %d, loOffset = %d", hiOffset, medOffset, loOffset);
    unsigned short int dataMask = 0x3FFC;
    ADCFrameLog.debug("Qhi = %d, Qmed = %d, Qlo = %d", 
        ( ( ( (receivedMessage[headerLength + bytes_per_row*hiOffset + 1] << 8) | receivedMessage[headerLength + bytes_per_row*hiOffset] ) 
            & dataMask ) >> 2),
        ( ( ( (receivedMessage[headerLength + bytes_per_row*medOffset + 1] << 8) | receivedMessage[headerLength + bytes_per_row*medOffset] ) 
            & dataMask ) >> 2),
        ( ( ( (receivedMessage[headerLength + bytes_per_row*loOffset + 1] << 8) | receivedMessage[headerLength + bytes_per_row*loOffset] ) 
            & dataMask ) >> 2)
        );
  }                
}

#endif

