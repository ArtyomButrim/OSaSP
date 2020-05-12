//
// Created by artyom on 9.05.20.
//

#include "dynamic_list.h"

DynList *createList() {
    DynList *list = malloc(sizeof(DynList));

    if (list ==NULL) {
        return NULL;
    }

    list->elements = malloc(sizeof(int));

    if (list->elements == NULL) {
        free(list);
        return NULL;
    }

    return list;
}

int addToList (DynList *list, int newElement) {
    if (list == NULL) {
        return -1;
    }

    int newElementIndex = list->size;
    list->size++;
    list->elements = realloc(list->elements, list->size*sizeof(int));

    if (list->elements == NULL) {
        return -1;
    }

    list->elements[newElementIndex] = newElement;
    return 0;
}

int getFromList(DynList *list, int index) {
    if (list ==NULL || index > list->size -1) {
        return -1;
    }
    return list->elements[index];
}