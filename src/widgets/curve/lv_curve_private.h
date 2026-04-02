/**
 * @file lv_curve_private.h
 *
 */

#ifndef LV_CURVE_PRIVATE_H
#define LV_CURVE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../core/lv_obj_private.h"
#include "lv_curve.h"

#if LV_USE_CURVE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Descriptor a curve series
 */
struct _lv_curve_series_t{
    int32_t * x_points;
    int32_t * y_points;
    lv_color_t color;
    uint32_t start_point;
    uint32_t hidden : 1;
};

struct _lv_curve_t {
    lv_obj_t obj;
    lv_curve_series_t* series;
    uint32_t point_cnt;         /**< Number of points in all series */
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_CURVE != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CURVE_PRIVATE_H*/
