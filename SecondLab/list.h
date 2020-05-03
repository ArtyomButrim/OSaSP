//
// Created by artyom on 1.05.20.
//

#ifndef UNTITLED_LIST_H
#define UNTITLED_LIST_H

#include <stdlib.h>

typedef struct ARRAY_lIST {
    int size;
    char **elements;
}ArrayList;

ArrayList *createList();
int addToList(ArrayList *arrayList, char *newElement);
char * getFromList(ArrayList *arrayList, int index);

#endif //UNTITLED_LIST_H
