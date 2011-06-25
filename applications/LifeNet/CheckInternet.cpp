/* 
 * File:   CheckInternet.cpp
 * Author: hrushi
 * 
 * Created on June 18, 2011, 11:21 PM
 */

#include "CheckInternet.h"

pthread_t CheckInternet::tGwCheck;
bool CheckInternet::connectedToInternet;

CheckInternet::CheckInternet() {
}

CheckInternet::CheckInternet(const CheckInternet& orig) {
}

CheckInternet::~CheckInternet() {
}

