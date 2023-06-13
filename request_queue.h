#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

// Structure to represent an item in the RequestQueue
typedef struct {
    int source;
    int clock;
} Request;

// Structure to represent the RequestQueue
typedef struct {
    Request* items;
    int size;
    int capacity;
} RequestQueue;

// Function to initialize a new RequestQueue
void initRequestQueue(RequestQueue* queue, int capacity);

// Function to add a new item to the RequestQueue
void add(RequestQueue* queue, int source, int clock);

// Function to remove the item with the provided source and lowest clock
void removeItem(RequestQueue* queue, int source);

// Function to get the source of the first item in the RequestQueue
int getFirstSource(RequestQueue* queue);

// Function to print the contents of the RequestQueue
void printRequestQueue(RequestQueue* queue);

// Function to deallocate memory used by the RequestQueue
void freeRequestQueue(RequestQueue* queue);

#endif