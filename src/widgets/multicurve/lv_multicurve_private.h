/**
 * @file lv_multicurve_private.h
 *
 */

#ifndef LV_MULTICURVE_PRIVATE_H
#define LV_MULTICURVE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../core/lv_obj_private.h"
#include "lv_multicurve.h"

#if 1//LV_USE_MULTICURVE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_multicurve_series_t {
    int32_t * y_points;
    uint32_t start_point;
    uint32_t point_cnt;         //< Number of points
    uint32_t batch_count;
    uint32_t full : 1;
    uint32_t y;
    uint32_t h;
    lv_color_t color;
};

struct _lv_multicurve_t {
    lv_obj_t obj;
    lv_ll_t series_ll;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_MULTICURVE != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MULTICURVE_PRIVATE_H*/
