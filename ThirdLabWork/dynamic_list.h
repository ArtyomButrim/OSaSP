//
// Created by artyom on 9.05.20.
//

#ifndef THIRDLABWORK_DYNAMIC_LIST_H
#define THIRDLABWORK_DYNAMIC_LIST_H

#include <stdlib.h>

typedef struct ARRAY_lIST {
    int size;
    int *elements;
}DynList;

DynList *createList();
int addToList(DynList *list, int newElement);
int getFromList(DynList *list, int index);

#endif //THIRDLABWORK_DYNAMIC_LIST_H
