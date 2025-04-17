#ifndef FIND_OBJECTS_H_
#define FIND_OBJECTS_H_

//struct to store collected sensor data and angles
typedef struct {
	float ir_val;
	float dist;
	uint8_t angle;
} sensor_data;

typedef struct {
	float dist;
	float width;
	short delta;
	short bisector;
} obj_data;

obj_data findSmallestObj(obj_data* objects, uint8_t obj_count);

void free_objs(obj_data** objects, uint8_t* const size);

obj_data* getObjects(uint8_t* const distant_obj_detected, float* const distant_obj_angle, float* const distant_obj_dist, obj_data* obj_list, uint8_t* const obj_list_sz, short cm_ping_range);

#endif //!FIND_OBJECTS_H_
