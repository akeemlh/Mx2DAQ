#ifndef EChannels_h
#define EChannels_h

/* system header files here */
#include <list>
#include <fstream>
#include <string>
#include <sstream>

/* CAEN VME header files here */
#include "CAENVMEtypes.h"

/* custom header files here */
#include "FEB.h"
#include "FrontEndBoard.h"
#include "MinervaDAQtypes.h"
#include "FrameTypes.h"
#include "VMECommunicator.h"

/*********************************************************************************
* Class for creating CROC-E channel objects for use with the 
* MINERvA data acquisition system and associated software projects.
* 
* Gabriel Perdue, The University of Rochester
***********************************************************************************/

/*! \class EChannels
 *
 *  \brief This class holds data associated with a CROC-E channel.
 *
 * This class holds the data which belongs to a CROC-E channel, including the list
 * of FEB's which are on the channel.  
 *
 */

class EChannels : public VMECommunicator {
	private:
		unsigned int channelNumber;             /*!< the channel identifying index, used for retrieval, currently indexed 0-3 */  
		unsigned int channelDirectAddress;	    /*!< base + offset */

		unsigned int receiveMemoryAddress;
		unsigned int sendMemoryAddress;
		unsigned int framePointersMemoryAddress;
		unsigned int configurationAddress;
		unsigned int commandAddress;
		unsigned int eventCounterAddress;
		unsigned int framesCounterAndLoopDelayAddress;
		unsigned int frameStatusAddress;
		unsigned int txRxStatusAddress;
		unsigned int receiveMemoryPointerAddress;

		std::vector<FEB*> FEBsVector;     
		std::vector<FrontEndBoard*> FrontEndBoardsVector;     

    bool isAvailable( FEB* feb ) const;
    bool isAvailable( FrontEndBoard* feb ) const;
    void UpdateConfigurationForVal( unsigned short val, unsigned short mask ) const;
    void SetChannelConfiguration( unsigned char* message ) const;

	public:
		explicit EChannels( unsigned int baseVMEAddress, unsigned int channelNumber, 
        const Controller* controller );
		~EChannels();

    virtual void exitIfError( int error, const std::string& msg ) const;

    void ClearAndResetStatusRegister() const;  
    unsigned short ReadFrameStatusRegister() const;
    unsigned short ReadTxRxStatusRegister() const;
    unsigned short GetChannelConfiguration() const;

    void WriteMessageToMemory( unsigned char* message, int messageLength ) const;
    void SendMessage() const;

    unsigned short ReadEventCounter() const;
    unsigned short WaitForSequencerReadoutCompletion() const;
    unsigned short WaitForMessageReceived() const;
    unsigned short ReadDPMPointer() const;
    unsigned char* ReadMemory( unsigned short dataLength ) const; 

    // write the frames - load them to memory (don't send messages)
    // no WriteFrame is deliberate - too many configs - we just offer preconfigured reads
    void WriteFrameRegistersToMemory( std::tr1::shared_ptr<LVDSFrame> frame ) const;
    void WriteFPGAProgrammingRegistersToMemory( FEB *feb ) const;
    /* void WriteFPGAProgrammingRegistersToMemory( std::tr1::shared_ptr<FPGAFrame> frame ) const; */
    void WriteFPGAProgrammingRegistersReadFrameToMemory( FEB *feb ) const; 
    void WriteFPGAProgrammingRegistersDumpReadToMemory( FEB *feb ) const; 
    void WriteFPGAProgrammingRegistersDumpReadToMemory( std::tr1::shared_ptr<FPGAFrame> frame ) const; 
    void WriteTRIPRegistersToMemory( FEB *feb, int tripNumber ) const;
    /* void WriteTRIPRegistersToMemory( std::tr1::shared_ptr<TRIPFrame> frame ) const; */
    void WriteTRIPRegistersReadFrameToMemory( FEB *feb, int tripNumber ) const;  // TODO - not sure this is worth a whole function
    void WriteTRIPRegistersReadFrameToMemory( std::tr1::shared_ptr<TRIPFrame> frame ) const; 
    // read the frames - load messages to memory, send them, and then check the dpm pointer
    unsigned short ReadFPGAProgrammingRegistersToMemory( FEB *feb ) const;
    unsigned short ReadFPGAProgrammingRegistersToMemory( std::tr1::shared_ptr<FPGAFrame> frame ) const;

    unsigned int GetChannelNumber() const;
    unsigned int GetParentECROCAddress() const;
    unsigned int GetParentCROCNumber() const;
    unsigned int GetDirectAddress() const;

    int DecodeStatusMessage( const unsigned short& status ) const;

    void EnableSequencerReadout() const;
    void DisableSequencerReadout() const;

    void SetupNFEBs( int nFEBs );
    std::vector<FEB*>* GetFEBVector();
    FEB* GetFEBVector( int index /* should always equal (FEB address - 1) */ );

    void SetupNFrontEndBoards( int nFEBs );
    std::vector<FrontEndBoard*>* GetFrontEndBoardVector();
    FrontEndBoard* GetFrontEndBoardVector( int index /* should always equal (FEB address - 1) */ );
};
#endif
