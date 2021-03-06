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

// The entire code is organized into different classes. Each class is dedicated to a particular function that it mostly performs in a
// thread. The main function simply calls the initialization function and starts all threads.

int main(int argc, char** argv) {

    NetworkStats netStats;
    CheckInternet checkInternet;
    Injector injector;
    Sniffer sniffer;
    SessionTimer sessiosTimer;

    if(argc < 4) {
        printf("\n\nsudo ./lifenet <nodename> <ifacename> <wanifacename> \"<ipaddress>\" <session_interval> <injection-interval> <tolerance>\n\n");
        exit(0);
    }

    // parameters that should be added - injection interval, tolerance ED update
#if PRINT_MAIN
    cout << "Arguments entered:\n";
    cout << "1." << argv[1] << "\n";
    cout << "2." << argv[2] << "\n";
    cout << "3." << argv[3] << "\n";
    cout << "4." << argv[4] << "\n";
    cout << "5." << argv[5] << "\n";
    cout << "6." << argv[6] << "\n";
    cout << "6." << argv[7] << "\n";
    fflush(stdout);
#endif

    MyInfo myInfo(argv[1], argv[2], argv[3], argv[4]);

    // Commented because causing segfault
//    netStats.initStats();

    uint8_t injInterval = atoi(argv[6]);
    injector.injectionInterval = injInterval;

    uint8_t tolerance = atoi(argv[7]);
    netStats.edUpdateTolerance = tolerance;
    
    injector.run();
    sniffer.run();
    checkInternet.run();

    int sessInterval = atoi(argv[5]);
    sessiosTimer.startTimer(sessInterval);
    netStats.run();

    checkInternet.join();
    injector.join();
    sniffer.join();
    netStats.join();

    return 0;
}

