#ifndef VMECommunicator_h
#define VMECommunicator_h
/*! \file VMECommunicator.h 
*/
#include <ostream>

#include "CAENVMEtypes.h"
#include "CAENVMElib.h"

#include "log4cppHeaders.h"
#include "Controller.h"
#include "VMEModuleTypes.h"

/*! 
  \class VMECommunicator
  \brief A class which wraps the CAEN VME access functions.
  \author Gabriel Perdue
  \author Elaine Schulte
  */


class VMECommunicator {

  friend std::ostream& operator<<(std::ostream&, const VMECommunicator&);

  private:
  const Controller* controller;
  int               controllerHandle;


  protected:
  VMEModuleTypes::VMECommunicatorType commType;

  unsigned int       address;             /*!< The most basic address, bit shifted. For channels, it will be the CROC address. */
  CVAddressModifier  addressModifier;
  CVAddressModifier  bltAddressModifier;  /*!< block transfers use a different address modifier */
  CVDataWidth        dataWidth;
  CVDataWidth        dataWidthSwapped;
  CVDataWidth        dataWidthReg;        /*!< use a different data width for talking to registers (as opposed to data) */
  CVDataWidth        dataWidthSwappedReg; /*!< use a different data width for talking to registers (as opposed to data) */

  virtual void throwIfError( int error, const std::string& msg ) const;
  virtual void VMEThrow( std::string msg ) const;

  public:

  explicit VMECommunicator( unsigned int address, 
      const Controller* controller );
  virtual ~VMECommunicator(); 

  virtual unsigned int GetAddress() const;

  const Controller* GetController() const;

  VMEModuleTypes::VMECommunicatorType GetCommType() const;
  int GetCrateNumber() const;

  int WriteCycle(int messageLength, unsigned char *send_message,  unsigned int address, 
      CVAddressModifier AM, CVDataWidth DW) const; 

  int ReadCycle(unsigned char *received_message,  unsigned int address, 
      CVAddressModifier AM, CVDataWidth DW) const; 

  int ReadBLT(unsigned char *received_message,  int blocks, unsigned int address, 
      CVAddressModifier AM, CVDataWidth DW) const; 

  int WriteFIFOBLT(int messageLength, unsigned char *send_message,  unsigned int address, 
      CVAddressModifier AM, CVDataWidth DW) const; 
};

#endif
