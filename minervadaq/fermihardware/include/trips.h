#ifndef trips_h
#define trips_h

#include <vector>
#include <iostream>

#include "Frames.h"

/*********************************************************************************
* Class for creating Trip-t objects for use with the 
* MINERvA data acquisition system and associated software projects.
*
* Elaine Schulte, Rutgers University
* Gabriel Perdue, The University of Rochester
**********************************************************************************/

/*! \class trips
 *
 * \brief This class holds all data associated with a TRiP-T on an FEB.
 *
 */
class trips : public Frames {
	private:
		unsigned char *buffer; /*!< an object specific buffer */
		unsigned char TripTChipID[1], TripTRead[1], TripTWrite[1]; /*!< command registers */
		/*! the following table describes the 15 entries in trip_values 
		*     These are the *logical* registers.
		*     - 0:  ibp:  preamp drive current (register addres 1)
		*     - 1:  ibbnfoll:  preamp feedback controll (register addres 2)
		*     - 2:  iff: preamb feadback control? (register addres 3)
		*     - 3:  ibpiff1ref:  preamp reset strength (register address 4)
		*     - 4:  ibopamp: opamp drive current (register address 5) 
		*     - 5:  ib_t: time circuit current source (register address 6)
		*     - 6:  iffp2:  opamp feedback control (register address 7)
		*     - 7:  ibcomp:  comparator drive current (register address 8)
		*     - 8:  v_ref:  reference voltage for opamps 2 & 3 (register address 9)
		*     - 9:  v_th:  reference voltage for comparator (register address 10)
		*     - 10: pipedelay:  pipeline depth (register address 11 bits 0-5)
		*     - 11: gain: gain (register address 11 bits 6-9)
		*     - 12: irsel: read drive currents (register 12 bits 2,3)
		*     - 13: iwsel: write drive currents (register 12 bits 0,1)
		*     - 14: inject: channels to test pulse! (register 14, which can be up to 34 bits long...I think)
		*         we may have to do something about this...
		*/
		long_m trip_values[15];  /*!<these are the values to be written to the registers */

		/*!
		*    Note that the *physical* registers are listed slightly differently...(these are the addresses)
		*	- 00 - All
		*	- 01 - IBP
		*	- 02 - IBBNFoll
		*	- 03 - IFF
		*	- 04 - IBPIFF1REF
		*	- 05 - IBOPAMP
		*	- 06 - IBPFoll2
		*	- 07 - IFF2
		*	- 08 - IBCOMP
		*	- 09 - VREF
		*	- 10 - VTH
		*	- 11 - GAINPIPEDEL
		*	- 12 - IRSELIWSEL
		*	- 13 - NOT USED
		*	- 14 - INJECT -> WRITE ONLY!!!
		*	- 15 - INVALID
		*/ 
		long_m trip_registers[14]; /*!<these represent the actual registers to which data is written */
		unsigned char trip_function; /*!< Trip to be addressed*/
		bool read; /*!< read(true) or write(false) to the trip registers */
		int bufferSize; /*! the length of the trip buffer */

                // log4cpp appender for printing log statements.
                log4cpp::Appender* tripsAppender;

	public:
		/*! specific constructor */
		trips(febAddresses a, TRiPFunctions f, int a, log4cpp::Appender* appender);
		/*! default destructor */
		~trips() { };
		/*! Function to assign a value to a logical trip_value entry */
		void inline SetRegisterValue(int index, int value) {trip_values[index]=value;};
		/*! Function to decode the register values (inherited from Frames) */
		int DecodeRegisterValues(int a);
		/*! Function to encode register values into special trip patters */
		void EncodeRegisterValues();
		/*! Function to pack the bits into the register values for trip patterns */
		void PackBits(bool control, bool clock, bool data, bool reset, 
			std::vector<unsigned char> &l);
		/*! Function to sort bits for packing */
		void SortNSet(bool reset, long_m data, int bits, bool control, 
			bool lowToHigh, std::vector<unsigned char> &l);
			/*! Function to parse return buffers for error bits */
		void ParseError(int i, int index);
		/*! Function to parse return buffers for error bits */
		void ParseError(int i);
		/*! Function to return a register value from the trips */
		void GetRegisterValue(int j,int &i, int b);
		/*! Function to make an outgoing message to trips (inherited from Frames) */
		void MakeMessage();
		/*! Function to clean up memory */
		void DeleteBuffer() {delete [] buffer;};
		/*! Function to filp the read/write message bit */
		void inline SetRead(bool a) {read = a;};
		/*! Function to return the lenght of the outgoing buffer */
		int inline GetMessageSize() {return bufferSize+MinHeaderLength;};
		/*! Function to return a trip register value */
		long_m inline GetTripValue(int i) {return trip_values[i];};  

};

#endif
