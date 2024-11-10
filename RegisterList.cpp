#include "RegisterList.h"

#include <iostream>

#include "Register.h"
using namespace std;

RegisterList::RegisterList() {
  head = nullptr;
  size = 0;
}

RegisterList::~RegisterList() {
  delete head;
}

Register* RegisterList::get_head() {return head;}

int RegisterList::get_size() { 
  return size;
}


Register* RegisterList::get_min_items_register() {
  if (head == nullptr) {
    return nullptr;  // Handle the case where the list is empty
  }
  // loop all registers to find the register with least number of items
  Register* minItems = head;
  Register* current = head;
  while(current -> get_next() != NULL){
    if(current -> get_queue_list() -> get_items() < minItems -> get_queue_list() -> get_items()){
      minItems = current;
    }
    current = current -> get_next();
  }
  return minItems;
}

Register* RegisterList::get_free_register() {
  // return the register with no customers
  // if all registers are occupied, return nullptr
  Register* current = head;
  while (current != nullptr) {
    // Check if the current register has no customers in the queue
    if (current->get_queue_list()->get_head() == nullptr) {
      return current;
    }
    current = current->get_next(); 
  }
  return nullptr;
}

void RegisterList::enqueue(Register* newRegister) {
  // a register is placed at the end of the queue
  // if the register's list is empty, the register becomes the head
  // Assume the next of the newRegister is set to null
  // You will have to increment size 
  newRegister -> set_next(nullptr);
  if (head == NULL) {
    head = newRegister;
    
  }else{
    Register* current = head;
    while (current -> get_next() != nullptr){
      current = current -> get_next();
    }
    current -> set_next(newRegister);
  }
  size ++;
}

bool RegisterList::foundRegister(int ID) {
  // look for a register with the given ID
  // return true if found, false otherwise
  Register* current = head;
  while (current != nullptr){
    if (current -> get_ID() == ID){
      return true;
    }
    current = current -> get_next();
  }
  return false;

}

Register* RegisterList::dequeue(int ID) {
  // dequeue the register with given ID

  // return the dequeued register
  // return nullptr if register was not found

  // if the head is NULL 
  if (head == nullptr) return nullptr; 
  Register* target = nullptr;

  // if head is the target
  if (head->get_ID() == ID) {
    target = head;
    head = head->get_next(); // Update head to the next register
    target->set_next(nullptr); // Disconnect the dequeued register
    size--; // Decrease the size of the list
    return target;
  }
  
  Register* current = head;
  while(current != NULL){
    if(current-> get_ID() == ID){
      target = current;
      current -> set_next(target -> get_next());
      target-> set_next(nullptr);
      size --;
      return target;
    }
    current = current -> get_next();
  }
  return nullptr;
}

Register* RegisterList::calculateMinDepartTimeRegister(double expTimeElapsed) {
  // return the register with minimum time of departure of its customer
  // if all registers are free, return nullptr
  if (head == NULL) return nullptr;
  Register* registerWithMinTime = head;
  Register* current = head;
  while(current != nullptr){
    if(current -> calculateDepartTime() != -1 && current-> calculateDepartTime() < registerWithMinTime -> calculateDepartTime()){
      registerWithMinTime = current;
    }
    current = current -> get_next();
  }
  return registerWithMinTime;
}

void RegisterList::print() {
  Register* temp = head;
  while (temp != nullptr) {
    temp->print();
    temp = temp->get_next();
  }
}
