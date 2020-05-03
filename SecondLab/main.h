//
// Created by artyom on 1.05.20.
//

#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <bits/types/FILE.h>
#include <time.h>
#include <limits.h>

#include "list.h"

typedef struct FILE_INFO {
    size_t fileSize;
    char *fileContent;
    unsigned long accessType;
    long indexNumberDescriptor;
    char *creationDate;
} FileInfo;

void readDirectory(char *directoryPath, ArrayList *firstList, char *executableName);

FileInfo *getFileInfo(char *filePath, int *buffersAmount);

bool areFilesEqual(FileInfo *firstFileInfo, FileInfo *secondFileInfo);

void getProp(struct stat buf, char *filepath);

#endif //UNTITLED_MAIN_H
