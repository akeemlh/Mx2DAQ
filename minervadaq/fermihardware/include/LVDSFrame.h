#ifndef LVDSFrame_h
#define LVDSFrame_h

#include <iostream>
#include <fstream>
#include <tr1/memory>  // for shared_ptrs

#include "FrameTypes.h"
#include "MinervaDAQSizes.h"
#include "TripTTypes.h"
#include "FlashTypes.h"
#include "log4cppHeaders.h"

/*********************************************************************************
 * Class for creating general Frame header objects for use with the 
 * MINERvA data acquisition system and associated software projects.
 *
 * Elaine Schulte, Rutgers University
 * Gabriel Perdue, The University of Rochester
 **********************************************************************************/

/*! \class LVDSFrame
 *
 * \brief The base class for all LVDS data exchange frames.
 *
 */

class LVDSFrame {

  friend std::ostream& operator<<(std::ostream&, const LVDSFrame&);

  protected:
    unsigned char FrameID[2];        
    unsigned char frameHeader[9];    
    unsigned char *outgoingMessage;  
    unsigned char *receivedMessage;  
    unsigned char febNumber[1], targetDevice[1], deviceFunction[1], 
                  broadcastCommand[1], messageDirection[1];
    inline void DeleteOutgoingMessage() { delete [] outgoingMessage; outgoingMessage = NULL; };

  public:
    LVDSFrame();
    virtual ~LVDSFrame();

    void MakeDeviceFrameTransmit(FrameTypes::Devices, FrameTypes::Broadcasts, 
        FrameTypes::Directions, unsigned int, unsigned int); 
    void MakeOutgoingHeader();

    virtual void MakeMessage();
    virtual void DecodeRegisterValues();
    virtual unsigned int GetOutgoingMessageLength();

    unsigned short ReceivedMessageLength() const;
    unsigned short ReceivedMessageStatus() const;
    void DecodeHeader();
    bool CheckForErrors();
    void printReceivedMessageToLog();

    inline unsigned char *GetOutgoingMessage() const { return outgoingMessage; };
    inline unsigned char *GetReceivedMessage() const { return receivedMessage; };
    inline void SetReceivedMessage(unsigned char* buffer) { receivedMessage = buffer; };
    inline int GetFEBNumber() const { return (int)febNumber[0]; };
    inline int GetDeviceType() const { return (int)targetDevice[0]; };

};

#endif
