#ifndef VMECrate_cxx
#define VMECrate_cxx

#include "VMECrate.h"

log4cpp::Category& vmeCrate = log4cpp::Category::getInstance(std::string("vmeCrate")); 

//---------------------------
VMECrate::VMECrate( int theCrateID, log4cpp::Priority::Value priority, bool VMEInit ) :
  crateID(theCrateID),
  vmeInit(VMEInit),
  runningMode((RunningModes)0)
{
  vmeCrate.setPriority(priority);

  controller = new Controller(0x00, crateID);
  int error = controller->Initialize();
  if ( 0 != error ) {
    vmeCrate.fatalStream() << "Controller contact error: " << error 
      << "; for Crate ID = " << crateID; 
    exit(error);
  }
  vmeCrate.debugStream() << "Made new ReadoutWokrer with crate ID = " << crateID 
    << "; VME Init = " << vmeInit
    << "; Logging Level = " << priority;
}

//---------------------------
VMECrate::~VMECrate() {
  for( std::vector<ECROC*>::iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p ) {
    delete (*p);
  }
  ecrocs.clear();
  for( std::vector<CRIM*>::iterator p=crims.begin(); p!=crims.end(); ++p ) {
    delete (*p);
  }
  crims.clear();
  delete controller; 
}

//---------------------------
void VMECrate::Initialize( RunningModes theRunningMode )
{
  runningMode = theRunningMode;
  vmeCrate.debugStream() << "Initialize " << (*this);

  vmeCrate.debugStream() << "Initializing " << crims.size() << " CRIMs...";
  for( std::vector<CRIM*>::iterator p=crims.begin(); p!=crims.end(); ++p ) {
    (*p)->Initialize( runningMode );
  }
  vmeCrate.debugStream() << "Initializing " << ecrocs.size() << " CROC-Es...";
  for( std::vector<ECROC*>::iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p ) {
    (*p)->Initialize();
  }
	// IRQ Enable will happen in the ReadoutWorker, which holds the proper MasterCRIM*
}

//---------------------------
void VMECrate::AddECROC( unsigned int address, int nFEBchan0, int nFEBchan1, int nFEBchan2, int nFEBchan3 )
{
  if (address < (1<<VMEModuleTypes::ECROCAddressShift)) {
    address = address << VMEModuleTypes::ECROCAddressShift;
  }
  vmeCrate.infoStream() << "Adding ECROC with address = 0x" 
    << std::hex << address << " and FEBs-to-Channel of (" 
    << std::dec << nFEBchan0 << ", " << nFEBchan1 << ", " << nFEBchan2 << ", " << nFEBchan3 << ")";
  if (nFEBchan0<0 || nFEBchan0>10) nFEBchan0 = 0;
  if (nFEBchan1<0 || nFEBchan1>10) nFEBchan1 = 0;
  if (nFEBchan2<0 || nFEBchan2>10) nFEBchan2 = 0;
  if (nFEBchan3<0 || nFEBchan3>10) nFEBchan3 = 0;

  ECROC *theECROC = new ECROC( address, this->controller );
  theECROC->ClearAndResetStatusRegisters();
  theECROC->DisableSequencerReadout(); // make sure this is disabled for init
  vmeCrate.debugStream() << " Adding FEBs to Channels...";
  theECROC->GetChannel( 0 )->SetupNFrontEndBoards( nFEBchan0 );
  vmeCrate.debugStream() << " Setup Channel 0 with " << nFEBchan0 << " FEBS.";
  theECROC->GetChannel( 1 )->SetupNFrontEndBoards( nFEBchan1 );
  vmeCrate.debugStream() << " Setup Channel 1 with " << nFEBchan1 << " FEBS.";
  theECROC->GetChannel( 2 )->SetupNFrontEndBoards( nFEBchan2 );
  vmeCrate.debugStream() << " Setup Channel 2 with " << nFEBchan2 << " FEBS.";
  theECROC->GetChannel( 3 )->SetupNFrontEndBoards( nFEBchan3 );
  vmeCrate.debugStream() << " Setup Channel 3 with " << nFEBchan3 << " FEBS.";
  theECROC->ClearEmptyChannels();
  ecrocs.push_back( theECROC );
  vmeCrate.debugStream() << "Added ECROC.";
}

//---------------------------
void VMECrate::AddCRIM( unsigned int address )
{
  if (address < (1<<VMEModuleTypes::CRIMAddressShift)) {
    address = address << VMEModuleTypes::CRIMAddressShift;
  }
  vmeCrate.infoStream() << "Adding CRIM with address = 0x" << std::hex << address; 
  CRIM* crim = new CRIM( address, this->controller );
  vmeCrate.debugStream() << " CRIM Status = 0x" << std::hex << crim->GetStatus();
  crims.push_back( crim );
  vmeCrate.debugStream() << "Added CRIM.";
}

//---------------------------
void VMECrate::SendSoftwareRDFE() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->SendSoftwareRDFE();
}

//---------------------------
void VMECrate::WaitForSequencerReadoutCompletion() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->WaitForSequencerReadoutCompletion();
}

//---------------------------
void VMECrate::EnableSequencerReadout() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->EnableSequencerReadout();
}

//---------------------------
void VMECrate::DisableSequencerReadout() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->DisableSequencerReadout();
}

//---------------------------
void VMECrate::ClearAndResetStatusRegisters() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->ClearAndResetStatusRegisters();
}

//---------------------------
void VMECrate::OpenGateFastCommand() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->FastCommandOpenGate();
}

//---------------------------
void VMECrate::SequencerDelayDisable() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->SequencerDelayDisable();
}

//---------------------------
void VMECrate::SequencerDelayEnable() const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->SequencerDelayEnable();
}

//---------------------------
void VMECrate::SetSequencerDelayValue( unsigned short delay ) const
{
  for (std::vector<ECROC*>::const_iterator p=ecrocs.begin(); p!=ecrocs.end(); ++p) 
    (*p)->SetSequencerDelayValue( delay );
}

//---------------------------
std::vector<ECROC*>* VMECrate::GetECROCVector()
{
  return &ecrocs;
}

//---------------------------
ECROC* VMECrate::GetECROCVector( int index )
{
  return ecrocs[index];
}

//---------------------------
std::vector<CRIM*>* VMECrate::GetCRIMVector()
{
  return &crims;
}

//---------------------------
CRIM* VMECrate::GetCRIMVector( int index )
{
  return crims[index];
}

//-----------------------------
std::ostream& operator<<(std::ostream& out, const VMECrate& s)
{
  out << "Crate = " << s.crateID << "; ";
  out << "Running Mode = " << s.runningMode << "; ";
  return out;
}

#endif
