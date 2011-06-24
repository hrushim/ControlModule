/* 
 * File:   main.cpp
 * Author: hrushi
 *
 * Created on June 18, 2011, 8:04 PM
 */

#include "main.h"
#include "FileOps.h"
#include "CheckInternet.h"
#include "Injector.h"
#include "MyInfo.h"
#include "SessionTimer.h"
#include "Sniffer.h"

using namespace std;




/*
 * 
 */
int main(int argc, char** argv) {

    NetworkStats netStats;
    CheckInternet checkInternet;
    Injector injector;
    Sniffer sniffer;
    SessionTimer sessiosTimer;

    cout << "Arguments entered:\n";
    cout << argv[1] << "\n";
    cout << argv[2] << "\n";
    cout << argv[3] << "\n";
    fflush(stdout);

    MyInfo myInfo(argv[1], argv[2], argv[3]);

    //netStats.initStats();

    injector.run();
    sniffer.run();
    checkInternet.run();
    sessiosTimer.startTimer(30);

    checkInternet.join();
    injector.join();
    sniffer.join();

    return 0;
}

