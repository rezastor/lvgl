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
#include "../../core/lv_obj.h"
#if LV_USE_CURVE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_curve_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_line_id_t {
    LV_PROPERTY_ID(LINE, Y_INVERT, LV_PROPERTY_TYPE_BOOL, 0),
    LV_PROPERTY_LINE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a curve object
 * @param parent pointer to an object, it will be the parent of the new curve
 * @return pointer to the created curve
 */
lv_obj_t * lv_curve_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Установить коэффициент децимации (пикселей на точку)
 * @param obj       pointer to a curve object
 * @param decim     number of pixels per point
 */
void lv_curve_set_decim(lv_obj_t* obj, uint16_t decim);

/**
 * Добавить точку в график
 * @param obj       pointer to a curve object
 * @param y_point   Y coordinate of the point (0 = top, height = bottom)
 */
void lv_curve_add_point(lv_obj_t* obj, uint16_t y_point);

/**
 * Очистить график
 * @param obj       pointer to a curve object
 */
void lv_curve_clear(lv_obj_t* obj);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the pointer to the array of points.
 * @param obj           pointer to a line object
 * @return              const pointer to the array of points
 */
const uint16_t* lv_curve_get_points(lv_obj_t * obj);

/**
 * Get the number of points in the array of points.
 * @param obj           pointer to a line object
 * @return              number of points in array of points
 */
uint32_t lv_curve_get_point_count(lv_obj_t * obj);

uint16_t lv_curve_get_decim(lv_obj_t* obj);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_CURVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CURVE_H*/
