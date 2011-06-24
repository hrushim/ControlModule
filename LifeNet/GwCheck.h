/* 
 * File:   GwCheck.h
 * Author: hrushi
 *
 * Created on June 18, 2011, 8:35 PM
 */

#ifndef GWCHECK_H
#define	GWCHECK_H

class GwCheck {
public:
    GwCheck();
    GwCheck(const GwCheck& orig);
    virtual ~GwCheck();
    void static run(void *ptr);
    void static isRunning(bool value);
private:
    bool RUN_FLAG;
};

#endif	/* GWCHECK_H */

