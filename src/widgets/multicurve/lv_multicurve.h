/**
 * @file lv_multicurve.h
 *
 */

#ifndef LV_MULTICURVE_H
#define LV_MULTICURVE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"
#include "../../core/lv_obj.h"

#if 1//LV_USE_MULTICURVE != 0

/*********************
 *      DEFINES
 *********************/

/**Default value of points. Can be used to not draw a point*/
#define LV_MULTICURVE_POINT_NONE     (INT32_MAX)
LV_EXPORT_CONST_INT(LV_MULTICURVE_POINT_NONE);

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_multicurve_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_multicurve_id_t {
    LV_PROPERTY_ID(MULTICURVE, POINT_COUNT,        LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_MULTICURVE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a multicurve object
 * @param parent    pointer to an object, it will be the parent of the new multicurve
 * @return          pointer to the created multicurve
 */
lv_obj_t * lv_multicurve_create(lv_obj_t * parent);


/**
 * Refresh a multicurve if its data line has changed
 * @param   obj   pointer to multicurve object
 */
void lv_multicurve_refresh(lv_obj_t * obj);

/*======================
 * Series
 *=====================*/

/**
 * Allocate and add a data series to the multicurve
 * @param obj       pointer to a multicurve object
 * @param color     color of the data series
 * @param axis      the y axis to which the series should be attached (::LV_CHART_AXIS_PRIMARY_Y or ::LV_CHART_AXIS_SECONDARY_Y)
 * @return          pointer to the allocated data series or NULL on failure
 */
lv_multicurve_series_t * lv_multicurve_add_series(lv_obj_t * obj, lv_color_t color, uint32_t y, uint32_t h);

/**
 * Deallocate and remove a data series from a multicurve
 * @param obj       pointer to a multicurve object
 * @param series    pointer to a data series on 'multicurve'
 */
void lv_multicurve_remove_series(lv_obj_t * obj, lv_multicurve_series_t * series);

/**
 * Hide/Unhide a single series of a multicurve.
 * @param multicurve     pointer to a multicurve object.
 * @param series    pointer to a series object
 * @param hide      true: hide the series
 */
void lv_multicurve_hide_series(lv_obj_t * multicurve, lv_multicurve_series_t * series, bool hide);


/**
 * Set the number of points on a data line on a multicurve
 * @param obj       pointer to a multicurve object
 * @param cnt       new number of points on the data lines
 */
void lv_multicurve_set_point_count_series(lv_obj_t * obj, lv_multicurve_series_t * series, uint32_t cnt);

/**
 * Get the data point number per data line on multicurve
 * @param obj       pointer to multicurve object
 * @return          point number on each data line
 */
uint32_t lv_multicurve_get_point_count_series(const lv_obj_t * obj, lv_multicurve_series_t * series);

/**
 * Get the next series.
 * @param multicurve     pointer to a multicurve
 * @param ser      the previous series or NULL to get the first
 * @return          the next series or NULL if there is no more.
 */
lv_multicurve_series_t * lv_multicurve_get_series_next(const lv_obj_t * obj, const lv_multicurve_series_t * ser);

/**
 * Set the next point's Y value according to the update mode policy.
 * @param obj       pointer to multicurve object
 * @param ser       pointer to a data series on 'multicurve'
 * @param value     the new value of the next data
 */
void lv_multicurve_set_next_value(lv_obj_t * obj, lv_multicurve_series_t* series, int32_t value);

/**
 * Same as `lv_multicurve_set_next_value` but set the values from an array
 * @param obj           pointer to multicurve object
 * @param ser           pointer to a data series on 'multicurve'
 * @param values        the new values to set
 * @param values_cnt    number of items in `values`
 */
void lv_multicurve_set_next_values(lv_obj_t * obj, lv_multicurve_series_t* series, const int32_t values[], size_t values_cnt);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_MULTICURVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MULTICURVE_H*/
