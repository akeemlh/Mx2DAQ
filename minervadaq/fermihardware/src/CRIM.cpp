#ifndef CRIM_cpp
#define CRIM_cpp

#include <iostream>
#include <iomanip>
#include "CRIM.h"
#include "exit_codes.h"

/*********************************************************************************
 * Class for creating Chain Read-Out Controller Interfact Module objects for 
 * use with the MINERvA data acquisition system and associated software projects.
 *
 * Elaine Schulte, Rutgers University
 * Gabriel Perdue, The University of Rochester
 **********************************************************************************/

log4cpp::Category& CRIMLog = log4cpp::Category::getInstance(std::string("CRIM"));

/*! mask constants for the CRIM */
unsigned short const CRIM::TimingSetupRegisterModeMask      = 0xF000;
unsigned short const CRIM::TimingSetupRegisterFrequencyMask = 0x0FFF;
unsigned short const CRIM::GateWidthRegisterMask            = 0x007F;
unsigned short const CRIM::GateWidthRegisterEnableCNRSTMask = 0x8000;
unsigned short const CRIM::TCALBDelayRegisterMask           = 0x03FF;
unsigned short const CRIM::InterruptMaskRegisterMask        = 0x00FF;
unsigned short const CRIM::InterruptStatusRegisterMask      = 0x00FF;
unsigned short const CRIM::ClearInterruptsRegisterMask      = 0x0081;
unsigned short const CRIM::InterruptConfigRegisterIRQMask   = 0x0007;
unsigned short const CRIM::InterruptConfigGlobalEnableMask  = 0x0080;
unsigned short const CRIM::TriggerRegisterPulseMask         = 0x0404;
unsigned short const CRIM::TCALBRegisterPulseMask           = 0x0404;
unsigned short const CRIM::SGATERegisterStartMask           = 0x0401;
unsigned short const CRIM::SGATERegisterStopMask            = 0x0402;
unsigned short const CRIM::CNRSTRegisterPulseMask           = 0x0202;
unsigned short const CRIM::CNRSTRegisterOneShotMask         = 0x0808;
unsigned short const CRIM::ControlRegisterFETriggerMask     = 0x1000;
unsigned short const CRIM::ControlRegisterCRCMask           = 0x2000;
unsigned short const CRIM::ControlRegisterSendMask          = 0x4000;
unsigned short const CRIM::ControlRegisterRetransmitMask    = 0x8000;
unsigned short const CRIM::TimingCommandRegisterMask        = 0x00FF;
unsigned short const CRIM::SequencerControlLatchResetMask   = 0x0202;
unsigned short const CRIM::TimingViolationRegisterMask      = 0x01FF;
unsigned short const CRIM::TimingViolationRegisterClearMask = 0x0101; 
unsigned short const CRIM::MinosSGATELowerBitsMask          = 0xFFFF;
unsigned short const CRIM::MinosSGATEUpperBitsMask          = 0x0FFF;

const unsigned short CRIM::softTrigger    = 0x0404;
const unsigned short CRIM::softTCALB      = 0x0404;
const unsigned short CRIM::softSGATEstart = 0x0401;
const unsigned short CRIM::softSGATEstop  = 0x0402;
const unsigned short CRIM::softCNRST      = 0x0202;
const unsigned short CRIM::softCNRSTseq   = 0x0808;

//----------------------------------------
CRIM::CRIM( unsigned int address, log4cpp::Appender* appender, Controller* controller ) :
  VMECommunicator( address, appender, controller )
{
  this->addressModifier = cvA24_U_DATA; 

  // NOTE: The IRQ level must be the same as the configuration register level.  
  // The BIT MASKS for these levels, however are not the same!
  irqLine              = SGATEFall; // correct choice for every running mode but Cosmics
  irqLevel             = cvIRQ5;    // interrupt level 5 for the CAEN interrupt handler
  interruptConfigValue = 5;         // the interrupt level stored in the interrupt config register
  resetInterrupts      = 0x81;      // clear all pending interrupts

  CRIMAppender = appender;
  if (CRIMAppender == 0 ) {
    std::cout << "CRIM Log Appender is NULL!" << std::endl;
    exit(EXIT_CRIM_UNSPECIFIED_ERROR);
  }
  CRIMLog.setPriority(log4cpp::Priority::DEBUG); 

  interruptStatusRegister = this->address + (unsigned int)CRIMInterruptStatus;
  interruptConfig         = this->address + (unsigned int)CRIMInterruptConfig;
  interruptsClear         = this->address + (unsigned int)CRIMClearInterrupts;
  interruptAddress        = this->address + (unsigned int)CRIMInterruptMask;
  timingRegister          = this->address + (unsigned int)CRIMTimingSetup;
  SGATEWidthRegister      = this->address + (unsigned int)CRIMSGATEWidth;
  TCALBDelayRegister      = this->address + (unsigned int)CRIMTCALBDelay;
  softwareTriggerRegister = this->address + (unsigned int)CRIMSoftwareTrigger;
  softwareTCALBRegister   = this->address + (unsigned int)CRIMSoftwareTCALB;
  softwareSGATERegister   = this->address + (unsigned int)CRIMSoftwareSGATE;
  softwareCNRSTRegister   = this->address + (unsigned int)CRIMSoftwareCNRST;
  controlRegisterAddress  = this->address + (unsigned int)CRIMControl;
  statusRegisterAddress   = this->address + (unsigned int)CRIMStatus;
  clearStatusRegister     = this->address + (unsigned int)CRIMClearStatus;
  gateTimeWordLowAddress  = this->address + (unsigned int)CRIMGateTimeWordLow;
  gateTimeWordHighAddress = this->address + (unsigned int)CRIMGateTimeWordHigh;
  sequencerResetRegister  = this->address + (unsigned int)CRIMSequencerControlLatch;

  // register value for control register (DAQ Mode control)
  // set crc & send to true and retransmit to false	
  controlRegister = ControlRegisterCRCMask | ControlRegisterSendMask 
    & ~ControlRegisterRetransmitMask; 
}

//----------------------------------------
void CRIM::Initialize( RunningModes runningMode )
{
  CRIMLog.infoStream() << "Initializing CRIM 0x" << std::hex << this->address;
  this->logRunningMode( runningMode );

  // The CRIM will ignore the SequencerEnable instruction in firmware before v9.
  unsigned short GateWidth         = 0x7F;     // GetWidth must never be zero!
  unsigned short TCALBDelay        = 0x3FF;    // Delay should also be non-zero.
  unsigned short TCALBEnable       = 0x1;      // Enable pulse delay.
  unsigned short SequencerEnable   = 0x1;      // Sequencer control (0 means always send gates, 1 for rearms).
  CRIMTimingFrequencies Frequency  = ZeroFreq; // Used to set ONE frequency bit!  ZeroFreq ~no Frequency.
  CRIMTimingModes       TimingMode = MTM;      // Default to MTM.

  switch (runningMode) {
    case OneShot:
      // "OneShot" is the casual name for CRIM internal timing with software gates.
      // We can use MTM mode though and prefer to when an MTM is available. This 
      // removes the need to switch clock modes for pure pedestal. We keep the name
      // "OneShot" for historical reasons no matter the clock mode.
#if NOMTMPEDESTAL 
      TimingMode   = crimInternal;
#endif
      break;
    case NuMIBeam:
      break;
    case PureLightInjection:
#if MTEST
      // Because no MTM is available at MTest, LI will use internal timing.
      TimingMode   = CRIMInternal;
      CRIMLog.infoStream() << " ->Using MTest timing (CRIM Internal).";
#endif
      break;
    case MixedBeamPedestal:
    case MixedBeamLightInjection:
      break;
    case Cosmics:
    case MTBFBeamMuon:
    case MTBFBeamOnly:
      // Cosmics, Beam-Muon, & Beam-Only use CRIM internal timing with gates send at a set frequency.
      Frequency    = F2;
      TimingMode   = crimInternal;
      this->SetIRQLine(Trigger); //crimInterrupts type
      break;
    default:
      CRIMLog.fatalStream() << "Error in acquire_data::InitializeCrim()! No Running Mode defined!";
      exit(EXIT_CONFIG_ERROR);
  }
  CRIMLog.debugStream() << " GateWidth       = 0x" << std::hex << GateWidth;
  CRIMLog.debugStream() << " TCALBDelay      = 0x" << std::hex << TCALBDelay;
  CRIMLog.debugStream() << " TCALBEnable     = 0x" << std::hex << TCALBEnable;
  CRIMLog.debugStream() << " SequencerEnable = 0x" << std::hex << SequencerEnable;
  CRIMLog.debugStream() << " Frequency       = " << Frequency;
  CRIMLog.debugStream() << " Timing Mode     = " << TimingMode;

}

//----------------------------------------
void CRIM::logRunningMode( RunningModes runningMode )
{
  switch (runningMode) {
    case OneShot:
      CRIMLog.infoStream() << " Running Mode is OneShot.";
      break;
    case NuMIBeam:
      CRIMLog.infoStream() << " Running Mode is NuMI Beam.";
      break;
    case PureLightInjection:
      CRIMLog.infoStream() << " Running Mode is PureLightInjection.";
      break;
    case MixedBeamPedestal:
      CRIMLog.infoStream() << " Running Mode is MixedBeamPedestal.";
      break;
    case MixedBeamLightInjection:
      CRIMLog.infoStream() << " Running Mode is MixedBeamLightInjection.";
      break;
    case Cosmics:
      CRIMLog.infoStream() << " Running Mode is Cosmic.";
      break;
    case MTBFBeamMuon:
      CRIMLog.infoStream() << " Running Mode is MTBFBeamMuon.";
      break;
    case MTBFBeamOnly:
      CRIMLog.infoStream() << " Running Mode is MTBFBeamOnly.";
      break;
    default:
      CRIMLog.critStream() << "Error in ReadoutWorker::logRunningMode()! Undefined Running Mode!";
  }
}

//----------------------------------------
unsigned int CRIM::GetAddress() 
{
  return this->address;
}

//----------------------------------------
void CRIM::SetCRCEnable(bool a) 
{
  /*! \fn
   *
   * this filps the crc enable bit(s) on the control register value
   *
   *  \param a status bit to decide how to set the bit
   */
  if (a) {
    controlRegister |= ControlRegisterCRCMask;
  } else {
    controlRegister &= ~ControlRegisterCRCMask;
  }
}

//----------------------------------------
void CRIM::SetSendEnable(bool a) 
{
  /*! \fn
   *
   * this filps the send enable bit(s) on the control register value
   *
   *  \param a status bit to decide how to set the bit
   */
  if (a) {
    controlRegister |= ControlRegisterSendMask;
  } else {
    controlRegister &= ~ControlRegisterSendMask;
  }
}

//----------------------------------------
void CRIM::SetReTransmitEnable(bool a) 
{
  /*! \fn
   *
   * this filps the transmit enable bit(s) on the control register value
   *
   *  \param a status bit to decide how to set th bit
   */
  if (a) {
    controlRegister |= ControlRegisterRetransmitMask;
  } else {
    controlRegister &= ~ControlRegisterRetransmitMask;
  }
}

//----------------------------------------
void CRIM::SetupTiming( CRIMTimingModes a, CRIMTimingFrequencies b ) 
{
  timingSetup = ( a & TimingSetupRegisterModeMask ) | 
    (b & TimingSetupRegisterFrequencyMask);

  CRIMLog.debugStream() << "  CRIM timingSetup = 0x" 
    << std::setfill('0') << std::setw( 4 ) << std::hex << timingSetup;
  CRIMLog.debugStream() << "        timingMode = 0x" 
    << std::setfill('0') << std::setw( 4 ) << std::hex << a;
  CRIMLog.debugStream() << "         frequency = 0x" 
    << std::setfill('0') << std::setw( 4 ) << std::hex << b;
} 

//----------------------------------------
void CRIM::SetupGateWidth( unsigned short tcalbEnable, unsigned short gateWidth ) 
{
  gateWidthSetup = ((tcalbEnable & 0x1)<<15) | (gateWidth & GateWidthRegisterMask);
}  

//----------------------------------------
void CRIM::SetupGateWidth( unsigned short tcalbEnable, 
    unsigned short gateWidth, unsigned short sequencerEnable ) 
{
  gateWidthSetup = ((tcalbEnable & 0x1)<<15) | 
    ((sequencerEnable & 0x1)<<10)            | 
    (gateWidth & GateWidthRegisterMask);
}

//----------------------------------------
void CRIM::SetupTCALBPulse( unsigned short pulseDelay ) 
{
  TCALBDelaySetup = pulseDelay & TCALBDelayRegisterMask;
} 

//----------------------------------------
unsigned short CRIM::GetStatus()
{
  unsigned char dataBuffer[] = {0x0,0x0}; 
  int error = ReadCycle( dataBuffer, statusRegisterAddress, addressModifier, dataWidthReg );
  if( error ) exitIfError( error, "Failure reading the CRIM Status Register!");
  unsigned short status = dataBuffer[1]<<8 | dataBuffer[0];
  return status;
}


#endif
