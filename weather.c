#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define API_KEY "bab55c96220fff1d83ff0fc5342465d7"
#define CITY_NAME "Kolkata"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userdata;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("Out of memory\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main() {
    CURL *curl;
    CURLcode res;
    char url[200], city[50], description[100];
    int temp, feels_like, humidity;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        sprintf(url, "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", CITY_NAME, API_KEY);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("Error: %s\n", curl_easy_strerror(res));
            return 1;
        }

        json_object *root, *weather, *main;
        root = json_tokener_parse(chunk.memory);
        json_object_object_get_ex(root, "weather", &weather);
        json_object_object_get_ex(weather, "description", &root);
        strcpy(description, json_object_get_string(root));
        json_object_object_get_ex(root, "main", &main);
        temp = json_object_get_int(json_object_object_get(main, "temp"));
        feels_like = json_object_get_int(json_object_object_get(main, "feels_like"));
        humidity = json_object_get_int(json_object_object_get(main, "humidity"));

        printf("Weather in %s:\n", CITY_NAME);
        printf("Description: %s\n", description);
        printf("Temperature: %d°C\n", temp - 273);
        printf("Feels Like: %d°C\n", feels_like - 273);
        printf("Humidity: %d%%\n", humidity);

        json_object_put(root);
        free(chunk.memory);
        curl_easy_cleanup(curl);
    }

    return 0;
}