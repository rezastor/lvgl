/**
 * @file lv_curve.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_curve_private.h"
#if LV_USE_CURVE != 0

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
#define MY_CLASS (&lv_curve_class)

#define LV_CURVE_POINT_CNT_DEF 10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_curve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_curve_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_curve_event(const lv_obj_class_t * class_p, lv_event_t * e);

static void draw_series_line(lv_obj_t * obj, lv_layer_t * layer);
static uint32_t get_index_from_x(lv_obj_t * obj, int32_t x);
static void invalidate_points(lv_obj_t* obj);
static void invalidate_point(lv_obj_t * obj, uint32_t i);
static void new_points_alloc(lv_obj_t * obj, uint32_t cnt, int32_t ** a);
static int32_t value_to_y(lv_obj_t * obj, int32_t v, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

#if LV_USE_OBJ_PROPERTY
static const lv_property_ops_t lv_curve_properties[] = {
    {
        .id = LV_PROPERTY_CURVE_POINT_COUNT,
        .setter = lv_curve_set_point_count,
        .getter = lv_curve_get_point_count,
    },
};
#endif

const lv_obj_class_t lv_curve_class = {
    .constructor_cb = lv_curve_constructor,
    .destructor_cb = lv_curve_destructor,
    .event_cb = lv_curve_event,
    .width_def = LV_PCT(100),
    .height_def = LV_DPI_DEF * 2,
    .instance_size = sizeof(lv_curve_t),
    .base_class = &lv_obj_class,
    .name = "lv_curve",
    LV_PROPERTY_CLASS_FIELDS(curve, CURVE)
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_curve_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_curve_set_point_count(lv_obj_t * obj, uint32_t cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_curve_t * curve  = (lv_curve_t *)obj;
    if(curve->point_cnt == cnt) return;

    if(cnt < 1) cnt = 1;
        
    new_points_alloc(obj, cnt, &curve->y_points);
    curve->start_point = 0;
    curve->batch_count = 0;
    curve->point_cnt = cnt;
    curve->full = 0;

    lv_curve_refresh(obj);
}

uint32_t lv_curve_get_point_count(const lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_curve_t * curve  = (lv_curve_t *)obj;
    return curve->point_cnt;
}

void lv_curve_refresh(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_invalidate(obj);
}


void lv_curve_set_next_value(lv_obj_t * obj, int32_t value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_curve_t * curve    = (lv_curve_t *)obj;

    curve->y_points[curve->start_point] = value;
    curve->batch_count++;
    if(curve->start_point == curve->point_cnt - 1 ||
        curve->batch_count >= curve->batch_size){
            invalidate_points(obj);
            curve->batch_count = 0;
    }
    
    curve->start_point = curve->start_point + 1;
    if(curve->start_point >= curve->point_cnt){
        curve->start_point = 0;
        curve->full = 1;
    }
}

void lv_curve_set_series_values(lv_obj_t * obj, const int32_t values[], size_t values_cnt)
{
    size_t i;
    for(i = 0; i < values_cnt; i++) {
        lv_curve_set_next_value(obj, values[i]);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_curve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_curve_t * curve = (lv_curve_t *)obj;

    curve->point_cnt   = LV_CURVE_POINT_CNT_DEF;
    curve->batch_count = 0;
    curve->batch_size = 3;
    curve->start_point = 0;
    curve->full = 0;
    /* Allocate memory for point_cnt points, handle failure below */
    curve->y_points = lv_malloc(sizeof(int32_t) * curve->point_cnt);
    LV_ASSERT_MALLOC(curve->y_points);

    int32_t i;
    const int32_t def = LV_CURVE_POINT_NONE;
    int32_t * p_tmp = curve->y_points;
    for(i = 0; i < curve->point_cnt; i++) {
        *p_tmp = def;
        p_tmp++;
    }

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_curve_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_curve_t * curve = (lv_curve_t *)obj;
    lv_free(curve->y_points);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_curve_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    lv_result_t res;

    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    lv_curve_t * curve  = (lv_curve_t *)obj;
    if(code == LV_EVENT_DRAW_MAIN) {
        lv_layer_t * layer = lv_event_get_layer(e);

        lv_area_t ext_coords;
        lv_obj_get_coords(obj, &ext_coords);
        int32_t ext_draw_size = lv_obj_get_ext_draw_size(obj);
        lv_area_increase(&ext_coords, ext_draw_size, ext_draw_size);

        lv_area_t clip_area;
        if(lv_area_intersect(&clip_area, &ext_coords, &layer->_clip_area)) {
            const lv_area_t clip_area_ori = layer->_clip_area;
            layer->_clip_area = clip_area;
            draw_series_line(obj, layer);
            layer->_clip_area = clip_area_ori;
        }
    }
}

static void draw_series_line(lv_obj_t * obj, lv_layer_t * layer)
{
    lv_curve_t * curve  = (lv_curve_t *)obj;
    if(curve->point_cnt < 2) return;

    int32_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    int32_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
    int32_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
    int32_t w     = lv_obj_get_content_width(obj);
    int32_t h     = lv_obj_get_content_height(obj);
    int32_t x_ofs = obj->coords.x1 + pad_left - lv_obj_get_scroll_left(obj);
    int32_t y_ofs = obj->coords.y1 + pad_top - lv_obj_get_scroll_top(obj);
    lv_curve_series_t * ser;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.base.layer = layer;
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);

    /*If there are at least as many points as pixels then draw only vertical lines*/
    bool crowded_mode = (int32_t)curve->point_cnt >= w;

    int32_t bullet_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
    int32_t bullet_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;
    int32_t extra_space_x;
    if(curve->point_cnt <= 1) extra_space_x = 0;
    else extra_space_x = w  / (curve->point_cnt - 1) + bullet_w + line_dsc.width;

    lv_draw_rect_dsc_t point_draw_dsc;
    if(crowded_mode == false) {
        lv_draw_rect_dsc_init(&point_draw_dsc);
        lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &point_draw_dsc);
        point_draw_dsc.base.id1 = line_dsc.base.id1;
    }

    lv_point_precise_t * points = NULL;
    if(crowded_mode) {
        points = lv_malloc((w + 2 * extra_space_x) * 3 * sizeof(lv_point_precise_t));
    }
    else {
        points = lv_malloc(curve->point_cnt * sizeof(lv_point_precise_t));
    }

    if(points == NULL) {
        LV_LOG_WARN("Couldn't allocate the points array");
        return;
    }

    line_dsc.points = points;

    /*Go through all data lines*/
    
    //line_dsc.color = ser->color;
    //line_dsc.base.drop_shadow_color = ser->color;

    int32_t start_point = 0;
    int32_t p_act = start_point;
    int32_t p_prev = start_point;

    lv_value_precise_t y_min = obj->coords.y2;
    lv_value_precise_t y_max = obj->coords.y1;
    lv_value_precise_t x_prev = -10000;
    line_dsc.p1.x = x_ofs;
    line_dsc.p2.x = x_ofs;
    line_dsc.point_cnt = 0;

    uint32_t i;
    for(i = 0; i < curve->point_cnt; i++) {
        lv_value_precise_t p_x = (int32_t)((w * i) / (curve->point_cnt - 1)) + x_ofs;
        if(p_x > layer->_clip_area.x2 + extra_space_x + 1) break;
        if(p_x < layer->_clip_area.x1 - extra_space_x - 1) {
            p_prev = p_act;
            continue;
        }
        p_act = (start_point + i) % curve->point_cnt;

        lv_value_precise_t p_y;
        if(curve->y_points[p_act] == LV_CURVE_POINT_NONE) {
            p_y = LV_DRAW_LINE_POINT_NONE;
        }
        else {
            int32_t v = curve->y_points[p_act];
            int32_t min_v = 0;
            int32_t max_v = h;
            p_y = (int32_t)lv_map(v, min_v, max_v, y_ofs + h, y_ofs);
        }

        /*In normal mode just collect the points here*/
        if(crowded_mode == false) {
            points[line_dsc.point_cnt].x = p_x;
            points[line_dsc.point_cnt].y = p_y;
            line_dsc.point_cnt++;
        }
        /*In crowded mode draw vertical lines from the min/max on the same X coordinate*/
        else {
            if(curve->y_points[p_prev] != LV_CURVE_POINT_NONE && curve->y_points[p_act] != LV_CURVE_POINT_NONE) {
                /*Draw only one vertical line between the min and max y-values on the same x-value*/
                y_max = LV_MAX(y_max, p_y);
                y_min = LV_MIN(y_min, p_y);
                if(x_prev != p_x) {
                    line_dsc.points[line_dsc.point_cnt].y = y_min;
                    line_dsc.points[line_dsc.point_cnt].x = p_x;
                    line_dsc.points[line_dsc.point_cnt + 1].y = y_max;
                    line_dsc.points[line_dsc.point_cnt + 1].x = p_x;
                    line_dsc.points[line_dsc.point_cnt + 2].y = LV_DRAW_LINE_POINT_NONE;
                    line_dsc.points[line_dsc.point_cnt + 2].x = p_x;

                    /*If they are the same no line would be drawn*/
                    if(line_dsc.points[line_dsc.point_cnt].y == line_dsc.points[line_dsc.point_cnt + 1].y) {
                        line_dsc.points[line_dsc.point_cnt + 1].y++;
                    }
                    y_min = p_y;  /*Start the line of the next x from the current last y*/
                    y_max = p_y;
                    x_prev = p_x;
                    line_dsc.point_cnt += 3;
                }
            }
        }

        p_prev = p_act;
    }

    /*Draw the line from the accumulated points*/
    lv_draw_line(layer, &line_dsc);

    if(points) lv_free(points);
}

/**
 * Get the nearest index to an X coordinate
 * @param curve pointer to a curve object
 * @param coord the coordination of the point relative to the series area.
 * @return the found index
 */
static uint32_t get_index_from_x(lv_obj_t * obj, int32_t x)
{
    lv_curve_t * curve  = (lv_curve_t *)obj;
    int32_t w = lv_obj_get_content_width(obj);
    int32_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    x -= pad_left;

    if(x < 0) return 0;
    if(x > w) return curve->point_cnt - 1;
    return (x * (curve->point_cnt - 1) + w / 2) / w;
}

static void invalidate_point(lv_obj_t * obj, uint32_t i)
{
    lv_curve_t * curve  = (lv_curve_t *)obj;
    if(i >= curve->point_cnt) return;

    int32_t w  = lv_obj_get_content_width(obj);
    int32_t bwidth = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    int32_t pleft = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    int32_t x_ofs = obj->coords.x1 + pleft + bwidth;
    int32_t x_step = w / (curve->point_cnt);

    int32_t line_width = lv_obj_get_style_line_width(obj, LV_PART_ITEMS);

    lv_area_t coords;
    lv_area_copy(&coords, &obj->coords);
    coords.y1 -= line_width;
    coords.y2 += line_width;

    /*Invalidate the area between the previous and the next points*/
    if(i == curve->point_cnt - 1) {
        coords.x1 = (i - 1) * x_step + x_ofs - line_width;
        coords.x2 = i * x_step + x_ofs - line_width;
    }
    else if(i == 0){
        coords.x1 = i * x_step + x_ofs - line_width;
        coords.x2 = (i + 1) * x_step + x_ofs - line_width;
    }
    else{
        coords.x1 = (i - 1) * x_step + x_ofs - line_width;
        coords.x2 = (i + 1) * x_step + x_ofs - line_width;
    }
    lv_obj_invalidate_area(obj, &coords);
}

static void invalidate_points(lv_obj_t * obj)
{
    lv_curve_t* curve = (lv_curve_t*)obj;

    int32_t w  = lv_obj_get_content_width(obj);
    int32_t bwidth = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    int32_t pleft = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    int32_t x_ofs = obj->coords.x1 + pleft + bwidth;
    int32_t x_step = w / (curve->point_cnt);

    int32_t line_width = lv_obj_get_style_line_width(obj, LV_PART_ITEMS);

    lv_area_t coords;
    lv_area_copy(&coords, &obj->coords);
    coords.y1 -= line_width;
    coords.y2 += line_width;

    int32_t end = curve->start_point;
    int32_t start = end - curve->batch_count;
    if(start < 0)
        start = 0;

    coords.x1 = start * x_step + x_ofs - line_width;
    coords.x2 = end * x_step + x_ofs - line_width;

    lv_obj_invalidate_area(obj, &coords);
}

static void new_points_alloc(lv_obj_t * obj, uint32_t cnt, int32_t ** a)
{
    if((*a) == NULL) return;

    lv_curve_t * curve = (lv_curve_t *) obj;
    uint32_t point_cnt_old = curve->point_cnt;
    uint32_t i;

    if(curve->start_point != 0) {
        int32_t * new_points = lv_malloc(sizeof(int32_t) * cnt);
        LV_ASSERT_MALLOC(new_points);
        if(new_points == NULL) return;

        if(cnt >= point_cnt_old) {
            for(i = 0; i < point_cnt_old; i++) {
                new_points[i] =
                    (*a)[(i + curve->start_point) % point_cnt_old]; /*Copy old contents to new array*/
            }
            for(i = point_cnt_old; i < cnt; i++) {
                new_points[i] = LV_CURVE_POINT_NONE; /*Fill up the rest with default value*/
            }
        }
        else {
            for(i = 0; i < cnt; i++) {
                new_points[i] =
                    (*a)[(i + curve->start_point) % point_cnt_old]; /*Copy old contents to new array*/
            }
        }

        /*Switch over pointer from old to new*/
        lv_free((*a));
        (*a) = new_points;
    }
    else {
        (*a) = lv_realloc((*a), sizeof(int32_t) * cnt);
        LV_ASSERT_MALLOC((*a));
        if((*a) == NULL) return;
        /*Initialize the new points*/
        if(cnt > point_cnt_old) {
            for(i = point_cnt_old - 1; i < cnt; i++) {
                (*a)[i] = LV_CURVE_POINT_NONE;
            }
        }
    }
}

/**
 * Map a value to a height
 * @param obj   pointer to a curve
 * @param ser   pointer to the series
 * @param v     the value to map
 * @param h     the height to which the value needs to be mapped
 * @return      the mapped y-coordinate value corresponding to the input value
 */
static int32_t value_to_y(lv_obj_t * obj,int32_t v, int32_t h)
{
    lv_curve_t * curve = (lv_curve_t *) obj;

    return lv_map(v, 0, h, 0, h);
}

#endif
