#include "request_queue.h"
#include "util.h"

void initRequestQueue(RequestQueue* queue, int capacity) {
    queue->items = (Request*)malloc(capacity * sizeof(Request));
    queue->size = 0;
    queue->capacity = capacity;
}

void add(RequestQueue* queue, int source, int clock) {
    if (queue->size == queue->capacity) {
        // Queue is full, resize the items array
        queue->capacity *= 2;
        queue->items = (Request*)realloc(queue->items, queue->capacity * sizeof(Request));
    }

    // Find the index to insert the new item
    int insertIndex = 0;
    while (insertIndex < queue->size && 
           (queue->items[insertIndex].clock < clock ||
            (queue->items[insertIndex].clock == clock && queue->items[insertIndex].source < source))) {
        insertIndex++;
    }

    // Shift items to the right to make room for the new item
    for (int i = queue->size - 1; i >= insertIndex; i--) {
        queue->items[i + 1] = queue->items[i];
    }

    // Insert the new item at the correct position
    queue->items[insertIndex].source = source;
    queue->items[insertIndex].clock = clock;

    queue->size++;
}

void removeItem(RequestQueue* queue, int source) {
    int removeIndex = -1;

    // Find the index of the item with the provided source and lowest clock
    for (int i = 0; i < queue->size; i++) {
        if (queue->items[i].source == source && (removeIndex == -1 || queue->items[i].clock < queue->items[removeIndex].clock)) {
            removeIndex = i;
        }
    }

    if (removeIndex != -1) {
        // Shift items to the left to remove the item
        for (int i = removeIndex; i < queue->size - 1; i++) {
            queue->items[i] = queue->items[i + 1];
        }

        queue->size--;
    }
}

int getFirstSource(RequestQueue* queue) {
    if (queue->size > 0) {
        return queue->items[0].source;
    } else {
        return -1;
    }
}

int getPosition(RequestQueue* queue, int source) {
    for (int i = 0; i < queue->size; i++) {
        if (queue->items[i].source == source) {
            return i;
        }
    }

    return queue->size;
}

void printRequestQueue(RequestQueue* queue) {
    debug("RequestQueue Contents:\n");
    for (int i = 0; i < queue->size; i++) {
        debug("Item %d: Source = %d, Clock = %d\n", i, queue->items[i].source, queue->items[i].clock);
    }
}

void freeRequestQueue(RequestQueue* queue) {
    free(queue->items);
    queue->items = NULL;
    queue->size = 0;
    queue->capacity = 0;
}