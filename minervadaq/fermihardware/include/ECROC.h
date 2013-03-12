#ifndef ECROC_h
#define ECROC_h

#include <list>
#include <fstream>
#include "CAENVMEtypes.h"
#include "EChannels.h"
#include "VMEModuleTypes.h"

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

class ECROC : public VMECommunicator {

	private:
		std::vector<EChannels*> ECROCChannels; 

		unsigned int timingSetupAddress;
		unsigned int resetAndTestPulseMaskAddress;
		unsigned int channelResetAddress;
		unsigned int fastCommandAddress;
		unsigned int testPulseAddress;
		unsigned int rdfePulseDelayAddress;
		unsigned int rdfePulseCommandAddress;

		unsigned short channelResetRegisterMessage;
		unsigned short testPulseRegisterMessage;

		void MakeChannels(); 

    static const unsigned short RDFEDelayRegisterDelayMask;
    static const unsigned short RDFEDelayRegisterEnableMask;
    static const unsigned short RDFEDelayRegisterEnableBit;

    unsigned short ReadSequencerPulseDelayRegister() const;
    void SetSequencerDelayeRegister( unsigned short registerConfig ) const;
    void SequencerDelayEnableDisable( unsigned short bit ) const;

	public:
		explicit ECROC( unsigned int address, const Controller* controller); 
		~ECROC(); 

    void Initialize() const;
		virtual unsigned int GetAddress() const; 

		EChannels *GetChannel( unsigned int i ); 
		std::vector<EChannels*>* GetChannelsVector(); 
    void ClearEmptyChannels();

		void SetupTimingRegister( VMEModuleTypes::ECROCClockModes clockMode, 
        unsigned short testPulseDelayEnabled, 
        unsigned short testPulseDelayValue) const; 
		void SetupResetAndTestPulseRegister( unsigned short resetEnable, 
        unsigned short testPulseEnable ) const; 
		void InitializeRegisters( VMEModuleTypes::ECROCClockModes clockMode, 
				unsigned short testPulseDelayValue,
				unsigned short testPulseDelayEnabled ) const; 

    void FastCommandOpenGate() const;
    void ClearAndResetStatusRegisters() const;
    void EnableSequencerReadout() const;
    void DisableSequencerReadout() const;
    void SendSoftwareRDFE() const; // manually start sequencer readout
    void WaitForSequencerReadoutCompletion() const;

    void SequencerDelayDisable() const;
    void SequencerDelayEnable() const;
    void SequencerDelayValue( unsigned short delay ) const; // 9 lowest bits
};

#endif
