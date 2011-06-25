/* 
 * File:   SessionTimer.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 10:39 PM
 */

#ifndef SESSIONTIMER_H
#define	SESSIONTIMER_H

#define PRINT_SESSION 0

#include <sys/time.h>
#include <signal.h>
#include <iostream>
#include<stdio.h>

#include "NetworkStats.h"

using namespace std;

class SessionTimer {
public:

    SessionTimer();
    SessionTimer(const SessionTimer& orig);
    virtual ~SessionTimer();

    static void startTimer(int interval) {
    NetworkStats netStats;

        iTimer.it_interval.tv_sec = interval;
        iTimer.it_interval.tv_usec = 0;
        iTimer.it_value.tv_sec = interval;
        iTimer.it_value.tv_usec = 0;

        netStats.sessionNum = netStats.lastSessionNum = 0;

        setitimer(ITIMER_REAL, &iTimer, NULL);
        signal(SIGALRM, signalHandler);
    }

private:

    static struct itimerval iTimer;

    static void signalHandler(int cause) {
        NetworkStats netStats;

        netStats.numLastTx = netStats.numTx;
        netStats.numTx = 0;

        netStats.lastSessionNum = netStats.sessionNum;
        netStats.sessionNum++;

#if PRINT_SESSION
        cout << "\n\nTimer expired";
        printf(" TX=%d SESS=%d LAST_SESS=%d\n\n", netStats.numLastTx, netStats.sessionNum, netStats.lastSessionNum);
#endif

        fflush(stdout);
    }
};

#endif	

