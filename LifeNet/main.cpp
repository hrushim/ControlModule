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
        printf("\n\nsudo ./lifenet <nodename> <ifacename> <wanifacename> \"<ipaddress>\" <session_interval>\n\n");
        exit(0);
    }

    cout << "Arguments entered:\n";
    cout << argv[1] << "\n";
    cout << argv[2] << "\n";
    cout << argv[3] << "\n";
    cout << argv[4] << "\n";
    cout << argv[5] << "\n";
    fflush(stdout);

    MyInfo myInfo(argv[1], argv[2], argv[3], argv[4]);

    netStats.initStats();

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

