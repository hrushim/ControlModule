/* 
 * File:   Injector.cpp
 * Author: hrushi
 * 
 * Created on June 19, 2011, 12:49 AM
 */

#include "Injector.h"

uint8_t Injector::injectionInterval;
pthread_t Injector::tInject;
int Injector::packetLen;

Injector::Injector() {
}

Injector::Injector(const Injector& orig) {
}

Injector::~Injector() {
}

