#ifndef FrontEndBoard_h
#define FrontEndBoard_h

#include "TRIPFrame.h"
#include "ADCFrame.h"
#include "DiscrFrame.h"
#include "FPGAFrame.h"


/*! \class FrontEndBoard
 *
 * \brief The class which holds all of the information associated with an FrontEndBoard.
 *
 */

class FrontEndBoard {

  private:
    FrameTypes::febAddresses boardNumber; 
    TRIPFrame  * tripFrame;    
    ADCFrame   * adcFrame;      
    DiscrFrame * discrFrame;   
    FPGAFrame  * fgpaFrame;


  public:
    FrontEndBoard( FrameTypes::febAddresses theAddress ); 
    ~FrontEndBoard() { };    

    FrameTypes::febAddresses inline GetBoardNumber() { return boardNumber; };

    std::tr1::shared_ptr<FPGAFrame> GetFPGAFrame();
    std::tr1::shared_ptr<TRIPFrame> GetTRIPFrame(int tripNumber); 
    std::tr1::shared_ptr<ADCFrame> GetADCFrame(int hitBlock); 
    std::tr1::shared_ptr<DiscrFrame> GetDiscrFrame(); 

};

#endif
