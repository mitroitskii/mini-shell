#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"


linkedlist_t* CreateLinkedListOfFiveItems(){
	linkedlist_t* list = CreateLinkedList();
	int i;
	for (i = 0; i < 5; i++) {
		AppendToLinkedList(list, NULL);	
	}
	return list; 
}

linkedlist_t* CreateLinkedList(){
	// allocating memory for a new linked list
	linkedlist_t* list = (linkedlist_t*)malloc(sizeof(list));
	// assigning null as a head value of the new list
	list->head = NULL;
	return list;
}

void PrintLinkedList(linkedlist_t* list){
	node_t* iter = list->head;
	while (iter != NULL) {
		printf("%s", iter->data);
		iter = iter->next;
	}	
}

void FreeLinkedList(linkedlist_t* list){
	node_t* current = list->head;
	node_t* next;
	// we are freeing nodes until the next value is null
	while(current != NULL) {
		next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	// freeing up the list itself
	free(list);
}

node_t* TraverseLinkedList(node_t* head) {
	node_t* iterator = head;
	while (iterator->next != NULL) {
		iterator = iterator->next;
	}
	return iterator;
}

void AppendToLinkedList(linkedlist_t* list, char* data){
	// create a new node with the given data
	node_t* newNode = (node_t*)malloc(sizeof(node_t));
	// allocate memory for the node data
	char* buf = (char*)malloc(strlen(data)*sizeof(char*));
	// copy data over to buf
	strcpy(buf, data);
	// set buf as data for the node
	newNode->data = buf;
	newNode->next = NULL;
	// check if the head of the list is null
	if (list->head == NULL) {
		list->head = newNode;
	} else {
	node_t* tail = TraverseLinkedList(list->head);
	tail->next = newNode;	
	}
}

