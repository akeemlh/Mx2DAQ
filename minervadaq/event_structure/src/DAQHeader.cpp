#ifndef DAQHeader_cxx
#define DAQHeader_cxx
/*! \file DAQHeader.cpp
*/

#include "log4cppHeaders.h"
#include "DAQHeader.h"

log4cpp::Category& daqevt = log4cpp::Category::getInstance(std::string("daqevt"));

//----------------------------------------------------------------
//! The default ctor constructs a Sentinel frame.
DAQHeader::DAQHeader(FrameHeader *header)
{
#ifndef GOFAST
  daqevt.setPriority(log4cpp::Priority::DEBUG);
  daqevt.debugStream() << "->Entering DAQHeader::DAQHeader... Building a Sentinel Frame.";
#else
  daqevt.setPriority(log4cpp::Priority::INFO);
#endif

  dataLength = daqHeaderSize;
  data = new unsigned char[dataLength];

  for (int i = 0; i < dataLength; i++) {
    data[i] = 0;
  }
  const unsigned short *tmpDAQHeader = header->GetBankHeader();
  int buffer_index = 0; 
  for (int i = 0; i < 4 ;i++) {
    data[buffer_index] = tmpDAQHeader[i]&0xFF;
    buffer_index++;
    data[buffer_index] = (tmpDAQHeader[i]&0xFF00)>>0x08;
    buffer_index++;
  }
}

//----------------------------------------------------------------
DAQHeader::DAQHeader(unsigned char det, unsigned short int config, int run, int sub_run, 
    unsigned short int trig, unsigned char ledLevel, unsigned char ledGroup, 
    unsigned long long g_gate, unsigned int gate, unsigned long long trig_time, 
    unsigned short int error, unsigned int minos, unsigned int read_time, FrameHeader *header, 
    unsigned short int nADCFrames, unsigned short int nDiscFrames,
    unsigned short int nFPGAFrames)
{
#ifndef GOFAST
  daqevt.setPriority(log4cpp::Priority::DEBUG);
  daqevt.debugStream() << "->Entering DAQHeader::DAQHeader... Building a DAQ Header.";
#else
  daqevt.setPriority(log4cpp::Priority::INFO);
#endif

  unsigned int event_info_block[12]; 
  dataLength = daqHeaderSize;
  data = new unsigned char[dataLength];

#ifndef GOFAST
  daqevt.debugStream() << " det       = " << (int)det;
  daqevt.debugStream() << " config    = " << config;
  daqevt.debugStream() << " run       = " << run;
  daqevt.debugStream() << " sub_run   = " << sub_run;
  daqevt.debugStream() << " trig      = " << trig;
  daqevt.debugStream() << " ledLevel  = " << (int)ledLevel;
  daqevt.debugStream() << " ledGroup  = " << (int)ledGroup;
  daqevt.debugStream() << " g_gate    = " << g_gate;
  daqevt.debugStream() << " gate      = " << gate;
  daqevt.debugStream() << " trig_time = " << trig_time;
  daqevt.debugStream() << " error     = " << error;
  daqevt.debugStream() << " minos     = " << minos;
  daqevt.debugStream() << " read_time = " << read_time;
#endif
  event_info_block[0] = det & 0xFF;
  event_info_block[0] |= 0 <<0x08; //a reserved byte
  event_info_block[0] |= (config & 0xFFFF)<<0x10;
  event_info_block[1] = run & 0xFFFFFFFF;
  event_info_block[2] = sub_run & 0xFFFFFFFF;
  event_info_block[3] = trig & 0xFF;
  event_info_block[3] |= ( ((int)ledLevel & 0x3) << 8 );
  event_info_block[3] |= ( ((int)ledGroup & 0xF8) << 8 );
  event_info_block[3] |= ( (nFPGAFrames & 0xFFFF) << 16 );
  event_info_block[4] = g_gate & 0xFFFFFFFF;            // the "global gate" least sig int 
  event_info_block[5] = (g_gate>>32) & 0xFFFFFFFF;      // the "global gate" most sig int
  event_info_block[6] = gate & 0xFFFFFFFF;              // the gate number least sig int 
  event_info_block[7] = (nDiscFrames << 16) | (nADCFrames);
  event_info_block[8] = trig_time & 0xFFFFFFFF;         // the gate time least sig int
  event_info_block[9] = (trig_time>>32) & 0xFFFFFFFF;   // the gate time most sig int
  event_info_block[10] = ( (error & 0x7) << 4 ) & 0xFF; // the error bits 4-7
  event_info_block[10] |= ( (read_time & 0xFFFFFF) << 8 ) & 0xFFFFFF00;  
  event_info_block[11] = minos & 0x3FFFFFFF;           // the minos gate (only 28 bits of data)
#ifndef GOFAST
  for (int i = 0; i < 12; i++) {
    daqevt.debugStream() << "   DAQHeader Data Int [" << i << "] = " << event_info_block[i];
  }
#endif
  // We need to allow room for the Minerva Frame Header we haven't added yet.
  int buffer_index = 4; // 4+4=8 bytes for the MINERvA Frame Header.
  for (int i = 0; i < 12; i++) {
    buffer_index += 4;   
    data[buffer_index]   = event_info_block[i] & 0xFF;
    data[buffer_index+1] = (event_info_block[i]>>8) & 0xFF;
    data[buffer_index+2] = (event_info_block[i]>>16) & 0xFF;
    data[buffer_index+3] = (event_info_block[i]>>24) & 0xFF;
  }

  const unsigned short *tmpDAQHeader = header->GetBankHeader();
  buffer_index = 0; 
  for (int i = 0; i < 4 ;i++) {
    data[buffer_index] = tmpDAQHeader[i]&0xFF;
    buffer_index++;
    data[buffer_index] = (tmpDAQHeader[i]&0xFF00)>>0x08;
    buffer_index++;
  }
#ifndef GOFAST
  daqevt.debugStream() << " DAQ Header Data...";
  for (int i = 0; i < daqHeaderSize; i++) {
    daqevt.debugStream() << "   data[" << i << "] = " << (int)data[i];  
  }
#endif
}

//----------------------------------------------------------------
DAQHeader::~DAQHeader() 
{ 
  this->ClearData();
}

//----------------------------------------------------------------
unsigned char* DAQHeader::GetData() const 
{ 
  return data; 
}

//----------------------------------------------------------------
unsigned short DAQHeader::GetDataLength() const 
{ 
  return dataLength;
}

//----------------------------------------------------------------
void DAQHeader::ClearData() 
{ 
  if ( (dataLength > 0) && (NULL != data) ) {
    delete [] data;
    data = NULL;
    dataLength = 0;
  }
}

//----------------------------------------------------------------
unsigned char DAQHeader::GetData(int i) const 
{
  if (i < dataLength) { 
    return data[i]; 
  }
  return 0;
}

#endif
