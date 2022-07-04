
#include "parser_json.h"
#include<json-c/json.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void parse_int32(int32_t* destination, struct json_object* tokens, const char* name) {
    struct json_object* tmp = NULL;
    json_object_object_get_ex(tokens, name, &tmp);
    *destination = json_object_get_int(tmp);
    free(tmp);
}

void parse_int64(int64_t* destination, struct json_object* tokens, const char* name) {
    struct json_object* tmp = NULL;
    json_object_object_get_ex(tokens, name, &tmp);
    *destination = json_object_get_int64(tmp);
    free(tmp);
}

void parse_float(float* destination, struct json_object* tokens, const char* name) {
    struct json_object* tmp = NULL;
    json_object_object_get_ex(tokens, name, &tmp);
    *destination = json_object_get_double(tmp);
    free(tmp);
}

void parse_string(char** destination, struct json_object* tokens, const char* name) {
    struct json_object* tmp = NULL;
    json_object_object_get_ex(tokens, name, &tmp);
    const char* value = json_object_to_json_string(tmp);
    const size_t value_lenght = strlen(value) + 1;
    *destination = NULL;
    *destination = calloc(sizeof(*destination), value_lenght);
    if (*destination == NULL) {
        printf("Failed to allocate memory for name\n");
        free(tmp);
    }
    memcpy(*destination, value, sizeof(*value) * value_lenght);
    free(tmp);
}



int parse_weather_json(char* buffer, WeartherInfo_t* weather) {
    struct json_object* parsed = json_tokener_parse(buffer);
    
    struct json_object* tokens = NULL;
    json_object_object_get_ex(parsed, "weather", &tokens);
    weather -> size_weathers = json_object_array_length(tokens);
    weather -> weathers = NULL;
    weather -> weathers = calloc(sizeof(*weather -> weathers), weather -> size_weathers);
	if (weather -> weathers == NULL) {
		printf("Failed to allocate memory for name\n");
		free(tokens);
		return -1;
	}
    for (int i = 0; i < weather -> size_weathers; i++) {
    	struct json_object* arr_item = json_object_array_get_idx(tokens, i);
		parse_string(&(weather -> weathers[i].main), arr_item, "main");
		parse_string(&(weather -> weathers[i].description), arr_item, "description");
		free(arr_item);
    }
    free(tokens);
    
    parse_string(&(weather -> base), parsed, "base");
    
    json_object_object_get_ex(parsed, "main", &tokens);
    parse_float(&(weather -> main.temp), tokens, "temp");
    parse_float(&(weather -> main.feels_like), tokens, "feels_like");
    parse_float(&(weather -> main.temp_min), tokens, "temp_min");
    parse_float(&(weather -> main.temp_max), tokens, "temp_max");
    parse_int32(&(weather -> main.pressure), tokens, "pressure");
    parse_int32(&(weather -> main.humidity), tokens, "humidity");
    free(tokens);
    
    parse_int32(&(weather -> visibility), parsed, "visibility");
    
    json_object_object_get_ex(parsed, "wind", &tokens);
    parse_float(&(weather -> wind.speed), tokens, "speed");
    parse_float(&(weather -> wind.deg), tokens, "deg");
    parse_float(&(weather -> wind.gust), tokens, "gust");
    free(tokens);
    
    json_object_object_get_ex(parsed, "clouds", &tokens);
    parse_int32(&(weather -> clouds.all), tokens, "all");
    free(tokens);
    
    parse_int32(&(weather -> dt), parsed, "dt");
    
    json_object_object_get_ex(parsed, "sys", &tokens);
    parse_int32(&(weather -> sys.type), tokens, "type");
    parse_int64(&(weather -> sys.id), tokens, "id");
    parse_string(&(weather -> sys.country), tokens, "country");
    parse_int64(&(weather -> sys.sunrise), tokens, "sunrise");
    parse_int64(&(weather -> sys.sunset), tokens, "sunset");
    free(tokens);
    
    parse_int64(&(weather -> timezone), parsed, "timezone");
    parse_int64(&(weather -> id), parsed, "id");
    parse_string(&(weather -> name), parsed, "name");
    parse_int32(&(weather -> cod), parsed, "cod");
    
    return 0;
}
