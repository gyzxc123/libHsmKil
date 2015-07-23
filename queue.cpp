#define LOG_TAG "Queue"
#if !defined(NO_DEBUG) && !defined(QUEUE_BUFFER_DEBUG_ENABLED)
#warning "QUEUE_BUFFER_DEBUG_ENABLED not defined, therfore no queue_buffer debug"
#define NO_DEBUG	1
#endif

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/**
 * Push an item into queue, if this is the first item,
 * both queue->head and queue->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
void push (queue* queue, int item) {
	// Create a new node
	node* n = (node*) malloc (sizeof(node));
	n->item = item;
	n->next = NULL;

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	DBG_OUT1("push item = %d\n", item);
#endif

	if (queue->head == NULL) { // no head
	queue->head = n;
	} else{
	queue->tail->next = n;
	}
	queue->tail = n;
	queue->size++;

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	// Display the current queue items (only if debug is enabled)
	display(queue);
#endif
}

/**
 * Return and remove the first item.
 */
int pop (queue* queue) {
	// get the first item
	node* head = queue->head;
	int item = head->item;
	// move head pointer to next node, decrease size
	queue->head = head->next;
	queue->size--;
	// free the memory of original head
	free(head);

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	DBG_OUT1("pop item = %d\n", item);
#endif

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	// Display the current queue items (only if debug is enabled)
	display(queue);
#endif

	return item;
}

/**
 * Return but not remove the first item.
 */
int peek (queue* queue) {
    node* head = queue->head;
    return head->item;
}

/**
 * Show all items in queue.
 */
void display (queue* queue) {
	int length = 0;
	char str[80];

	length += sprintf(str+length, "Queue Display: ");

	// no item
	if (queue->size == 0) {
		length += sprintf(str+length, "No items in queue.");
	}
	else { // has item(s)
		node* head = queue->head;
		int i, size = queue->size;
		length += sprintf(str+length, "%d item(s): ", queue->size);
		for (i = 0; i < size; i++) {
			if (i > 0) {
				length += sprintf(str+length, ", ");
			}
			length += sprintf(str+length, "%d", head->item);
			head = head->next;
		}
	}	
	
	length += sprintf(str+length,"\n");	

	DBG_OUT1(str);
}

/**
 * Create and initiate a Queue
 */
queue createQueue () {
#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	DBG_OUT1("%s++\n", __FUNCTION__);
#endif
	queue queue;
	queue.size = 0;
	queue.head = NULL;
	queue.tail = NULL;
	queue.push = &push;
	queue.pop = &pop;
	queue.peek = &peek;
	queue.display = &display;

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	// Display the current queue items (only if debug is enabled)
	display(&queue);
#endif

#ifdef QUEUE_BUFFER_DEBUG_ENABLED
	DBG_OUT1("%s--\n", __FUNCTION__);
#endif
	return queue;
}
