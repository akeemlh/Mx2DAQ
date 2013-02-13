#ifndef VMECommunicator_h
#define VMECommunicator_h
/*! \file VMECommunicator.h 
 *
 * \brief The header file for the VMECommunicator class.
 *
 * This class is a base class for modules that can communicate via the VME protocol.
 */

#include "CAENVMEtypes.h"
#include "CAENVMElib.h"

#include "Controller.h"
#include "log4cppHeaders.h"

/*! \class VMECommunicator
 *  \brief A class which wraps the CAEN VME access functions.
 *
 */


class VMECommunicator {

  private:
    const Controller*  controller;
    int                controllerHandle;

  protected:
    unsigned int       address;             // this is the most basic address available 
                                            // (for channels, it will be the CROC address), bit-shifted
    CVAddressModifier  addressModifier;
    CVAddressModifier  bltAddressModifier;  // block transfers use a different address modifier
    CVDataWidth        dataWidth;
    CVDataWidth        dataWidthSwapped;
    CVDataWidth        dataWidthReg;        // use a different data width for talking to registers (as opposed to data)
    CVDataWidth        dataWidthSwappedReg; // use a different data width for talking to registers (as opposed to data)

    virtual void exitIfError( int error, const std::string& msg ) const;

  public:

    explicit VMECommunicator( unsigned int address, 
        const Controller* controller );
    virtual ~VMECommunicator(); 

    const Controller* GetController() const;

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
