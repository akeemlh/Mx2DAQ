/*----------------------------------------------------------------------------*
 *  Copyright (c) 2001        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Author:  Carl Timmer                                                    *
 *             timmer@jlab.org                   Jefferson Lab, MS-12H        *
 *             Phone: (757) 269-5130             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*/

package org.jlab.coda.et.exception;
import java.lang.*;

/**
 * This class represents a general error of an ET system.
 *
 * @author Carl Timmer
 */

public class EtException extends Exception {

    /**
     * Create an exception indicating an error specific to the ET system.
     * {@inheritDoc}<p/>
     *
     * @param message {@inheritDoc}<p/>
     */
    public EtException(String message) {
        super(message);
    }

}
