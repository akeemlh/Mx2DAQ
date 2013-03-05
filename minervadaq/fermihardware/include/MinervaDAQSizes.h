#ifndef MinervaDAQSizes_h
#define MinervaDAQSizes_h

#include <cmath>

/*! \file MinervaDAQSizes.h
 *
 * \brief Constant sizes.
 *
 */

/* A new, shiny "Minerva Long" for those pesky >32-bit integers on a 32-bit kernel 
   Elaine Schulte, June 4, 2009 
   */
#ifdef SYS_32
typedef unsigned long long long_m;
#else
typedef unsigned long long_m;
#endif 


namespace MinervaDAQSizes {


  static const unsigned int ADCFramesMaxNumber        = 7 + 1; // timed + 1 untimed
  static const unsigned int FrameHeaderLengthOutgoing = 9; //size (in bytes) of an outgoing LVDS header for ANY device
  static const unsigned int FrameHeaderLengthIncoming = 9; //12? size (in bytes) of an outgoing LVDS header for ANY device

  static const unsigned int FPGANumRegisters    =   54;  // Firmware Dependent!
  static const unsigned int FPGAFrameMaxSize    =   68;  // bytes, Firmware Dependent!
  static const unsigned int ADCFrameMaxSize     =  446;  // bytes
  static const unsigned int DiscrFrameMaxSize   = 1138;  // bytes, == 18 + 40 * 4 * 7 (40 bytes / trip / hit)
  static const unsigned int FEBTotalDataMaxSize = FPGAFrameMaxSize + 
    ADCFramesMaxNumber*ADCFrameMaxSize + DiscrFrameMaxSize; // bytes

  static const unsigned int TRiPProgrammingFrameReadSize = 652; // bytes; THIS IS THE compose SIZE!
  static const unsigned int TRiPProgrammingFrameWriteSize = 758; // bytes; THIS IS THE compose SIZE!
  static const unsigned int TRiPProgrammingFrameReadResponseSize = 656; // bytes; THIS IS THE RESPONSE SIZE!
  static const unsigned int TRiPProgrammingFrameWriteResponseSize = 762; // bytes; THIS IS THE RESPONSE SIZE!

  static const unsigned int MaxFEBsPerChain = 10;

  static const unsigned int MaxTotalDataPerChain = MaxFEBsPerChain * FEBTotalDataMaxSize;

  static const unsigned int nDiscrChPerTrip = 16; // number of channels
  static const unsigned int nPixelsPerFEB         = 64;
  static const unsigned int nHiMedTripsPerFEB     = 4;
  static const unsigned int nSides                = 2;
  static const unsigned int nPixelsPerTrip        = 16;  // nPixelsPerFEB / nHiMedTripsPerFEB
  static const unsigned int nPixelsPerSide        = 32;  // nPixelsPerFEB / nSides
  static const unsigned int nChannelsPerTrip      = 36;  // 1 dummy ADC reading + 32 real channel + 2 edge channels + 1 ADC latency
  static const unsigned int nSkipChannelsPerTrip  = 3;

}

#endif