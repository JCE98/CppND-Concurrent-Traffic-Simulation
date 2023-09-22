#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */


template <class T>
T MessageQueue<T>::Receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    // DONE

    std::unique_lock<std::mutex> uniqueLock(_mtx);
    _condition.wait(uniqueLock, [this] { return !_queue.empty(); });

	// Get the latest element and remove it from the queue
	T msg = std::move(_queue.back());
	_queue.pop_back();
	return msg;
}

template <class T>
void MessageQueue<T>::Send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // DONE

    std::lock_guard<std::mutex> lockGuard(_mtx);
    _queue.push_back(std::move(msg));
	_condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    //DONE

    while(true)
    {
        // Call receive function on the message queue
        _currentPhase = _queue->Receive();

        // Check if the TrafficLight::_currentPhase is green
        if(_currentPhase==TrafficLightPhase::red)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    // DONE
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    //DONE

    // Set up random number generator for TrafficLight::_currentPhase cycling
    std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(4, 6);
    int cycleDuration = distr(eng); //Duration of a single simulation cycle in seconds, is randomly chosen

    auto prevTimeStamp = std::chrono::system_clock::now();
    while(true)
    {
        // Count time since last cycle of TrafficLight::_currentPhase
        long timeSinceUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - prevTimeStamp).count();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Sleep at every iteration to reduce CPU usage

        // Cycle TrafficLight::_currentPhase on randomly selected cycle
        if(timeSinceUpdate >= cycleDuration)
        {
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }

            // Send an update to the message queue and wait for it to be sent
            auto is_sent = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::Send, _queue, std::move(_currentPhase));
			is_sent.wait();

            // Reset stopwatch and randomly select another cycle duration
            prevTimeStamp = std::chrono::system_clock::now();
            cycleDuration = distr(eng);

        }
    }

}