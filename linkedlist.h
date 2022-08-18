#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node{
    char* data;
    struct node* next;
}node_t;

// Data structure for a linked list
// Holds the first node in our list
typedef struct linkedlist{
    node_t* head;
}linkedlist_t;

inkedlist_t* CreateLinkedListOfFiveItems();

// The job of this function is to create a list.
inkedlist_t* CreateLinkedList();

// PrintLinkedList takes a pointer to a LinkedList as the
// input. This function will iterate through a linked
// list.
oid PrintLinkedList(linkedlist_t* list);

// This function will 'free' the linked list that is
// passed in. The goal is to 'delete' all of the node_t's
// (including the 'head' in the linkedlist_t) data structure.
oid FreeLinkedList(linkedlist_t* list);

// The purpose of this function is to append a new node to the end of the linked list.
oid AppendToLinkedList(linkedlist_t* list,char* data);

#endif
