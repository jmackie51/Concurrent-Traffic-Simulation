#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <cstdlib>
#include <ctime>
#include <time.h>

using namespace std;

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() --COMPLETE
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    unique_lock<mutex> lock(_mutex);
    _condition.wait(lock, [this]{return !_queue.empty(); }); //pass unique lock to condition variable.  make sure not a spurias wake up

    T msg = move(_queue.back()); //pull message from the queue using move semantics.
    _queue.pop_back();  //remove from queue

    return msg; //received object should then be returned by the receive function

}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> --COMPLETE 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification. --COMPLETE

    lock_guard<mutex> lock(_mutex); //perform vector modification under lock
    _queue.emplace_back(move(msg)); //move message into the queue
    _condition.notify_one(); //notify the mutex that new data is available to wake up the next thread

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}




void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop --COMPLETE
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true)
        if (TrafficLight::msgQueue.receive() == green){ return;  } // when green is encoutered then return
    
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    //To do this, use the thread queue in the base class. --COMPLETE
    threads.emplace_back(thread(&TrafficLight::cycleThroughPhases, this));

}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles --COMPLETE
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    chrono::time_point<std::chrono::system_clock> lastUpdate, now;
    double randomTimeFrame;
    long elapsedTime;


    lastUpdate = chrono::system_clock::now(); //initiate 
    srand((unsigned) time(0)); //seed random num generator
    this_thread::sleep_for(chrono::milliseconds(500));  //sleep to generate varied seed times
    randomTimeFrame = (rand() % 3 + 4); //generate random number

    while (true) {

        now = chrono::system_clock::now(); //get current time
        elapsedTime = chrono::duration_cast<chrono::seconds>(now - lastUpdate).count(); //get time diff

        if ((elapsedTime) >= randomTimeFrame){ //if time frame has been met then change phase

           switch(_currentPhase) {
           case red:
            _currentPhase = green;
            break;

           case green:
            _currentPhase = red;
            break;
           }

           TrafficLight::msgQueue.send(move(_currentPhase)); //send update to message queue with move semantics

           lastUpdate = chrono::system_clock::now(); //update
        }
        
        this_thread::sleep_for(chrono::milliseconds(1));  //sleep
    } 
}

