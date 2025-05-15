#ifndef RW_LOGIC_H
#define RW_LOGIC_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "traffic.h"

#define VEHICLE_PREFIX "vehicle"
#define VEHICLE_PREFIX_LEN 7


/**
 * @brief Checks the correctness of the entered arguments
 * 
 * @return true if correct, false otherwise
 */
bool rw_checkInputArgs(int argc, char** argv);

/**
 * @brief Get traffic commands from json file and store it into single conected list
 * 
 * @param commands_file path to json file with commands
 * @param car_number_dst pointer to varieble where number of all cars will be stored
 * @param sim_steps_dst pointer to varieble where number of simulaton steps will be stored
 * 
 * @return pointer to first car in list if successfully open file, NULL otherwise
 */
Car* rw_getCommands(const char* commands_file, int* car_number_dst, int* sim_steps_dst);

/**
 * @brief Change vehicleId in string to int to write in car structure
 * 
 * @param vehicleIdString pointer to vehicleId string
 * 
 * @return vehicleId as int if successfully, -1 otherwise
 */
int rw_vehicleIdToInt(const char* vehicleIdString);

/**
 * @brief Change startRoad or endRoad to single char represnting direction
 * 
 * @param roadString pointer to road string
 * 
 * @return char representing direction: 'N','E','S','W' if successfully, '\0' otherwise
 */
char rw_roadToChar(const char* roadString);

/**
 * @brief Create output JSON file with cars with leaved intersection
 * 
 * @param outlog_file path to json file where simulation results will be saved
 * @param output_head pointer to the top of the queue where cars will be stored after leaved intersection
 * @param sim_steps number of all simulation steps, must be greater than zero
 * 
 * @return TRUE if file created successfully, FALSE otherwise
 */
bool rw_createOutLog(const char* outlog_file, Car* output_head, int sim_steps);

/**
 * @brief Change vehicleId int to string to write in output file
 * 
 * @param IntvehicleId vehicleId as int
 * @param StringVehicleIdDst pointer to destination
 * 
 * @return TRUE if converted successfully, FALSE otherwise
 */
bool rw_intToVehicleId(int IntVehicleId, char** StringVehicleIdDst);

#endif