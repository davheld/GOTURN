/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef TRAX_H
#define TRAX_H

#include <stdio.h>
#include <fcntl.h>

#ifndef __TRAX_EXPORT
#if defined(_MSC_VER)
    #define __TRAX_EXPORT __declspec(dllexport)
#elif defined(_GCC)
    #define __TRAX_EXPORT __attribute__((visibility("default")))
#else
    #define __TRAX_EXPORT
#endif
#endif

#if defined(__OS2__) || defined(__WINDOWS__) || defined(WIN32) || defined(WIN64) || defined(_MSC_VER)
    #define TRAX_NO_LOG (~0)
#else
    #define TRAX_NO_LOG -1
#endif

#define TRAX_VERSION 1

#define TRAX_ERROR -1
#define TRAX_HELLO 0
#define TRAX_INITIALIZE 1
#define TRAX_FRAME 2
#define TRAX_QUIT 3
#define TRAX_STATUS 4
#define TRAX_STATE 4

#define TRAX_IMAGE_PATH 0
#define TRAX_IMAGE_URL 1 // Not implemented yet!
#define TRAX_IMAGE_DATA 2 // Not implemented yet!

#define TRAX_REGION_RECTANGLE 1
#define TRAX_REGION_POLYGON 2
#define TRAX_REGION_MASK 3 // Not implemented yet!
#define TRAX_REGION_SPECIAL 0

#define TRAX_FLAG_VALID 1
#define TRAX_FLAG_SERVER 2
#define TRAX_FLAG_TERMINATED 4

#define TRAX_PARAMETER_VERSION 0
#define TRAX_PARAMETER_CLIENT 1
#define TRAX_PARAMETER_SOCKET 2
#define TRAX_PARAMETER_REGION 3

#define TRAX_LOCALHOST "127.0.0.1"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A trax image data structure. At the moment only the path format is supported
 * so a lot of fields are empty.
 * Use trax_image_* functions to access the data.
**/
typedef struct trax_image {
    short type;
    int width;
    int height;
    char* data;
} trax_image;

/**
 * A placeholder for region structure. Use the trax_region_* functions to manipulate
 * the data.
**/
typedef void trax_region;

/**
 * A placeholder for properties structure. Use the trax_properties_* functions to manipulate
 * the data.
**/
typedef struct trax_properties trax_properties;

/**
 * Some basic configuration data used to set up the server.
**/
typedef struct trax_configuration {
        int format_region;
        int format_image;
} trax_configuration;

/**
 * Core object of the protocol. Do not manipulate it directly.
**/
typedef struct trax_handle {
    int flags;
    int version;
    void* stream;
    trax_properties* properties;
    FILE* log;
    trax_configuration config;
} trax_handle;

typedef void(*trax_enumerator)(const char *key, const char *value, const void *obj);

__TRAX_EXPORT const char* trax_version();

/**
 * Setups the protocol state object for the client and returns a handle object.
**/
__TRAX_EXPORT trax_handle* trax_client_setup_file(int input, int output, FILE* log);

/**
 * Setups the protocol state object for the client and returns a handle object.
**/
__TRAX_EXPORT trax_handle* trax_client_setup_socket(int server, FILE* log);

/**
 * Waits for a valid protocol message from the server.
**/
__TRAX_EXPORT int trax_client_wait(trax_handle* client, trax_region** region, trax_properties* properties);

/**
 * Sends an initialize message.
**/
__TRAX_EXPORT void trax_client_initialize(trax_handle* client, trax_image* image, trax_region* region, trax_properties* properties);

/**
 * Sends a frame message.
**/
__TRAX_EXPORT void trax_client_frame(trax_handle* client, trax_image* image, trax_properties* properties);

/**
 * Setups the protocol for the server side and returns a handle object.
**/
__TRAX_EXPORT trax_handle* trax_server_setup(trax_configuration config, FILE* log);

/**
 * Setups the protocol for the server side and returns a handle object.
**/
__TRAX_EXPORT trax_handle* trax_server_setup_file(trax_configuration config, int input, int output, FILE* log);

/**
 * Waits for a valid protocol message from the client.
**/
__TRAX_EXPORT int trax_server_wait(trax_handle* server, trax_image** image, trax_region** region, trax_properties* properties);

/**
 * Sends a status reply to the client.
**/
__TRAX_EXPORT void trax_server_reply(trax_handle* server, trax_region* region, trax_properties* properties);

/**
 * Used in client and server. Closes communication, sends quit message if needed.
 * Releases the handle structure.
**/
__TRAX_EXPORT int trax_cleanup(trax_handle** handle);

/**
 * Sets the parameter for the client or server instance.
**/
__TRAX_EXPORT int trax_set_parameter(trax_handle* handle, int id, int value);

/**
 * Gets the parameter for the client or server instance.
**/
__TRAX_EXPORT int trax_get_parameter(trax_handle* handle, int id, int* value);

/**
 * Releases image structure, frees allocated memory.
**/
__TRAX_EXPORT void trax_image_release(trax_image** image);

/**
 * Creates a file-system path image description.
**/
__TRAX_EXPORT trax_image* trax_image_create_path(const char* path);

/**
 * Returns a file path from a file-system path image description. This function
 * returns a pointer to the internal data which should not be modified.
**/
__TRAX_EXPORT const char* trax_image_get_path(trax_image* image);

/**
 * Releases region structure, frees allocated memory.
**/
__TRAX_EXPORT void trax_region_release(trax_region** region);

/**
 * Returns type identifier of the region object.
**/
__TRAX_EXPORT int trax_region_get_type(const trax_region* region);

/**
 * Creates a special region object. Only one paramter (region code) required.
**/
__TRAX_EXPORT trax_region* trax_region_create_special(int code);

/**
 * Sets the code of a special region.
**/
__TRAX_EXPORT void trax_region_set_special(trax_region* region, int code);

/**
 * Returns a code of a special region object.
**/
__TRAX_EXPORT int trax_region_get_special(const trax_region* region);

/**
 * Creates a rectangle region.
**/
__TRAX_EXPORT trax_region* trax_region_create_rectangle(float x, float y, float width, float height);

/**
 * Sets the coordinates for a rectangle region.
**/
__TRAX_EXPORT void trax_region_set_rectangle(trax_region* region, float x, float y, float width, float height);

/**
 * Retreives coordinate from a rectangle region object.
**/
__TRAX_EXPORT void trax_region_get_rectangle(const trax_region* region, float* x, float* y, float* width, float* height);

/**
 * Creates a polygon region object for a given amout of points. Note that the coordinates of the points
 * are arbitrary and have to be set after allocation.
**/
__TRAX_EXPORT trax_region* trax_region_create_polygon(int count);

/**
 * Sets coordinates of a given point in the polygon.
**/
__TRAX_EXPORT void trax_region_set_polygon_point(trax_region* region, int index, float x, float y);

/**
 * Retrieves the coordinates of a specific point in the polygon.
**/
__TRAX_EXPORT void trax_region_get_polygon_point(const trax_region* region, int index, float* x, float* y);

/**
 * Returns the number of points in the polygon.
**/
__TRAX_EXPORT int trax_region_get_polygon_count(const trax_region* region);

/**
 * Creates a rectangle region object that bounds the input region (in case the input
 * region is also a rectangle it just clones it).
 **/
__TRAX_EXPORT trax_region* trax_region_get_bounds(const trax_region* region);

/**
 * Destroy a properties object and clean up the memory.
 **/
__TRAX_EXPORT void trax_properties_release(trax_properties** properties);

/**
 * Clear a properties object.
 **/
__TRAX_EXPORT void trax_properties_clear(trax_properties* properties);

/**
 * Create a property object.
 **/
__TRAX_EXPORT trax_properties* trax_properties_create();

/**
 * Set a string property (the value string is cloned).
 **/
__TRAX_EXPORT void trax_properties_set(trax_properties* properties, const char* key, const char* value);

/**
 * Set an integer property. The value will be encoded as a string.
 **/
__TRAX_EXPORT void trax_properties_set_int(trax_properties* properties, const char* key, int value);

/**
 * Set an floating point value property. The value will be encoded as a string.
 **/
__TRAX_EXPORT void trax_properties_set_float(trax_properties* properties, const char* key, float value);

/**
 * Get a string property. The resulting string is a clone of the one stored so it should
 * be released when not needed anymore.
 **/
__TRAX_EXPORT char* trax_properties_get(const trax_properties* properties, const char* key);

/**
 * Get an integer property. A stored string value is converted to an integer. If this is not possible
 * or the property does not exist a given default value is returned.
 **/
__TRAX_EXPORT int trax_properties_get_int(const trax_properties* properties, const char* key, int def);

/**
 * Get an floating point value property. A stored string value is converted to an integer. If this is not possible
 * or the property does not exist a given default value is returned.
 **/
__TRAX_EXPORT float trax_properties_get_float(const trax_properties* properties, const char* key, float def);

/**
 * Iterate over the property set using a callback function. An optional pointer can be given and is forwarded
 * to the callback.
 **/
__TRAX_EXPORT void trax_properties_enumerate(trax_properties* properties, trax_enumerator enumerator, void* object);

#ifdef __cplusplus
}
#endif

#endif

