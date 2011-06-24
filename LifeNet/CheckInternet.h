/* 
 * File:   CheckInternet.h
 * Author: hrushi
 *
 * Created on June 18, 2011, 11:21 PM
 */

#ifndef CHECKINTERNET_H
#define	CHECKINTERNET_H

#include<stdio.h>
#include<iostream>
#include<stdint.h>
#include <pthread.h>
#include <sched.h>
#include<string.h>

#include"FileOps.h"
#include "LkmOps.h"
#include "MyInfo.h"

using namespace std;

// This is class checks whether the current node is connected to the internet or not. If yes, then this node should
// automatically become the gateway.

class CheckInternet {

private:

    static pthread_t tGwCheck;

    // This function is actually invoked by the thread.
    // It periodically pings a website through the WAN interface and checks if the ping goes through.

    static void *checkPresenceOfGw(void *ptr) {

        LkmOps lkmOps;
        char line[80], command_str[100];
        FileOps fileOps;

        while (lkmOps.checkManifoldLkmStatus()) {
            memset(&line, 0, sizeof (line));
            memset(command_str, '\0', 100);
            sprintf(command_str, "ping -I %s -c 1 www.google.com 2> /tmp/inject_log | grep \"ttl=\"", (char *) ptr);
            if (fileOps.first_line_exists_in_file(command_str) == 1)
                connectedToInternet = true;
            else
                connectedToInternet = false;
           // cout << "Connected to Internet? " << connectedToInternet << "\n";
            fflush(stdout);
            sched_yield();
        }
    }

public:
    static bool connectedToInternet;
    CheckInternet();
    CheckInternet(const CheckInternet& orig);
    virtual ~CheckInternet();

    static void run() {
        MyInfo myInfo;
        pthread_create(&tGwCheck, NULL, CheckInternet::checkPresenceOfGw, (void *) myInfo.wanIfaceName);
    }

    static void join() {
        pthread_join(CheckInternet::tGwCheck, NULL);
    }
};


#endif	/* CHECKINTERNET_H */

