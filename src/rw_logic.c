#include "rw_logic.h"

bool rw_checkInputArgs(int argc, char** argv)
{
    if(argc != 3)
    {
        printf("Usage: %s <input.json> <output.json>\n",argv[0]);
        return false;
    }
    return true;
}

Car* rw_getCommands(const char* commands_file, int* car_number_dst, int* sim_steps_dst)
{
    if((commands_file == NULL)||(car_number_dst == NULL)||(sim_steps_dst == NULL)) return NULL;

    FILE* f = fopen(commands_file,"r");
    if(f == NULL)
    {
        printf("RW_ERROR_OPENING_FILE\n");
        return NULL;
    }

    fseek(f,0,SEEK_END);
    size_t f_len = (size_t)ftell(f);
    rewind(f);

    char* json_buf = (char*)calloc(f_len+1,sizeof(char));   //f_len+1 ('\0')
    if(json_buf == NULL)
    {
        printf("RW_ERROR_ALOCATING_MEMORY\n");
        return NULL;
    }

    size_t f_read = fread(json_buf,sizeof(char),f_len,f);
    if(f_read == 0)
    {
        free(json_buf);
        fclose(f);
        printf("RW_ERROR_NO_DATA_READ\n");
        return NULL;
    }
    json_buf[f_len] = '\0';
    fclose(f);

    cJSON* json_root = cJSON_Parse(json_buf);
    if(json_root == 0)
    {
        free(json_buf);
        printf("RW_ERROR_JSON_PARSE: %s\n",cJSON_GetErrorPtr());
        return NULL;
    }
    free(json_buf);

    cJSON* json_commands = cJSON_GetObjectItem(json_root,"commands");
    if(cJSON_IsArray(json_commands) == 0)
    {
        cJSON_Delete(json_root);
        printf("RW_ERROR_COMMANDS_BAD_FORMAT\n");
        return NULL;
    }

    Car* parking_head = NULL;     //place where will be stored cars before add to sim

    cJSON* json_command_el = NULL;
    cJSON_ArrayForEach(json_command_el,json_commands)
    {
        cJSON* json_command_type = cJSON_GetObjectItem(json_command_el,"type");

        if(cJSON_IsString(json_command_type) == 0) continue;
        else if(strcmp(json_command_type->valuestring,"addVehicle") == 0)
        {
            cJSON* json_command_vehicleId = cJSON_GetObjectItem(json_command_el,"vehicleId");
            cJSON* json_command_startRoad = cJSON_GetObjectItem(json_command_el,"startRoad");
            cJSON* json_command_endRoad = cJSON_GetObjectItem(json_command_el,"endRoad");

            if(json_command_vehicleId && json_command_startRoad && json_command_endRoad)
            {
                Car new_car = {
                    .vehicleId = rw_vehicleIdToInt(json_command_vehicleId->valuestring),
                    .startRoad = rw_roadToChar(json_command_startRoad->valuestring),
                    .endRoad = rw_roadToChar(json_command_endRoad->valuestring),
                    .enterSimStep = *sim_steps_dst,
                    .outSimStep = -1
                };
        
                if(!(traffic_insertCarToQueue(&parking_head, new_car)))
                {
                    printf("RW_ERROR_INSERTING_CAR\n");
                    return NULL;
                }
                (*car_number_dst)++;
            }
        }
        else if(strcmp(json_command_type->valuestring,"step") == 0)
        {
            (*sim_steps_dst)++;
        }
    }

    cJSON_Delete(json_root);
    return parking_head;
}

int rw_vehicleIdToInt(const char* vehicleIdString)
{
    if(vehicleIdString == NULL) return -1;
    if(strncmp(vehicleIdString, VEHICLE_PREFIX, VEHICLE_PREFIX_LEN) != 0) return -1;

    for(int i=0;i<VEHICLE_PREFIX_LEN;i++) vehicleIdString++;

    int output = atoi(vehicleIdString);
    return output;
}

char rw_roadToChar(const char* roadString)
{
    if(roadString == NULL) return '\0';

    if(strcmp(roadString,"north") == 0) return 'N';
    else if (strcmp(roadString,"east") == 0) return 'E';
    else if (strcmp(roadString,"south") == 0) return 'S';
    else if (strcmp(roadString,"west") == 0) return 'W';
    else return '\0';
}

bool rw_createOutLog(const char* outlog_file, Car* output_head, int sim_steps)
{
    if(outlog_file == NULL) return false;
    if(sim_steps == 0)
    {
        printf("ERROR_NO_SIM_STEP\n");
        return false;
    }
    
    int** output_data = calloc((size_t)sim_steps, sizeof(int*));
    for(int i=0;i<sim_steps;i++)
    {
        int car_count = traffic_getCarsOutCountByStep(output_head,i);

        output_data[i] = (int*)calloc((size_t)car_count+1,sizeof(int));     //+1 for last -1

        for(size_t j=0;j<(car_count+1);j++)
        {
            if(j == car_count) output_data[i][j] = -1;
            else
            {
                Car* new_car = NULL;
                if((new_car = traffic_getCarFromQueue(&output_head)) == NULL)
                {
                    printf("ERROR_GET_CAR_FROM_QUEUE\n");
                    return false;
                }

                output_data[i][j] = new_car->vehicleId;
                free(new_car);
            }
            //printf("output_data[%d][%zu] = %d\n",i,j,output_data[i][j]);
        }
    }
    
    cJSON* json_obj = cJSON_CreateObject();
    cJSON* json_steps = cJSON_CreateArray();

    for(int i=0;i<sim_steps;i++)
    {
        cJSON* json_step_obj = cJSON_CreateObject();
        cJSON* json_left_vehicles = cJSON_CreateArray();
    
        for(int j = 0; output_data[i][j] != -1; j++)
        {
            char* vehicle_text = NULL;
            if(!(rw_intToVehicleId(output_data[i][j],&vehicle_text)))
            {
                printf("ERROR_RW_INT_TO_VEHICLEID for value = %d\n", output_data[i][j]);
                return false;
            }
            
            cJSON_AddItemToArray(json_left_vehicles,cJSON_CreateString(vehicle_text));
            free(vehicle_text);
        }

        cJSON_AddItemToObject(json_step_obj,"leftVehicles",json_left_vehicles);
        cJSON_AddItemToArray(json_steps, json_step_obj);
    }

    cJSON_AddItemToObject(json_obj,"stepStatuses",json_steps);

    char* json_buf = cJSON_Print(json_obj);
    size_t json_buf_len = strlen(json_buf);

    FILE* f = fopen(outlog_file, "w");
    if(f == NULL)
    {
        printf("RW_ERROR_CREATING_FILE\n");
        return false;
    }
    
    fwrite(json_buf,sizeof(char),json_buf_len,f);
    fclose(f);

    cJSON_Delete(json_obj);
    free(json_buf);

    fclose(f);
    return true;
}

bool rw_intToVehicleId(int IntVehicleId, char** StringVehicleIdDst)
{
    if(IntVehicleId <= 0) return false;
    if(StringVehicleIdDst == NULL) return false;

    int string_len = snprintf(NULL,0,"%s%d",VEHICLE_PREFIX,IntVehicleId);

    *StringVehicleIdDst = (char*)calloc(string_len+1,sizeof(char));
    if(*StringVehicleIdDst == NULL) return false;

    sprintf(*StringVehicleIdDst,"%s%d",VEHICLE_PREFIX,IntVehicleId);
    
    return true;
}
