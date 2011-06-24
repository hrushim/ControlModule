/* 
 * File:   Sniffer.cpp
 * Author: hrushi
 * 
 * Created on June 21, 2011, 6:27 PM
 */

#include "Sniffer.h"

pthread_t Sniffer::tSniff;
int Sniffer::tolerance;

Sniffer::Sniffer() {
}

Sniffer::Sniffer(const Sniffer& orig) {
}

Sniffer::~Sniffer() {
}

