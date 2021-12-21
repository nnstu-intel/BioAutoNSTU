#ifndef CWRAPPER_H
#define CWRAPPER_H

#include <stdlib.h>
#include "macros_defs.h"

EXTERN_C
    // Returns error message if any exception occured else returns NULL
    // The message is returned only once
    API const char* receive_error();

    // Returns EXIT_SUCCESS or EXIT_FAILURE
    // Initializes the OpenVINO face classifier
    API int init_ie_facenet_v1(const char* xml, const char* bin, const char* device);

    // Returns EXIT_SUCCESS or EXIT_FAILURE
    // Releases the OpenVINO face classifier
    API int release_ie_facenet_v1();

    // Returns EXIT_SUCCESS or EXIT_FAILURE
    // Compute distance between two descriptors
    API int compute_distance(
        const float* dist1,
        const float* dist2,
        const int size,
        float& result
    );

    // Returns EXIT_SUCCESS or EXIT_FAILURE
    // Compute embedding for an image
    // You need to release embedding vector after you used it with free() function
    API int compute_embedding(
        const int height,
        const int width,
        const int type,
        const int step,
        const u_char* data,
        const int data_size,
        float *const result,
        int& result_size
    );
EXTERN_C_END

#endif
