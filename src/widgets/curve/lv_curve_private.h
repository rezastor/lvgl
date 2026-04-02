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

struct _lv_curve_t {
    lv_obj_t obj;
    int32_t * y_points;
    uint32_t start_point;
    uint32_t point_cnt;         //< Number of points
    uint32_t full : 1;
    uint32_t batch_size;
    uint32_t batch_count;
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
