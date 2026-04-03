/**
 * @file lv_multicurve.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_multicurve_private.h"
#if 1//LV_USE_MULTICURVE != 0

#include "../../misc/lv_area_private.h"
#include "../../draw/lv_draw_private.h"
#include "../../draw/lv_draw_vector_private.h"
#include "../../core/lv_obj_private.h"
#include "../../core/lv_obj_class_private.h"
#include "../../core/lv_obj_draw_private.h"
#include "../../misc/lv_assert.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_multicurve_class)

#define LV_MULTICURVE_POINT_CNT_DEF 10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_multicurve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_multicurve_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_multicurve_event(const lv_obj_class_t * class_p, lv_event_t * e);

static void draw_series_line(lv_obj_t * obj, lv_multicurve_series_t* ser, lv_layer_t * layer);
static void invalidate_points(lv_obj_t* obj, lv_multicurve_series_t* series);
static void new_points_alloc(lv_obj_t * obj, lv_multicurve_series_t* series,  uint32_t cnt);
static int32_t value_to_y(lv_obj_t * obj, int32_t v, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_multicurve_class = {
    .constructor_cb = lv_multicurve_constructor,
    .destructor_cb = lv_multicurve_destructor,
    .event_cb = lv_multicurve_event,
    .width_def = LV_PCT(100),
    .height_def = LV_DPI_DEF * 2,
    .instance_size = sizeof(lv_multicurve_t),
    .base_class = &lv_obj_class,
    .name = "lv_multicurve",
    LV_PROPERTY_CLASS_FIELDS(multicurve, MULTICURVE)
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_multicurve_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


void lv_multicurve_refresh(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_invalidate(obj);
}

lv_multicurve_series_t * lv_multicurve_add_series(lv_obj_t * obj, lv_color_t color, uint32_t y, uint32_t h){
    LV_LOG_INFO("begin");

    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_multicurve_t * multicurve    = (lv_multicurve_t *)obj;

    /* Allocate space for a new series and add it to the multicurve series linked list */
    lv_multicurve_series_t * ser = lv_ll_ins_tail(&multicurve->series_ll);
    LV_ASSERT_MALLOC(ser);
    if(ser == NULL) return NULL;
    lv_memzero(ser, sizeof(lv_multicurve_series_t));

    /* Allocate memory for point_cnt points, handle failure below */
    ser->y_points = lv_malloc(sizeof(int32_t) * LV_MULTICURVE_POINT_CNT_DEF);
    LV_ASSERT_MALLOC(ser->y_points);

    if(ser->y_points == NULL) {
        lv_ll_remove(&multicurve->series_ll, ser);
        lv_free(ser);
        return NULL;
    }

    /* Set series properties on successful allocation */
    ser->color = color;
    ser->start_point = 0;
    ser->full = 0;
    ser->h = h;
    ser->y = y;
    ser->point_cnt = LV_MULTICURVE_POINT_CNT_DEF;
    ser->batch_count = 0;
    
    uint32_t i;
    const int32_t def = LV_MULTICURVE_POINT_NONE;
    int32_t * p_tmp = ser->y_points;
    for(i = 0; i < ser->point_cnt; i++) {
        *p_tmp = def;
        p_tmp++;
    }

    return ser;
}

void lv_multicurve_remove_series(lv_obj_t * obj, lv_multicurve_series_t * series){
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(series);

    lv_multicurve_t * multicurve    = (lv_multicurve_t *)obj;
    if(series->y_points) 
        lv_free(series->y_points);
    
    lv_ll_remove(&multicurve->series_ll, series);
    lv_free(series);

    return;
}

void lv_multicurve_hide_series(lv_obj_t * multicurve, lv_multicurve_series_t * series, bool hide){

}

void lv_multicurve_set_point_count_series(lv_obj_t * obj, lv_multicurve_series_t * series, 
                                            uint32_t cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_multicurve_t * multicurve  = (lv_multicurve_t *)obj;
    if(series->point_cnt == cnt) return;

    if(cnt < 1) cnt = 1;
        
    new_points_alloc(obj, series, cnt);
    series->start_point = 0;
    series->point_cnt = cnt;
    series->full = 0;
    series->batch_count;

    lv_multicurve_refresh(obj);
}

uint32_t lv_multicurve_get_point_count_series(const lv_obj_t * obj, lv_multicurve_series_t * series)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    return series->point_cnt;
}

lv_multicurve_series_t * lv_multicurve_get_series_next(const lv_obj_t * obj, const lv_multicurve_series_t * ser){
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_multicurve_t * multicurve  = (lv_multicurve_t *)obj;
    if(ser == NULL) return lv_ll_get_head(&multicurve->series_ll);
    else return lv_ll_get_next(&multicurve->series_ll, ser);
}

void lv_multicurve_set_next_value(lv_obj_t * obj, lv_multicurve_series_t* series, int32_t value){
    LV_ASSERT_OBJ(obj, MY_CLASS);
    
    series->y_points[series->start_point] = value;
    invalidate_points(obj, series);
    series->start_point = series->start_point + 1;
    if(series->start_point >= series->point_cnt){
        series->start_point = 0;
        series->full = 1;
    }
}

void lv_multicurve_set_next_values(lv_obj_t * obj, lv_multicurve_series_t* series, const int32_t values[], size_t values_cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    for(size_t i = 0; i < values_cnt; i++){
        series->y_points[series->start_point] = values[i];
        series->batch_count++;
        series->start_point++;
        if(series->start_point >= series->point_cnt){
            invalidate_points(obj, series);
            series->batch_count = 0;
            series->start_point = 0;
            series->full = 1;
        }
    }

    if(series->batch_count){
        invalidate_points(obj, series);
        series->batch_count = 0;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_multicurve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_multicurve_t * multicurve = (lv_multicurve_t *)obj;

    lv_ll_init(&multicurve->series_ll, sizeof(lv_multicurve_series_t));

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_multicurve_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_multicurve_t * multicurve = (lv_multicurve_t *)obj;
    lv_multicurve_series_t * ser;
    while(multicurve->series_ll.head) {
        ser = lv_ll_get_head(&multicurve->series_ll);
        if(!ser) continue;

        lv_free(ser->y_points);
        lv_ll_remove(&multicurve->series_ll, ser);
        lv_free(ser);
    }
    lv_ll_clear(&multicurve->series_ll);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_multicurve_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    lv_result_t res;

    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_multicurve_t* multicurve = (lv_multicurve_t*)obj;

    if(code == LV_EVENT_DRAW_MAIN) {
        lv_layer_t * layer = lv_event_get_layer(e);
        lv_multicurve_series_t* ser;
        const lv_area_t clip_area_ori = layer->_clip_area;
        LV_LL_READ(&multicurve->series_ll, ser){
            lv_area_t ser_area;
            lv_area_copy(&ser_area, &obj->coords);
            ser_area.y1 += ser->y;
            ser_area.y2 = ser_area.y1 + ser->h;
            lv_area_t clip_area;
            if(lv_area_intersect(&clip_area, &ser_area, &layer->_clip_area)){
                layer->_clip_area = clip_area;
                draw_series_line(obj, ser, layer);
            }
        }
        layer->_clip_area = clip_area_ori;
    }
}

static void draw_series_line(lv_obj_t * obj, lv_multicurve_series_t* series, lv_layer_t * layer)
{
    lv_multicurve_t * multicurve  = (lv_multicurve_t *)obj;
    if(series->point_cnt < 2) return;

    int32_t w     = lv_obj_get_content_width(obj);
    int32_t h     = series->h;
    int32_t x_ofs = obj->coords.x1;
    int32_t y_ofs = obj->coords.y1 + series->y;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.base.layer = layer;
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);
    line_dsc.color = series->color;

    /*If there are at least as many points as pixels then draw only vertical lines*/
   
    uint32_t x_step = w / series->point_cnt;
    uint16_t start = (layer->_clip_area.x1 - x_ofs) / x_step;
    uint16_t end = (layer->_clip_area.x2 - x_ofs) / x_step;
    end = LV_MIN(end, series->point_cnt - 1);
    lv_value_precise_t p_x = x_ofs + start * x_step;
    for(uint16_t i = start; i < end; i++){
        int32_t y1 = series->y_points[i];
        y1 = y_ofs + value_to_y(obj, y1, h);

        int32_t y2 = series->y_points[i + 1];
        y2 = y_ofs + value_to_y(obj, y2, h);

        line_dsc.p1.x = p_x;
        line_dsc.p1.y = y1;
        line_dsc.p2.x = p_x;
        line_dsc.p2.y = y2;

        p_x += x_step;

        lv_draw_line(layer, &line_dsc);
    }

}

static void invalidate_points(lv_obj_t * obj, lv_multicurve_series_t* series)
{
    lv_multicurve_t* multicurve = (lv_multicurve_t*)obj;

    int32_t w  = lv_obj_get_content_width(obj);
    int32_t bwidth = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    int32_t pleft = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    int32_t x_ofs = obj->coords.x1 + pleft + bwidth;
    int32_t x_step = w / (series->point_cnt);

    lv_area_t coords;
    
    coords.y1 = obj->coords.y1 + series->y;
    coords.y2 = coords.y1 + series->h;
    
    int32_t start = series->start_point - series->batch_count - 1;
    int32_t end = series->start_point;
    if(start < 0)
        start = 0;
    if(end >= series->point_cnt){
        end = series->point_cnt - 1;
    }

    coords.x1 = start * x_step + x_ofs;
    coords.x2 = end * x_step + x_ofs;

    lv_obj_invalidate_area(obj, &coords);
}

static void new_points_alloc(lv_obj_t * obj, lv_multicurve_series_t* series, uint32_t cnt)
{
    if((series->y_points) == NULL) return;

    uint32_t point_cnt_old = series->point_cnt;
    uint32_t i;

    if(series->start_point != 0) {
        int32_t * new_points = lv_malloc(sizeof(int32_t) * cnt);
        LV_ASSERT_MALLOC(new_points);
        if(new_points == NULL) return;

        if(cnt >= point_cnt_old) {
            for(i = 0; i < point_cnt_old; i++) {
                new_points[i] =
                    series->y_points[(i + series->start_point) % point_cnt_old]; /*Copy old contents to new array*/
            }
            for(i = point_cnt_old; i < cnt; i++) {
                new_points[i] = LV_MULTICURVE_POINT_NONE; /*Fill up the rest with default value*/
            }
        }
        else {
            for(i = 0; i < cnt; i++) {
                new_points[i] =
                    series->y_points[(i + series->start_point) % point_cnt_old]; /*Copy old contents to new array*/
            }
        }

        /*Switch over pointer from old to new*/
        lv_free(series->y_points);
        series->y_points = new_points;
    }
    else {
        series->y_points = lv_realloc(series->y_points, sizeof(int32_t) * cnt);
        LV_ASSERT_MALLOC(series->y_points);
        if(series->y_points == NULL) return;
        /*Initialize the new points*/
        if(cnt > point_cnt_old) {
            for(i = point_cnt_old - 1; i < cnt; i++) {
                series->y_points[i] = LV_MULTICURVE_POINT_NONE;
            }
        }
    }
}

/**
 * Map a value to a height
 * @param obj   pointer to a multicurve
 * @param ser   pointer to the series
 * @param v     the value to map
 * @param h     the height to which the value needs to be mapped
 * @return      the mapped y-coordinate value corresponding to the input value
 */
static int32_t value_to_y(lv_obj_t * obj,int32_t v, int32_t h)
{
    LV_UNUSED(obj);
    if(v < 0)
        v = 0;
    else if(v > h)
        v = h;

    return h - v;
}

#endif
