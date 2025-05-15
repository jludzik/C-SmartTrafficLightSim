## Smart Traffic Light Intersection Simulation

## Compilation
- Language: C (C18)
- Compiler: x86_64-w64-mingw32
- Version: 14.1.0

- The project uses the cJSON [library](https://github.com/DaveGamble/cJSON)

- Compile from terminal:
gcc -o build/main.exe src/main.c src/rw_logic.c src/traffic.c libs/cJSON/cJSON.c -Iinclude -Wall -std=c18

- Or use a Makefile



## Running the simulation
The program require **two aruments**:
1. Path to the input `*.json` file with commands
2. Path to the output `*.json` file

Example input file can be found in example_json_files directory.



##  Simulation Assumptions
- Only one road has a green light at one simulation step
- In one step, only one car can pass the intersection if:
    - has a green light
    - is first in queue
- Vehicles must have unique identifiers for differentiate in output file
- Vehicle identifiers are created according to the following formula: `vehicleX`, where X >= 0
- When a car leaves the intersection, its Car structure stores the step number in int outSimStep
    - (-1) in program means the vehicle did not leave the intersection
- Traffic light states are stored in the Road structure using the enum TrafficLightState:
    - `LIGHT_RED` – red, vehicles stop
    - `LIGHT_YELLOW_TO_RED` – yellow (to red), vehicles stop
    - `LIGHT_YELLOW_TO_GREEN` – yellow (to green), vehicles stop
    - `LIGHT_GREEN` – green, vehicles move



## Data Handling and Processing
1. Initialization
    - Read number of simulation steps and store it to `int simSteps`
    - Read all vehicles and store it in dynamic FIFO queue `parkingHead`
2. Simulation steps
    - New vehicles arriving are dequeued from `parkingHead` and assigned to one of the four road queues
    - Vehicles leaving intersection are dequeued from roads queues and stored in the `outputHead` queue with `outSimStep` set
3. After simulation
    - The output queue is written to an output json file



## Traffic Light Control Algorithm
1. Lights change based on car count and red light duration, weighted by priorities.

2. Priority calculation
    - If the road is empty: priority = 0
    - If the road is not empty and light is red:
        - `priority` = (`car_count` * `PRIORITY_CAR_COUNT`) + (`light_step_counter` * `PRIORITY_LIGHT_STEP_COUNTER`)

    - If the road is not empty and light is green:
        - `priority` = (`car_count` * `PRIORITY_CAR_COUNT`)


    - Definitions:
        - `car_count` - number of vehicles on the road
        - `light_step_counter` - how long the red light has lasted since a vehicle appeared, (in simulation steps), (the variable is not incremented when the road is empty)
        - `PRIORITY_CAR_COUNT` - weight of car_count factor, default: 2
        - `PRIORITY_LIGHT_STEP_COUNTER` - weight of light_step_counter factor, default: 1

3. Selecting the road with highest calculated priority.

4. Starting process of changing light from red to green via yellow light for one simulation step.

5. When the red light is set, the process of changing the lights on the previously selected road to green begins, also passing through the yellow lights for one simulation step.

6. Green light lasts at least `TRAFFIC_MIN_GREEN_STEPS`, default: 4 steps

7. Then, after the green light time has elapsed, the algorithm repeats. If the same raod is chosen again, the lights remain green (steps 4. and 5. are not performed)
