#ifndef ECROC_h
#define ECROC_h

#include <list>
#include <fstream>
#include "CAENVMEtypes.h"
#include "echannels.h"
#include "MinervaDAQtypes.h"

/*********************************************************************************
* Class for creating CROC-E objects for use with the 
* MINERvA data acquisition system and associated software projects.
*
* Gabriel Perdue, The University of Rochester
**********************************************************************************/

/*! \class ECROC
 *
 * \brief The class which controls data for CROC-Es. 
 *
 */

class ECROC {
	private:
		unsigned int vmeAddress; 		// the bit-shifted VME address
		int id; 				// the id is an "index" used by the controller
		std::list<echannels*> ECROCChannels; 
		CVAddressModifier addressModifier; 
		CVDataWidth dataWidth;
		CVDataWidth dataWidthSwapped;

		unsigned int timingSetupAddress;
		unsigned int resetAndTestPulseMaskAddress;
		unsigned int channelResetAddress;
		unsigned int fastCommandAddress;
		unsigned int testPulseAddress;
		unsigned int rdfePulseDelayAddress;
		unsigned int rdfePulseCommandAddress;

		unsigned short timingRegisterMessage;
		unsigned short resetAndTestPulseMaskRegisterMessage;
		unsigned short channelResetRegisterMessage;
		unsigned short fastCommandRegisterMessage;
		unsigned short testPulseRegisterMessage;

		log4cpp::Appender* ECROCAppender;

	public:
		ECROC( unsigned int address, int ECROCid, log4cpp::Appender* appender=0 ); 
		~ECROC(); 

		unsigned int GetAddress(); 
		int GetCrocID(); 
		CVAddressModifier GetAddressModifier(); 
		CVDataWidth GetDataWidth();
		CVDataWidth GetDataWidthSwapped(); 

		void SetupChannels(); 
		echannels *GetChannel( unsigned int i ); 
		std::list<echannels*>* GetChannelsList(); 

		unsigned int GetTimingSetupAddress();
		void SetTimingRegisterMessage(crocClockModes clockMode, unsigned short testPulseDelayEnabled, unsigned short testPulseDelayValue); 
		unsigned short GetTimingRegisterMessage(); 

		void SetResetAndTestPulseRegisterMessage( unsigned short resetEnable, unsigned short testPulseEnable ); 

		void SetFastCommandRegisterMessage(unsigned short value);
		unsigned int GetFastCommandAddress();
		unsigned short GetFastCommandRegisterMessage();

		void InitializeRegisters( crocClockModes clockMode, 
				unsigned short testPulseDelayValue,
				unsigned short testPulseDelayEnabled ); 



};

#endif
