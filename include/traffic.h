#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//ROAD_PRIORITY
#define PRIORITY_CAR_COUNT 2
#define PRIORITY_LIGHT_STEP_COUNTER 1

//TRAFFIC_MIN_GREEN_STEPS (MIN 1)(TYP 4)
#define TRAFFIC_MIN_GREEN_STEPS 4

//TRAFFIC_CYCLE 
#define TRAFFIC_CYCLE (TRAFFIC_MIN_GREEN_STEPS+2)


#define ROADS_NUMBER 4

typedef struct Car{
    int vehicleId;
    char startRoad;
    char endRoad;
    int enterSimStep;   //step when car arrives at intersection
    int outSimStep;     //step when car leaves intersection, -1 means that the car didn't leave the intersection
    
    struct Car* next;   //pointer to next car in queue
} Car;

typedef enum {
    LIGHT_RED = 0,
    LIGHT_YELLOW_TO_RED = 1,
    LIGHT_YELLOW_TO_GREEN = 2,
    LIGHT_GREEN = 3
} TrafficLightState;

typedef struct Road{
    struct Car* head;
    TrafficLightState light_state;
    int light_step_counter;
    int car_count;
    int priority;
    char tag;
} Road;

typedef struct Intersection{
    struct Road* north;
    struct Road* east;
    struct Road* south;
    struct Road* west;
} Intersection;

/**
 * @brief Allocate memory for new Car structure and insert in to queue
 * 
 * @param queue_head pointer to pointer to head of queue
 * @param car  which will be add to queue
 * 
 * @return true if successfully added car to queue, false otherwise 
 */
bool traffic_insertCarToQueue(Car** queue_head, Car car);

/**
 * @brief Removes Car from queue if it belongs to the given simulation step
 * 
 * @param queue_head pointer to pointer to head of queue
 * @param sim_step simulation step for filtering
 * 
 * @return pointer to the first deleted Car structure if exist, NULL on failure
 */
Car* traffic_getCarFromQueueBySimStep(Car** queue_head, int sim_step);

/**
 * @brief Removes Car from queue
 * 
 * @param queue_head pointer to pointer to head of queue
 * 
 * @return pointer to the deleted Car structure if exist, NULL on failure
 */
Car* traffic_getCarFromQueue(Car** queue_head);

/**
 * @brief Gets the number of Car structures in the queue
 * 
 * @param queue_head pointer to head of queue
 * @param car_count_dst pointer to destination
 * 
 * @return returns TRUE on success, FALSE otherwise
 */
bool traffic_getQueueLen(Car* queue_head, int* car_count_dst);

/**
 * @brief Print on stdout all Car elements in queue
 * 
 * @param queue_head pointer to head of queue
 */
void traffic_printQueue(Car* queue_head);

/**
 * Print on stdout Car data
 * 
 * @param car pointer to Car structure
 */
void traffic_printCar(Car* car);

/**
 * @brief Execute step of traffic simulation
 * 
 * @param parking_head pointer to the pointer to the top of the queue where cars are waiting before being added to the simulation
 * @param output_head pointer to the pointer to the top of the queue where cars will be stored after leaved intersection
 * 
 * @return TRUE if the simulation step was successful, FALSE otherwise
 */
bool traffic_simStep(Car** parking_head, Car** output_head);

/**
 * @brief Initialize traffic lights. The light change cycle will start with the green light first where the movement appears.
 * 
 * @param parking_head pointer to the pointer to the top of the queue where cars are waiting before being added to the simulation
 * 
 * @return TRUE if initialized successfully, FALSE otherwise
 */
bool traffic_simInit(Car** parking_head);

/**
 * @brief Processes cars at the intersection
 * 
 * @param output_head pointer to the pointer to the top of the queue where cars will be stored after leaved intersection
 * 
 * @return TRUE if car processing was successful, FALSE otherwise
 */
bool traffic_simProcCars(Car** output_head);

/**
 * @brief Processes the state of the traffic lights at an intersection
 * 
 * @return TRUE if traffic lights processing was successful, FALSE otherwise
 */
bool traffic_simProcLights(void);

/**
 * @brief Compares Road structures by the road priority. Used in qsort
 * 
 * @return -1 if a is greater than b;
 *         1 if b is greater than a;
 *         0 if a is equal to b
 */
int traffic_compare_priority(const void* a, const void* b);

/**
 * @brief Get the number of cars that left the intersection by simulation step
 * 
 * @param pointer to the top of the queue where cars will be stored after leaved intersection
 * @param sim_step filtered simulation step
 * 
 * @return number of cars that left the intersection in the given step, -1 on error
 */
int traffic_getCarsOutCountByStep(Car* output_head, int sim_step);

/**
 * @brief Starts the process of changing the lights to green on the given road. First it goes through the yellow state which lasts one step
 * 
 * @param dst_road pointer to the destination road
 * 
 * @return TRUE on success, FALSE otherwise
 */
bool traffic_changeToGreen(Road* dst_road);

/**
 * @brief Starts the process of changing the lights to red on the given road. First it goes through the yellow state which lasts one step
 * 
 * @param dst_road pointer to the destination road
 * 
 * @return TRUE on success, FALSE otherwise
 */
bool traffic_changeToRed(Road* dst_road);

/**
 * @brief Print states of all lights in intersection
 */
void traffic_printIntersection(void);

/**
 * @brief Helper function to traffic_printIntersection
 */
void traffic_printIntersectionLightState(TrafficLightState tls);

/**
 * @brief Calculates the priority for a given road for selecting traffic lights
 * @brief Takes into account the number of cars and waiting time
 * @brief Minimum valid priority is 0
 * 
 * @return returns the priority value for the given road. Higher value means higher priority. returns -1 if the function error occurs
 */
int traffic_calculateRoadPriority(Road* road);

#endif