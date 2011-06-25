/* 
 * File:   GwCheck.cpp
 * Author: hrushi
 * 
 * Created on June 18, 2011, 8:35 PM
 */

#include "GwCheck.h"

GwCheck::GwCheck() {
    RUN_FLAG = false;
}

GwCheck::GwCheck(const GwCheck& orig) {
}

GwCheck::~GwCheck() {
}

void GwCheck::run(void *ptr) {

}

void GwCheck::isRunning(bool value) {
    RUN_FLAG = value;
}