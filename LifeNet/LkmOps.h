/* 
 * File:   LkmOps.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 1:34 PM
 */

#ifndef LKMOPS_H
#define	LKMOPS_H

#include<stdio.h>
#include <stdint.h>

// This class performs all the functions that are directly related to the kernel module

class LkmOps {
public:
    LkmOps();

    LkmOps(const LkmOps& orig);

    virtual ~LkmOps();

    static uint8_t checkManifoldLkmStatus() {
        return 1;
    }
    
private:

};

#endif	/* LKMOPS_H */

