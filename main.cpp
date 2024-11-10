#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "Customer.h"
#include "QueueList.h"
#include "Register.h"
#include "RegisterList.h"

using namespace std;

// Function Declarations:

// Set mode of the simulation
string getMode();

// Register
void parseRegisterAction(stringstream &lineStream, string mode);
void openRegister(
    stringstream &lineStream,
    string mode);  // register opens (it is upto customers to join)
void closeRegister(stringstream &lineStream,
                   string mode);  // register closes

// Customer
void addCustomer(stringstream &lineStream,
                 string mode);  // customer wants to join

// Helper functions
bool getInt(stringstream &lineStream, int &iValue);
bool getDouble(stringstream &lineStream, double &dValue);
bool foundMoreArgs(stringstream &lineStream);

// Global variables
RegisterList *registerList;  // holding the list of registers
QueueList *doneList;         // holding the list of customers served
QueueList *singleQueue;      // holding customers in a single virtual queue
double expTimeElapsed;  // time elapsed since the beginning of the simulation

// List of commands:
// To open a register
// register open <ID> <secPerItem> <setupTime> <timeElapsed>
// To close register
// register close <ID> <timeElapsed>
// To add a customer
// customer <items> <timeElapsed>

int main() {
  registerList = new RegisterList();
  doneList = new QueueList();
  singleQueue = new QueueList();
  expTimeElapsed = 0;

  // Set mode by the user
  string mode = getMode();

  string line;
  string command;

  cout << "> ";  // Prompt for input
  getline(cin, line);

  while (!cin.eof()) {
    stringstream lineStream(line);
    lineStream >> command;
    if (command == "register") {
      parseRegisterAction(lineStream, mode);
    } else if (command == "customer") {
      addCustomer(lineStream, mode);
    } else {
      cout << "Invalid operation" << endl;
    }
    cout << "> ";  // Prompt for input
    getline(cin, line);
  }
  // Calculate Statistics
  cout << "Finished at time " << expTimeElapsed << endl;

  int maxWait = 0;
  double totalWait = 0, aveWait = 0;
  double total_std = 0, sqrt_std = 0;
  int custCount = 0;
  Customer *curr = doneList->get_head();
  while (curr != nullptr) {
    double currWait = curr->get_departureTime() - curr->get_arrivalTime();
    if (currWait > maxWait) {
      maxWait = currWait;
    }
    totalWait += currWait;
    custCount++;

    curr = curr->get_next();
  }
  aveWait = totalWait / custCount;

  curr = doneList->get_head();
  while (curr != nullptr) {
    double currWait = curr->get_departureTime() - curr->get_arrivalTime();
    total_std += pow((currWait - aveWait), 2) ;

    curr = curr->get_next();
  }

  sqrt_std = sqrt(total_std / custCount);

  cout << "Statistics:" << endl;
  cout << "Maximum wait time: " << maxWait << endl;
  cout << "Average wait time: " << aveWait << endl;
  cout << "Standard Deviation of wait time: " << sqrt_std << endl;

  // You have to make sure all dynamically allocated memory is freed
  // before return 0
  return 0;
}

string getMode() {
  string mode;
  cout << "Welcome to ECE 244 Grocery Store Queue Simulation!" << endl;
  cout << "Enter \"single\" if you want to simulate a single queue or "
          "\"multiple\" to "
          "simulate multiple queues: \n> ";
  getline(cin, mode);

  if (mode == "single") {
    cout << "Simulating a single queue ..." << endl;
  } else if (mode == "multiple") {
    cout << "Simulating multiple queues ..." << endl;
  }

  return mode;
}

void addCustomer(stringstream &lineStream, string mode) {
  int items;
  double timeElapsed;

  // Parse items and timeElapsed from lineStream
  if (!getInt(lineStream, items) || !getDouble(lineStream, timeElapsed)) {
    cout << "Error: too few arguments." << endl;
    return;
  }
  if (foundMoreArgs(lineStream)) {
    cout << "Error: too many arguments." << endl;
    return;
  }

  // update the expTimeElapsed
  expTimeElapsed += timeElapsed;

  // Update the System (are there any customers done)

  for (int i = 1; i <= expTimeElapsed; i++) {
    Register *current_prime = registerList->get_head();
    while (current_prime != nullptr) {
      if (current_prime->get_queue_list()->get_head() != nullptr) {
        // choosing whether the customer arrival time or register ready time is
        // latest
        int startTime = 0;
        if (current_prime->get_availableTime() >=
            current_prime->get_queue_list()->get_head()->get_arrivalTime()) {
          startTime = current_prime->get_availableTime();
        } else {
          startTime =
              current_prime->get_queue_list()->get_head()->get_arrivalTime();
        }
        double expectedFinish =
            current_prime->get_queue_list()->get_head()->get_numOfItems() *
                current_prime->get_secPerItem() +
            startTime + current_prime->get_overheadPerCustomer();
        if (expectedFinish <= expTimeElapsed && expectedFinish == i) {
          current_prime->departCustomer(doneList);
          cout << "Departed a customer at register ID "
               << current_prime->get_ID() << " at " << expectedFinish << endl;
        }
      }
      current_prime = current_prime->get_next();
    }
  }

  // Create a new customer
  Customer *newCus = new Customer(expTimeElapsed, items);
  cout << "A customer entered " << endl;

  if (mode == "single") {
    // Check for any free register
    Register *freeRegister = registerList->get_free_register();
    if (freeRegister != nullptr) {
      // free register => QUEUE CUSTOMER INTO REGISTER
      freeRegister->get_queue_list()->enqueue(newCus);
      newCus->set_arrivalTime(expTimeElapsed);
      // update the available time of the register that you put the cutomer into
      cout << "Queued a customer with free register " << freeRegister->get_ID()
           << endl;
    } else {
      // Otherwise, add the customer to the singleQueue
      singleQueue->enqueue(newCus);
      cout << "No free registers" << endl;
    }
  } else if (mode == "multiple") {
    // Find the register with the minimum number of items
    Register *minItemsRegister = registerList->get_min_items_register();
    if (minItemsRegister != nullptr) {
      minItemsRegister->get_queue_list()->enqueue(newCus);
      // update the available time of the register that you put the customer
      // into
      minItemsRegister->set_availableTime(
          items * minItemsRegister->get_secPerItem() + expTimeElapsed);
      cout << "Customer added to register " << minItemsRegister->get_ID()
           << " with the fewest items." << endl;
    }
  }
}

void parseRegisterAction(stringstream &lineStream, string mode) {
  string operation;
  lineStream >> operation;
  if (operation == "open") {
    openRegister(lineStream, mode);
  } else if (operation == "close") {
    closeRegister(lineStream, mode);
  } else {
    cout << "Invalid operation" << endl;
  }
}

void openRegister(stringstream &lineStream, string mode) {
  int ID;
  double secPerItem, setupTime, timeElapsed;

  // Convert strings to int and double
  if (!getInt(lineStream, ID) || !getDouble(lineStream, secPerItem) ||
      !getDouble(lineStream, setupTime) ||
      !getDouble(lineStream, timeElapsed)) {
    cout << "Error: too few arguments." << endl;
    return;
  }

  if (foundMoreArgs(lineStream)) {
    cout << "Error: too many arguments" << endl;
    return;
  }
  // update expTimeElapsed
  expTimeElapsed += timeElapsed;

  // Update the System (are there any customers done)

  for (int i = 1; i <= expTimeElapsed; i++) {
    Register *current_prime = registerList->get_head();
    while (current_prime != nullptr) {
      if (current_prime->get_queue_list()->get_head() != nullptr) {
        // choosing whether the customer arrival time or register ready time is
        // latest
        int startTime = 0;
        if (current_prime->get_availableTime() >=
            current_prime->get_queue_list()->get_head()->get_arrivalTime()) {
          startTime = current_prime->get_availableTime();
        } else {
          startTime =
              current_prime->get_queue_list()->get_head()->get_arrivalTime();
        }
        double expectedFinish =
            current_prime->get_queue_list()->get_head()->get_numOfItems() *
                current_prime->get_secPerItem() +
            startTime + current_prime->get_overheadPerCustomer();
        if (expectedFinish <= expTimeElapsed && expectedFinish == i) {
          current_prime->departCustomer(doneList);
          cout << "Departed a customer at register ID "
               << current_prime->get_ID() << " at " << expectedFinish << endl;
        }
      }
      current_prime = current_prime->get_next();
    }
  }

  // Check if the register is already open
  Register *current = registerList->get_head();
  while (current != nullptr) {
    if (current->get_ID() == ID) {  // If the register is already open
      cout << "Error: register " << ID << " is already open" << endl;
      return;  // Stop processing further if the register is already open
    }
    current = current->get_next();  // Move to the next register
  }

  // If we did not find an open register with the same ID, open the new register
  Register *newReg = new Register(ID, secPerItem, setupTime,
                                  timeElapsed);  // Create a new register
  registerList->enqueue(newReg);                 // Add it to the register list
  newReg->set_next(nullptr);  // Set the next pointer to null (not required if
                              // it's the last element)
  newReg->set_availableTime(
      expTimeElapsed);  // newly opened registers will be available right away
  cout << "Opened register " << ID << endl;

  // If simulating a single queue and there are customers in line, assign a
  // customer to the new register
  if (mode == "single" && singleQueue->get_head() != nullptr) {
    Customer *nextInLine =
        singleQueue->dequeue();  // Get the next customer from the single queue
    newReg->get_queue_list()->enqueue(
        nextInLine);  // Add the customer to the register's queue
    // Update the available time of the newReg
    newReg->set_availableTime(expTimeElapsed);
    cout << "Queued a customer with free register " << newReg->get_ID() << endl;
  }
}

void closeRegister(stringstream &lineStream, string mode) {
  int ID;
  double timeElapsed;
  // convert string to int
  if (!getInt(lineStream, ID) || !getDouble(lineStream, timeElapsed)) {
    cout << "Error: too few arguments." << endl;
    return;
  }
  if (foundMoreArgs(lineStream)) {
    cout << "Error: too many arguments" << endl;
    return;
  }
  // Update expTimeElapsed
  expTimeElapsed += timeElapsed;

  // Update the System (are there any customers done)

  for (int i = 1; i <= expTimeElapsed; i++) {
    Register *current_prime = registerList->get_head();
    while (current_prime != nullptr) {
      if (current_prime->get_queue_list()->get_head() != nullptr) {
        // choosing whether the customer arrival time or register ready time is
        // latest
        int startTime = 0;
        if (current_prime->get_availableTime() >=
            current_prime->get_queue_list()->get_head()->get_arrivalTime()) {
          startTime = current_prime->get_availableTime();
        } else {
          startTime =
              current_prime->get_queue_list()->get_head()->get_arrivalTime();
        }
        double expectedFinish =
            current_prime->get_queue_list()->get_head()->get_numOfItems() *
                current_prime->get_secPerItem() +
            startTime + current_prime->get_overheadPerCustomer();
        if (expectedFinish <= expTimeElapsed && expectedFinish == i) {
          current_prime->departCustomer(doneList);
          cout << "Departed a customer at register ID "
               << current_prime->get_ID() << " at " << expectedFinish << endl;
        }
      }
      current_prime = current_prime->get_next();
    }
  }

  // Check if the register is open
  Register *current = registerList->get_head();
  while (current != nullptr) {
    if (current->get_ID() == ID) {  // If the register is already open
      // If it is open dequeue it and free it's memory
      delete (registerList->dequeue(ID)->get_queue_list()->get_head());
      delete (registerList->dequeue(ID));
      cout << "Closed register " << ID << endl;
      return;
    }
    current = current->get_next();  // Move to the next register
  }
  cout << "Error: register " << ID << " is not open" << endl;
}

bool getInt(stringstream &lineStream, int &iValue) {
  // Reads an int from the command line
  string command;
  lineStream >> command;
  if (lineStream.fail()) {
    return false;
  }
  iValue = stoi(command);
  return true;
}

bool getDouble(stringstream &lineStream, double &dvalue) {
  // Reads a double from the command line
  string command;
  lineStream >> command;
  if (lineStream.fail()) {
    return false;
  }
  dvalue = stod(command);
  return true;
}

bool foundMoreArgs(stringstream &lineStream) {
  string command;
  lineStream >> command;
  if (lineStream.fail()) {
    return false;
  } else {
    return true;
  }
}
