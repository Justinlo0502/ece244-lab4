
// Created by Salma Emara on 2023-06-02.
#include "Register.h"

#include <iostream>

Register::Register(int id, double timePerItem, double overhead,
                   double entryTime) {
  ID = id;
  secPerItem = timePerItem;
  overheadPerCustomer = overhead;
  availableTime = entryTime;
  next = nullptr;           // no other registers yet
  queue = new QueueList();  // no customers in line yet, but has to initialize a
                            // queue
}

Register::~Register() {
  delete next;
} // probably need to implement sth for DMA

QueueList* Register::get_queue_list() {return queue;}

Register* Register::get_next() {return next;}

int Register::get_ID() {return ID;}

double Register::get_secPerItem() {return secPerItem;}

double Register::get_overheadPerCustomer() {return overheadPerCustomer;}

double Register::get_availableTime() {return availableTime;}

void Register::set_next(Register* nextRegister) {
  next = nextRegister;
}


void Register::set_availableTime(double availableSince) {
  availableTime = availableSince;
}

double Register::calculateDepartTime() {
  // Get the departure time of the first customer in the queue
  // returns -1 if no customer is in the queue
  if(queue == nullptr || queue -> get_head() == nullptr){
    return -1;
  } else{
    return queue -> get_head() -> get_departureTime();
  }
}

void Register::departCustomer(QueueList* doneList) {

  // Dequeue the current customer (head of the queue)
  Customer* done = queue->dequeue();

  // Add the customer to the doneList (completed customers)
  doneList->enqueue(done);

  int startTime = 0;
  if(availableTime >= done->get_arrivalTime()){
    startTime = availableTime;
  }else{
    startTime = done->get_arrivalTime();
  }
  // Set the available time 
  availableTime = startTime+secPerItem*done->get_numOfItems()+overheadPerCustomer;

  // Set the departure time of the customer to the updated availableTime
  done->set_departureTime(availableTime);
}


void Register::print() {
  std::cout << "Register ID: " << ID << std::endl;
  std::cout << "Time per item: " << secPerItem << std::endl;
  std::cout << "Overhead per customer: " << overheadPerCustomer << std::endl;
  if (queue->get_head() != nullptr) {
    std::cout << "Queue has customers: \n";
    queue->print();
  }
}
