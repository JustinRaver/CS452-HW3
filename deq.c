/*
 * Author: Justin Raver
 * Class: CS452
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];		// next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];			// head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

/*
 * Paramaters: 
 * Rep r the list to be added to
 * End e the end of the list to add to
 * Data d the data to be contained in the node added
 * 
 * This function takes a piece of data creates a new node containing
 * it and then adds the new node to the e end (either head or tail)
 */
static void put(Rep r, End e, Data d) {
  // create space on the heap for the node to be added
  Node node = (Node) malloc(sizeof(*node));
  if (!node) ERROR("malloc() failed");
  node->data = d;

  if (r->len > 0) {
    int opp = (e == Head ? Tail : Head);

    node->np[opp] = r->ht[e]; // set new nodes tail to current head
    r->ht[e]->np[e] = node; // set head of current head to new node
    r->ht[e] = node;// set rep head to new node
    node->np[e] = NULL; //set new nodes tail to null
  } else {
    // set head and tail of new node to null
    node->np[Head] = node->np[Tail] = NULL;
    // set reps head and tail to new node
    r->ht[Head] = r->ht[Tail] = node;
  }
  // increment rep counter 
  r->len += 1;
}

/*
 * Parameters:
 * Rep r the list to search
 * End e the end of the list (either head or tail)
 * int i the index of the data to return based 0 from either end
 * 
 * finds the ith node from e end (either head or tail) and then
 * returns its data
 */
static Data ith(Rep r, End e, int i) { 
  // check for valid index base 0
  if (i > (r->len)-1 || i < 0) return 0;
  // create node to contain return result
  Node node = r->ht[e];
  // iterate through the list until the ith node is found
  for(int j = 0 ; j != i ; j++){
    node = node->np[(e == Head ? Tail : Head)];
  }  
  return node->data; 
}

/*
 * Rep r the list to search
 * End e the end of the list (either head or tail)
 * Data d the data of the node to remove
 * 
 * Removes the first instance of the data found from the starting end,
 * frees the node, and returns the data
 */
static Data rem(Rep r, End e, Data d) { 
  if (r->len == 0) return 0;
  // node to store the current node while iterating
  Node node = r->ht[e];

  // iterate from first node of end till found or null
  int j;
  for (j=0 ; node->data != d ; j++) {
    node = node->np[e == Head ? Tail : Head];
    if (node == NULL) return 0;
  }

  // opposite end to e
  int opp = (e == Head ? Tail : Head);

  if (r->len == 1) {
    r->ht[Head] = r->ht[Tail] = NULL;
  } else if (j == 0) {
    // head node case
    r->ht[e] = node->np[opp]; // replace Head in queue
    r->ht[e]->np[e] = NULL; // set new Heads head null
  } else if (j == (r->len)-1){
    // tail node case
    r->ht[opp] = node->np[e];
    r->ht[opp]->np[opp] = NULL;
  } else {
    // center node case
    node->np[Tail]->np[Head] = node->np[Head]; // tails head to head
    node->np[Head]->np[Tail] = node->np[Tail]; // heads tail to tail
  }

  // free the node
  free(node);
  // decrements the size of r
  r->len -= 1;
  return d; 
}

/*
 * Rep r the list to search
 * End e the end of the list (either head or tail)
 * 
 * gets and removes the node at e from r and returns its data
 * returns 0 if r's len is 0
 */
static Data get(Rep r, End e) { 
  // uses the rem function to get and return the desired data
  return r->len == 0 ? 0 : rem(r, e, r->ht[e]->data);
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head); }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail); }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
