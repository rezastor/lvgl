/**
 * @file lv_curve.h
 *
 */

#ifndef LV_CURVE_H
#define LV_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"
#include "../../core/lv_obj.h"

#if LV_USE_CURVE != 0

/*********************
 *      DEFINES
 *********************/

/**Default value of points. Can be used to not draw a point*/
#define LV_CURVE_POINT_NONE     (INT32_MAX)
LV_EXPORT_CONST_INT(LV_CURVE_POINT_NONE);

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_curve_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_curve_id_t {
    LV_PROPERTY_ID(CURVE, POINT_COUNT,        LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_CURVE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a curve object
 * @param parent    pointer to an object, it will be the parent of the new curve
 * @return          pointer to the created curve
 */
lv_obj_t * lv_curve_create(lv_obj_t * parent);

/**
 * Set the number of points on a data line on a curve
 * @param obj       pointer to a curve object
 * @param cnt       new number of points on the data lines
 */
void lv_curve_set_point_count(lv_obj_t * obj, uint32_t cnt);

/**
 * Get the data point number per data line on curve
 * @param obj       pointer to curve object
 * @return          point number on each data line
 */
uint32_t lv_curve_get_point_count(const lv_obj_t * obj);

/**
 * Get the current index of the x-axis start point in the data array
 * @param obj       pointer to a curve object
 * @param ser       pointer to a data series on 'curve'
 * @return          the index of the current x start point in the data array
 */
uint32_t lv_curve_get_x_start_point(const lv_obj_t * obj, lv_curve_series_t * ser);

/**
 * Get the position of a point to the curve.
 * @param obj       pointer to a curve object
 * @param ser       pointer to series
 * @param id        the index.
 * @param p_out     store the result position here
 */
void lv_curve_get_point_pos_by_id(lv_obj_t * obj, lv_curve_series_t * ser, uint32_t id, lv_point_t * p_out);

/**
 * Refresh a curve if its data line has changed
 * @param   obj   pointer to curve object
 */
void lv_curve_refresh(lv_obj_t * obj);

/**
 * Set the next point's Y value according to the update mode policy.
 * @param obj       pointer to curve object
 * @param ser       pointer to a data series on 'curve'
 * @param value     the new value of the next data
 */
void lv_curve_set_next_value(lv_obj_t * obj, int32_t value);

/**
 * Same as `lv_curve_set_next_value` but set the values from an array
 * @param obj           pointer to curve object
 * @param ser           pointer to a data series on 'curve'
 * @param values        the new values to set
 * @param values_cnt    number of items in `values`
 */
void lv_curve_set_next_values(lv_obj_t * obj, const int32_t values[], size_t values_cnt);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_CURVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CURVE_H*/
