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

/** Data of line */
struct _lv_curve_t {
    lv_obj_t obj;
    uint16_t* point_array;              /**< Pointer to an array with the points of the line*/
    uint32_t point_num;                 /**< Number of points in 'point_array'*/
    uint32_t x_step;                    /**< Step between points in x axis*/
    uint32_t current;
    uint32_t full : 1;
    lv_timer_t* timer;
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
