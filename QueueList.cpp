#include "QueueList.h"

#include "Customer.h"

QueueList::QueueList() { head = nullptr; }

QueueList::QueueList(Customer* customer) { head = customer; }

QueueList::~QueueList() {
  delete head; // do I need this?
}

Customer* QueueList::get_head() {return head;}

void QueueList::enqueue(Customer* customer) {
  // a customer is placed at the end of the queue
  // if the queue is empty, the customer becomes the head
  if (head == nullptr){ head = customer;}
  else{
    Customer* current = head;
    
    // not given tail => traverse through using current
    while(current -> get_next() != nullptr){
      current = current -> get_next();
    }
    current -> set_next(customer);
  }
  customer -> set_next(nullptr);
}

Customer* QueueList::dequeue() {
  // remove a customer from the head of the queue 
  // and return a pointer to it
  if(head == NULL) return NULL;
  Customer* temp = head;
  head = head -> get_next();
  temp -> set_next(NULL);
  return temp;
}

int QueueList::get_items() {
  // count total number of items each customer in the queue has
  Customer* current = head;
  int itemCount;
  while (current -> get_next() != NULL){
    itemCount += current -> get_numOfItems();
    current = current -> get_next();
  }
  itemCount += current -> get_numOfItems();
  return itemCount;
}

void QueueList::print() {
  // print customers in a queue
  Customer* temp = head;
  while (temp != nullptr) {
    temp->print();
    temp = temp->get_next();
  }
}
