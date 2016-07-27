#include "vot.h"

#include <cfloat>

#define VOT_RECTANGLE

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef VOT_RECTANGLE
#define VOT_POLYGON
#endif

#ifdef VOT_POLYGON
void vot_region_release(vot_region** region) {
    if (!(*region)) return;

    if ((*region)->x) {
        free((*region)->x);
        (*region)->x = NULL;
    }
    if ((*region)->y) {
        free((*region)->y);
        (*region)->y = NULL;
    }

    free(*region);

    *region = NULL;
}

vot_region* vot_region_create(int n) {
    vot_region* region = (vot_region*) malloc(sizeof(vot_region));
    region->x = (float *) malloc(sizeof(float) * n);
    region->y = (float *) malloc(sizeof(float) * n);
    memset(region->x, 0, sizeof(float) * n);
    memset(region->y, 0, sizeof(float) * n);
    region->count = n;
    return region;
}

vot_region* vot_region_copy(const vot_region* region) {
    vot_region* copy = vot_region_create(region->count);
    int i;
    for (i = 0; i < region->count; i++) {
        copy->x[i] = region->x[i];
        copy->y[i] = region->y[i];
    }
    return copy;
}

#else

void vot_region_release(vot_region** region) {

    if (!(*region)) return;

    free(*region);

    *region = NULL;

}

vot_region* vot_region_create() {
    vot_region* region = (vot_region*) malloc(sizeof(vot_region));
    region->x = 0;
    region->y = 0;
    region->width = 0;
    region->height = 0;
    return region;
}

vot_region* vot_region_copy(const vot_region* region) {
    vot_region* copy = vot_region_create();
    copy->x = region->x;
    copy->y = region->y;
    copy->width = region->width;
    copy->height = region->height;
    return copy;
}


#endif // VOT_POLYGON

vot_region* _parse_region(char* buffer) {

    int i;
    float* numbers = (float*) malloc(sizeof(float) * (strlen(buffer) / 2));
  char* pch = strtok(buffer, ",");

    for (i = 0; ; i++) {
        if (!pch) break;
        numbers[i] = (float) atof(pch);
    pch = strtok(NULL, ",");
    }

    vot_region* region;

#ifdef VOT_POLYGON
    {
        // Check if region is actually a rectangle and convert it
        if (i == 4) {

            region = vot_region_create(4);

      region->count = 4;

      region->x[0] = numbers[0];
      region->x[1] = numbers[0] + numbers[2];
      region->x[2] = numbers[0] + numbers[2];
      region->x[3] = numbers[0];

      region->y[0] = numbers[1];
      region->y[1] = numbers[1];
      region->y[2] = numbers[1] + numbers[3];
      region->y[3] = numbers[1] + numbers[3];


        } else {
            int count = i / 2;
            assert(count >= 3);

            region = vot_region_create(count);

            for (i = 0; i < count; i++) {
                region->x[i] = numbers[i*2];
                region->y[i] = numbers[i*2+1];
            }

            region->count = count;
        }
    }
#else
    {
        assert(i > 3);

        region = vot_region_create();

        // Check if the input region is actually a polygon and convert it
        if (i > 6) {
            int j;
        float top = FLT_MAX;
        float bottom = FLT_MIN;
        float left = FLT_MAX;
        float right = FLT_MIN;

        for (j = 0; j < i / 2; j++) {
          top = MIN(top, numbers[j * 2 + 1]);
          bottom = MAX(bottom, numbers[j * 2 + 1]);
          left = MIN(left, numbers[j * 2]);
          right = MAX(right, numbers[j * 2]);
        }

          region->x = left;
          region->y = top;
          region->width = right - left;
          region->height = bottom - top;

        } else {
            region = vot_region_create();
            region->x = numbers[0];
            region->y = numbers[1];
            region->width = numbers[2];
            region->height = numbers[3];
        }
    }
#endif // VOT_POLYGON

    free(numbers);

    return region;
}

/**
 * Reads the input data and initializes all structures. Returns the initial
 * position of the object as specified in the input data. This function should
 * be called at the beginning of the program.
 */
vot_region* VOT_PREFIX(vot_initialize)() {

    int i, j;
    FILE *inputfile;
    FILE *imagesfile;

    _vot_sequence_position = 0;
    _vot_sequence_size = 0;

#ifdef VOT_TRAX
    if (getenv("TRAX")) {
        trax_configuration config;
        trax_image* _trax_image = NULL;
        trax_region* _trax_region = NULL;
        int response;

        #ifdef VOT_POLYGON
        config.format_region = TRAX_REGION_POLYGON;
        #else
        config.format_region = TRAX_REGION_RECTANGLE;
        #endif
        config.format_image = TRAX_IMAGE_PATH;
        _trax_handle = trax_server_setup(config, NULL);

        response = trax_server_wait(_trax_handle, &_trax_image, &_trax_region, NULL);

        assert(response == TRAX_INITIALIZE);

        strcpy(_trax_image_buffer, trax_image_get_path(_trax_image));

        trax_server_reply(_trax_handle, _trax_region, NULL);

        vot_region* region = _trax_to_region(_trax_region);

        trax_region_release(&_trax_region);
        trax_image_release(&_trax_image);

        return region;
    }
#endif // VOT_TRAX

    inputfile = fopen("region.txt", "r");
    imagesfile = fopen("images.txt", "r");

    if (!inputfile) {
        fprintf(stderr, "Initial region file (region.txt) not available. Stopping.\n");
        exit(-1);
    }

    if (!imagesfile) {
        fprintf(stderr, "Image list file (images.txt) not available. Stopping.\n");
        exit(-1);
    }

    int linelen;
    size_t linesiz = sizeof(char) * VOT_READ_BUFFER;
    char* linebuf = (char*) malloc(sizeof(char) * VOT_READ_BUFFER);

    getline(&linebuf, &linesiz, inputfile);
    vot_region* region = _parse_region(linebuf);

    fclose(inputfile);

    j = 32;
    _vot_sequence = (char**) malloc(sizeof(char*) * j);

    while (1) {

        if ((linelen=getline(&linebuf, &linesiz, imagesfile))<1)
            break;

        if ((linebuf)[linelen - 1] == '\n') {
            (linebuf)[linelen - 1] = '\0';
        }

        if (_vot_sequence_size == j) {
            j += 32;
            _vot_sequence = (char**) realloc(_vot_sequence, sizeof(char*) * j);
        }

        _vot_sequence[_vot_sequence_size] = (char *) malloc(sizeof(char) * (strlen(linebuf) + 1));

        strcpy(_vot_sequence[_vot_sequence_size], linebuf);

        _vot_sequence_size++;
    }

    free(linebuf);

    _vot_result = (vot_region**) malloc(sizeof(vot_region*) * _vot_sequence_size);

    return region;
}

/**
 * Stores results to the result file and frees memory. This function should be
 * called at the end of the tracking program.
 */
void VOT_PREFIX(vot_quit)() {

    int i;

#ifdef VOT_TRAX
    if (_trax_handle) {
        trax_cleanup(&_trax_handle);
        return;
    }
#endif // VOT_TRAX

    FILE *outputfile = fopen("output.txt", "w");

    for (i = 0; i < _vot_sequence_position; i++) {
#ifdef VOT_POLYGON
        {
            int j;
            fprintf(outputfile, "%f,%f", _vot_result[i]->x[0], _vot_result[i]->y[0]);
            for (j = 1; j < _vot_result[i]->count; j++)
                fprintf(outputfile, ",%f,%f", _vot_result[i]->x[j], _vot_result[i]->y[j]);
            fprintf(outputfile, "\n");
        }
#else
        fprintf(outputfile, "%f,%f,%f,%f\n", _vot_result[i]->x, _vot_result[i]->y, _vot_result[i]->width, _vot_result[i]->height);
#endif // VOT_POLYGON
        vot_region_release(&(_vot_result[i]));
    }

    fclose(outputfile);

    if (_vot_sequence) {
        for (i = 0; i < _vot_sequence_size; i++)
            free(_vot_sequence[i]);

        free(_vot_sequence);
    }

    if (_vot_result)
        free(_vot_result);

}

/**
 * Returns the file name of the current frame. This function does not advance
 * the current position.
 */
const char* VOT_PREFIX(vot_frame)() {

#ifdef VOT_TRAX
    if (_trax_handle) {
        int response;
        trax_image* _trax_image = NULL;
        trax_region* _trax_region = NULL;

        if (_vot_sequence_position == 0) {
            _vot_sequence_position++;
            return _trax_image_buffer;
        }

        response = trax_server_wait(_trax_handle, &_trax_image, &_trax_region, NULL);

        if (response != TRAX_FRAME) {
            vot_quit();
            exit(0);
        }

        strcpy(_trax_image_buffer, trax_image_get_path(_trax_image));
        trax_image_release(&_trax_image);

        return _trax_image_buffer;

    }
#endif // VOT_TRAX

    if (_vot_sequence_position >= _vot_sequence_size)
        return NULL;

    return _vot_sequence[_vot_sequence_position];

}

/**
 * Used to report position of the object. This function also advances the
 * current position.
 */
void VOT_PREFIX(vot_report)(vot_region* region) {

#ifdef VOT_TRAX
    if (_trax_handle) {
        trax_region* _trax_region = _region_to_trax(region);
        trax_server_reply(_trax_handle, _trax_region, NULL);
        trax_region_release(&_trax_region);
        return;
    }
#endif // VOT_TRAX

    if (_vot_sequence_position >= _vot_sequence_size)
        return;

    _vot_result[_vot_sequence_position] = vot_region_copy(region);
    _vot_sequence_position++;
}

int VOT_PREFIX(vot_end)() {

#ifdef VOT_TRAX
    return 0;
#endif // VOT_TRAX

    if (_vot_sequence_position >= _vot_sequence_size)
        return 1;

    return 0;
}
