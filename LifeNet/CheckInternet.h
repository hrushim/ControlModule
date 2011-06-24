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

using namespace std;

class CheckInternet {

private:

    static pthread_t tGwCheck;

    static void *checkPresenceOfGw(void *ptr) {

        LkmOps lkmOps;
        char line[80], command_str[100];
        FileOps fileOps;

        while (lkmOps.checkManifoldLkmStatus()) {
            memset(&line, 0, sizeof (line));
            memset(command_str, '\0', 100);
            sprintf(command_str, "ping -I %s -c 1 www.mymanet.org 2> /tmp/inject_log | grep \"ttl=\"", (char *) ptr);
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
        pthread_create(&tGwCheck, NULL, CheckInternet::checkPresenceOfGw, (void *) "wlan0");
    }

    static void join() {
        pthread_join(CheckInternet::tGwCheck, NULL);
    }
};


#endif	/* CHECKINTERNET_H */

