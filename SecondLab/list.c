#include "list.h"

ArrayList *createList() {
    ArrayList *arrayList = malloc(sizeof(ArrayList));

    if (arrayList ==NULL) {
        return NULL;
    }

    arrayList->elements = malloc(sizeof(char *));

    if (arrayList->elements == NULL) {
        free(arrayList);
        return NULL;
    }

    return arrayList;
}

int addToList (ArrayList *arrayList, char *newElement) {
    if (arrayList == NULL || newElement == NULL) {
        return -1;
    }

    int newElementIndex = arrayList->size;
    arrayList->size++;
    arrayList->elements = realloc(arrayList->elements, arrayList->size*sizeof(char *));

    if (arrayList->elements == NULL) {
        return -1;
    }

    arrayList->elements[newElementIndex] = newElement;
    return 0;
}

char *getFromList(ArrayList *arrayList, int index) {
    if (arrayList ==NULL || index > arrayList->size -1) {
        return NULL;
    }
    return arrayList->elements[index];
}