/* 
 * File:   FileOps.cpp
 * Author: hrushi
 * 
 * Created on June 18, 2011, 9:51 PM
 */

#include "FileOps.h"

FileOps::FileOps() {
}

FileOps::FileOps(const FileOps& orig) {
}

FileOps::~FileOps() {
}


int FileOps::readFromFirstThreeLinesOfFileHavingPattern(char *buf1, char *buf2, char *buf3, char *file_name, char *pattern, int token_num) {

    int i = 0;
    FILE *fptr;
    char line[80];

    memset(&line, '\0', sizeof (line));
    fptr = fopen(file_name, "r");
    if (fptr == NULL) {
        printf("\nCannot open %s. Terminating...", file_name);
        exit(1);
    }

    while (fgets(line, 80, fptr) != NULL) {
        char *nameserver = NULL, *buf = NULL;

        nameserver = strtok(line, " \n\t");
        buf = strtok(NULL, " \n\t");

        if (strncmp(nameserver, "nameserver", 10) == 0) {
            if (i == 0) {
                strncpy(buf1, buf, 16);
            } else if (i == 1) {
                strncpy(buf2, buf, 16);
            } else if (i == 2) {
                strncpy(buf3, buf, 16);
            }
            i++;
            if (i == MAX_NUM_DNS_TX) {
                break;
            }
        }
        memset(&line, '\0', 80);
    }
    if (fptr) {
        fclose(fptr);
    }

    return i;

}


bool FileOps::first_line_exists_in_file(char *file_name) {

    FILE *fptr;
    char buf[80];

    fptr = popen(file_name, "r");
    if (fptr == NULL) {
        printf("\nCould not open %s. Returning 0...", file_name);
        return 0;
    }

    if (fgets(buf, 80, fptr) == NULL) {
#if DEBUG
        printf("\n\tCould not read from %s. Returning 0...", file_name);
        fflush(stdout);
#endif
        pclose(fptr);
        return 0;
    }

    pclose(fptr);
    return 1;

}