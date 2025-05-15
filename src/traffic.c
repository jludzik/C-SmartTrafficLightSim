#include "traffic.h"

//#define TRAFFIC_TEST

static Road north_road;
static Road east_road;
static Road south_road;
static Road west_road;

static Intersection intersection = {
    .north = &north_road,
    .east = &east_road,
    .south = &south_road,
    .west = &west_road
};

static int traffic_simStepCount = 0;

bool traffic_insertCarToQueue(Car** queue_head, Car car)
{
    Car* new_car = (Car*)malloc(sizeof(Car));
    if(new_car == NULL) return false;

    new_car->vehicleId = car.vehicleId;
    new_car->startRoad = car.startRoad;
    new_car->endRoad = car.endRoad;
    new_car->enterSimStep = car.enterSimStep;
    new_car->outSimStep = car.outSimStep;
    new_car->next = NULL;
    
    if(*queue_head == NULL) *queue_head = new_car;
    else
    {
        Car* car_ptr = *queue_head;

        while(car_ptr->next != NULL) car_ptr = car_ptr->next;

        car_ptr->next = new_car;
    }
    return true;
}

Car* traffic_getCarFromQueue(Car** queue_head)
{   
    if(*queue_head == NULL) return NULL;

    Car* car_ptr = *queue_head;

    if(car_ptr->next == NULL)
    {
        *queue_head = NULL;
        car_ptr->next = NULL;
    } 
    else
    {
        *queue_head = car_ptr->next;
        car_ptr->next = NULL;
    } 

    return car_ptr;
}

Car* traffic_getCarFromQueueBySimStep(Car** queue_head, int sim_step)
{
    if(*queue_head == NULL) return NULL;

    Car* car_ptr = *queue_head;

    while(car_ptr != NULL)
    {
        if(car_ptr->enterSimStep == sim_step)
        {
            *queue_head = car_ptr->next;

            car_ptr->next = NULL;
            return car_ptr;
        }

        car_ptr = car_ptr->next;
    }
    return NULL;
}

int traffic_getCarsOutCountByStep(Car* output_head, int sim_step)
{
    if(output_head == NULL) return 0;

    int car_counter = 0;
    Car* car_ptr = output_head;

    while(car_ptr != NULL)
    {
        if(car_ptr->outSimStep == sim_step) car_counter++;
        car_ptr = car_ptr->next;
    }

    return car_counter;
}

void traffic_printQueue(Car* queue_head)
{
    if(queue_head == NULL)
    {
        printf("Queue empty\n");
        return;
    } 

    Car* car_ptr = queue_head;

    printf("QUEUE: \n");

    while(car_ptr != NULL)
    {
        printf("vehicleId%d\n",car_ptr->vehicleId);
        printf("startRoad: %c\n",car_ptr->startRoad);
        printf("endRoad: %c\n",car_ptr->endRoad);
        printf("enterSimStep: %d\n",car_ptr->enterSimStep);
        printf("outSimStep: %d\n", car_ptr->outSimStep);
        printf("\n");
        car_ptr = car_ptr->next;
    }
    return;
}

void traffic_printCar(Car* car)
{
    if(car == NULL) return;

    printf("VEHICLE\n");
    printf("vehicleId%d\n", car->vehicleId);
    printf("startdRoad: %c\n", car->startRoad);
    printf("endRoad: %c\n", car->endRoad);
    printf("enterSimStep: %d\n", car->enterSimStep);
    printf("outSimStep: %d\n", car->outSimStep);
    if(car->next == NULL) printf("next->NULL\n");
    else printf("next->vehicleId%d\n",car->next->vehicleId);
    printf("\n");
}

bool traffic_getQueueLen(Car* queue_head, int* car_count_dst)
{
    if(car_count_dst == NULL) return false;

    if(queue_head == NULL)
    {
        *car_count_dst = 0;
        return true;
    }
    else
    {
        *car_count_dst = 0;
        Car* car_ptr = queue_head;
        
        while(car_ptr != NULL)
        {
            (*car_count_dst)++;
            car_ptr = car_ptr->next;
        }
        return true;
    }
}

int traffic_compare_priority(const void* a, const void* b)
{
    Road* _a = *((Road**)a);
    Road* _b = *((Road**)b);

    if(_a->priority > _b->priority) return -1;
    else if(_a->priority < _b->priority) return 1;
    else return 0;
}

bool traffic_simInit(Car** parking_head)
{
    intersection.north->tag = 'N';
    intersection.east->tag = 'E';
    intersection.south->tag = 'S';
    intersection.west->tag = 'W';

    intersection.north->car_count = 0;
    intersection.east->car_count = 0;
    intersection.south->car_count = 0;
    intersection.west->car_count = 0;

    intersection.north->head = NULL;
    intersection.east->head = NULL;
    intersection.south->head = NULL;
    intersection.west->head = NULL;

    intersection.north->light_state = LIGHT_RED;
    intersection.east->light_state = LIGHT_RED;
    intersection.south->light_state = LIGHT_RED;
    intersection.west->light_state = LIGHT_RED;

    intersection.north->light_step_counter = 0;
    intersection.east->light_step_counter = 0;
    intersection.south->light_step_counter = 0;
    intersection.west->light_step_counter = 0;

    intersection.north->priority = 0;
    intersection.east->priority = 0;
    intersection.south->priority = 0;
    intersection.west->priority = 0;

    Car* new_car = NULL;
    while((new_car = traffic_getCarFromQueueBySimStep(parking_head, 0)) != NULL)
    {
        switch(new_car->startRoad)
        {
            case 'N':
                if(!(traffic_insertCarToQueue(&(intersection.north->head),*new_car))) return false;
                intersection.north->car_count++;
            break;
            case 'E':
                if(!(traffic_insertCarToQueue(&(intersection.east->head),*new_car))) return false;
                intersection.east->car_count++;
            break;
            case 'S':
                if(!(traffic_insertCarToQueue(&(intersection.south->head),*new_car))) return false;
                intersection.south->car_count++;
            break;
            case 'W':
                if(!(traffic_insertCarToQueue(&(intersection.west->head),*new_car))) return false;
                intersection.west->car_count++;
            break;
            default: return false;
        }
        free(new_car);
    }

    return true;
}

bool traffic_simStep(Car** parking_head, Car** output_head)
{
    if(!traffic_simProcCars(output_head)) return false;

    Car* new_car = NULL;
    while((new_car = traffic_getCarFromQueueBySimStep(parking_head, traffic_simStepCount)) != NULL)
    {
        switch(new_car->startRoad)
        {
            case 'N':
                if(!(traffic_insertCarToQueue(&(intersection.north->head),*new_car))) return false;
                intersection.north->car_count++;
            break;
            case 'E':
                if(!(traffic_insertCarToQueue(&(intersection.east->head),*new_car))) return false;
                intersection.east->car_count++;
            break;
            case 'S':
                if(!(traffic_insertCarToQueue(&(intersection.south->head),*new_car))) return false;
                intersection.south->car_count++;
            break;
            case 'W':
                if(!(traffic_insertCarToQueue(&(intersection.west->head),*new_car))) return false;
                intersection.west->car_count++;
            break;
        }
        free(new_car);
    }

    if(!traffic_simProcLights()) return false;
    
    traffic_simStepCount++;

    return true;
}

bool traffic_simProcCars(Car** output_head)
{
    Car* leave_car = NULL;

    if((intersection.north->head != NULL)&&(intersection.north->light_state == LIGHT_GREEN))
    {
        if((leave_car = traffic_getCarFromQueue(&(intersection.north->head))) == NULL) return false;
        leave_car->outSimStep = traffic_simStepCount;
        intersection.north->car_count--;
    }
    else if((intersection.east->head != NULL)&&(intersection.east->light_state == LIGHT_GREEN))
    {
        if((leave_car = traffic_getCarFromQueue(&(intersection.east->head))) == NULL) return false;
        leave_car->outSimStep = traffic_simStepCount;
        intersection.east->car_count--;
    }
    else if((intersection.south->head != NULL)&&(intersection.south->light_state == LIGHT_GREEN))
    {
        if((leave_car = traffic_getCarFromQueue(&(intersection.south->head))) == NULL) return false;
        leave_car->outSimStep = traffic_simStepCount;
        intersection.south->car_count--;
    }
    else if((intersection.west->head != NULL)&&(intersection.west->light_state == LIGHT_GREEN))
    {
        if((leave_car = traffic_getCarFromQueue(&(intersection.west->head))) == NULL) return false;
        leave_car->outSimStep = traffic_simStepCount;
        intersection.west->car_count--;
    }
    else if(leave_car == NULL) return true;

    Car new_car = {
        .vehicleId = leave_car->vehicleId,
        .startRoad = leave_car->startRoad,
        .endRoad = leave_car->endRoad,
        .enterSimStep = leave_car->enterSimStep,
        .outSimStep = leave_car->outSimStep,
        .next = NULL
    };

    free(leave_car);

    if(!traffic_insertCarToQueue(output_head, new_car)) return false;

    return true;
}

bool traffic_simProcLights(void)
{
    static Road* roads[ROADS_NUMBER];

    if((traffic_simStepCount%TRAFFIC_CYCLE) == 0)
    {    
        if((intersection.north->priority = traffic_calculateRoadPriority(intersection.north)) < 0) return false;
        if((intersection.east->priority = traffic_calculateRoadPriority(intersection.east)) < 0) return false;
        if((intersection.south->priority = traffic_calculateRoadPriority(intersection.south)) < 0) return false;
        if((intersection.west->priority = traffic_calculateRoadPriority(intersection.west)) < 0) return false;

        roads[0] = intersection.north;
        roads[1] = intersection.east;
        roads[2] = intersection.south;
        roads[3] = intersection.west;

        qsort(roads,ROADS_NUMBER,sizeof(Road*),traffic_compare_priority);

        if(!(traffic_changeToRed(roads[1]))) return false;
        if(!(traffic_changeToRed(roads[2]))) return false;
        if(!(traffic_changeToRed(roads[3]))) return false;
    }
    else if((traffic_simStepCount%TRAFFIC_CYCLE) == 1)
    {
        if(!(traffic_changeToGreen(roads[0]))) return false;
        if(!(traffic_changeToRed(roads[1]))) return false;
        if(!(traffic_changeToRed(roads[2]))) return false;
        if(!(traffic_changeToRed(roads[3]))) return false;
    }
    else if((traffic_simStepCount%TRAFFIC_CYCLE) == 2 )
    {
        if(!(traffic_changeToGreen(roads[0]))) return false;
    }
    else if((traffic_simStepCount%TRAFFIC_CYCLE) >= 3) {}

    //-----------TEST------------------------------
    #ifdef TRAFFIC_TEST
    printf("TEST_MODE: Press any key to continue\n");
    getchar();
    traffic_printIntersection();
    #endif
    //---------------------------------------------

    if(intersection.north->car_count > 0) intersection.north->light_step_counter++;
    if(intersection.east->car_count > 0) intersection.east->light_step_counter++;
    if(intersection.south->car_count > 0) intersection.south->light_step_counter++;
    if(intersection.west->car_count > 0) intersection.west->light_step_counter++;
    
    return true;
}

int traffic_calculateRoadPriority(Road* road)
{
    if(road == NULL) return -1;

    int priority = 0;
    if(road->car_count == 0) return priority;
    else
    {
        if(road->light_state == LIGHT_RED)
        {
            priority = (road->car_count * PRIORITY_CAR_COUNT) + (road->light_step_counter * PRIORITY_LIGHT_STEP_COUNTER);
        }
        else
        {
            priority = (road->car_count * PRIORITY_CAR_COUNT);
        }
    }

    return priority;
}

bool traffic_changeToGreen(Road* dst_road)
{
    if(dst_road == NULL) return false;

    switch(dst_road->light_state)
    {
        case LIGHT_RED:
            dst_road->light_state = LIGHT_YELLOW_TO_GREEN;
            dst_road->light_step_counter = 0;
        break;
        case LIGHT_YELLOW_TO_GREEN:
            dst_road->light_state = LIGHT_GREEN;
            dst_road->light_step_counter = 0;
        break;
        default:
        break;
    }

    return true;
}

bool traffic_changeToRed(Road* dst_road)
{
    if(dst_road == NULL) return false;

    switch(dst_road->light_state)
    {
        case LIGHT_GREEN:
            dst_road->light_state = LIGHT_YELLOW_TO_RED;
            dst_road->light_step_counter = 0;
        break;
        case LIGHT_YELLOW_TO_RED:
            dst_road->light_state = LIGHT_RED;
            dst_road->light_step_counter = 0;
        break;
        default:
        break;
    }
    
    return true;
}

void traffic_printIntersection(void)
{
    printf(">INTERSECTION:\n");

    printf(">>Road: %c ", intersection.north->tag);
    traffic_printIntersectionLightState(intersection.north->light_state);
    printf(" State_counter: %d Cars: %d\n",intersection.north->light_step_counter, intersection.north->car_count);
    
    printf(">>Road: %c ", intersection.east->tag);
    traffic_printIntersectionLightState(intersection.east->light_state);
    printf(" State_counter: %d Cars: %d\n",intersection.east->light_step_counter, intersection.east->car_count);

    printf(">>Road: %c ", intersection.south->tag);
    traffic_printIntersectionLightState(intersection.south->light_state);
    printf(" State_counter: %d Cars: %d\n",intersection.south->light_step_counter, intersection.south->car_count);

    printf(">>Road: %c ", intersection.west->tag);
    traffic_printIntersectionLightState(intersection.west->light_state);
    printf(" State_counter: %d Cars: %d\n",intersection.west->light_step_counter, intersection.west->car_count);
}

void traffic_printIntersectionLightState(TrafficLightState tls)
{
    switch(tls)
    {
        case LIGHT_RED:
            printf("State: LIGHT_RED,");
        break;
        case LIGHT_YELLOW_TO_RED:
            printf("State: LIGHT_YELLOW_TO_RED,");
        break;
        case LIGHT_YELLOW_TO_GREEN:
            printf("State: LIGHT_YELLOW_TO_GREEN,");
        break;
        case LIGHT_GREEN:
            printf("State: LIGHT_GREEN,");
        break;
    }
}