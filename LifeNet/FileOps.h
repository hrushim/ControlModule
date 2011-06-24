/* 
 * File:   FileOps.h
 * Author: hrushi
 *
 * Created on June 18, 2011, 9:51 PM
 */

#ifndef FILEOPS_H
#define	FILEOPS_H

#define MAX_NUM_DNS_TX 3

#include<stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

class FileOps {
public:
    FileOps();
    FileOps(const FileOps& orig);
    virtual ~FileOps();
    bool first_line_exists_in_file(char *file_name);
    int readFromFirstThreeLinesOfFileHavingPattern(char *buf1, char *buf2, char *buf3, char *file_name, char *pattern, int token_num);
private:

};

#endif	/* FILEOPS_H */

