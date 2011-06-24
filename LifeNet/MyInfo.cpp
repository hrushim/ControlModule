/* 
 * File:   MyInfo.cpp
 * Author: hrushi
 * 
 * Created on June 19, 2011, 2:47 PM
 */

#include "MyInfo.h"

char MyInfo::nodeName[10];
char MyInfo::iFaceName[10];
char MyInfo::ipAddress[20];

MyInfo::MyInfo() {
}

MyInfo::MyInfo(char *nName, char *iFace, char *ipAdd) {
    strncpy(nodeName, nName, 10);
    strncpy(iFaceName, iFace, 10);
    strncpy(ipAddress, ipAdd, 20);
}

MyInfo::MyInfo(const MyInfo& orig) {
}

MyInfo::~MyInfo() {
}

