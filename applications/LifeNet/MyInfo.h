/* 
 * File:   MyInfo.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 2:47 PM
 */

#ifndef MYINFO_H
#define	MYINFO_H

#include<stdio.h>
#include <string.h>

// This class stores the information of the current host node

class MyInfo {
public:
    static char nodeName[10];
    static char iFaceName[10];
    static char wanIfaceName[10];
    static char ipAddress[20];
    //static char macAddress[20];

    MyInfo();
    MyInfo(char *nName, char *iFace, char *wanIface, char *ipAdd);
    MyInfo(const MyInfo& orig);
    virtual ~MyInfo();

private:

};

#endif	/* MYINFO_H */

