/**
 * @file lv_curve.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_curve_private.h"
#include "../../core/lv_obj_class_private.h"


#if LV_USE_CURVE != 0
#include "../../misc/lv_assert.h"
#include "../../misc/lv_math.h"
#include "../../misc/lv_types.h"
#include "../../draw/lv_draw.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_curve_class)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_curve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_curve_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void lv_curve_size_event_cb(lv_event_t * e);
static void lv_curve_pos_to_area(lv_obj_t* obj, int32_t pos, lv_area_t* area);
static void lv_curve_area_to_pos(lv_obj_t* obj, const lv_area_t* area, int32_t* start, int32_t* end);
static void invalidate_point(lv_obj_t * obj, uint32_t i);
static void invalidate_points(lv_obj_t* obj);

/**********************
 *  STATIC VARIABLES
 **********************/

#if LV_USE_OBJ_PROPERTY
static const lv_property_ops_t lv_line_properties[] = {
    {
        .id = LV_PROPERTY_LINE_Y_INVERT,
        .setter = lv_line_set_y_invert,
        .getter = lv_line_get_y_invert,
    },
};
#endif

const lv_obj_class_t lv_curve_class = {
    .constructor_cb = lv_curve_constructor,
    .event_cb = lv_curve_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_curve_t),
    .base_class = &lv_obj_class,
    .name = "lv_curve",
    LV_PROPERTY_CLASS_FIELDS(curve, LINE)
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

/*=====================
 * Setter functions
 *====================*/

void lv_curve_set_decim(lv_obj_t* obj, uint16_t decim)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    if(decim == 0) return;
    
    lv_curve_t * curve = (lv_curve_t *)obj;
    if(curve->x_step == decim) return;
    
    curve->x_step = decim;
    
    /* Пересчитываем размер буфера под новую ширину */
    int32_t width = lv_obj_get_width(obj);
    uint32_t new_point_num = width / decim;
    
    if(new_point_num < 2) new_point_num = 2;
    
    /* Изменяем размер буфера */
    uint16_t * new_buffer = lv_realloc(curve->point_array, new_point_num * sizeof(uint16_t));
    if(new_buffer == NULL) return;
    
    curve->point_array = new_buffer;
    
    /* Если новый буфер больше, инициализируем новые элементы */
    if(new_point_num > curve->point_num) {
        for(uint32_t i = curve->point_num; i < new_point_num; i++) {
            curve->point_array[i] = 0;
        }
    }
    
    /* Сбрасываем позицию, если текущий индекс выходит за пределы */
    if(curve->current >= new_point_num) {
        curve->current = 0;
        curve->full = 0;
        curve->point_wait_current = 0;
    }
    
    curve->point_num = new_point_num;
    
    lv_obj_invalidate(obj);
}

void lv_curve_add_point(lv_obj_t* obj, uint16_t y_point)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    
    lv_curve_t * curve = (lv_curve_t *)obj;
    if(curve->point_array == NULL || curve->point_num == 0) return;

    curve->point_array[curve->current] = y_point;
    curve->point_wait_current++;

    curve->current++;
    if(curve->current >= curve->point_num - 1){
        curve->current = 0;
        curve->full = 1;
        invalidate_points(obj);
    }
    else if(curve->point_wait_current >= curve->point_wait_count){
        invalidate_points(obj);
    }

}

void lv_curve_clear(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    
    lv_curve_t * curve = (lv_curve_t *)obj;
    if(curve->point_array == NULL) return;
    
    /* Обнуляем буфер */
    lv_memset(curve->point_array,0, curve->point_num * sizeof(uint16_t));
    
    /* Сбрасываем указатель */
    curve->current = 0;
    curve->full = 0;
    curve->point_wait_current = 0;
    
    /* Полностью перерисовываем виджет */
    lv_obj_invalidate(obj);
}


/*=====================
 * Getter functions
 *====================*/

const uint16_t * lv_curve_get_points(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_curve_t * curve = (lv_curve_t *)obj;
    return curve->point_array;
}

uint32_t lv_curve_get_point_count(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_curve_t * curve = (lv_curve_t *)obj;
    if(curve->full == 0){
        return curve->current;
    }
    else{
        return curve->point_num;
    }
}

uint16_t lv_curve_get_decim(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_curve_t * curve = (lv_curve_t *)obj;
    return curve->x_step;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_curve_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_curve_t * curve = (lv_curve_t *)obj;

    curve->point_num   = 0;
    curve->point_array = NULL;
    curve->x_step      = 1;      /* По умолчанию 1 пиксель на точку */
    curve->current     = 0;
    curve->full        = 0;
    curve->point_wait_count = 5;
    curve->point_wait_current = 0;

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, lv_curve_size_event_cb, LV_EVENT_SIZE_CHANGED, NULL);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);

    LV_TRACE_OBJ_CREATE("finished");
}

static void invalidate_points(lv_obj_t* obj){
    lv_curve_t * curve  = (lv_curve_t *)obj;
    uint16_t s = lv_curve_get_point_count(obj);
    if(s <= 1){
        return;
    }
    int32_t x_ofs = obj->coords.x1;
    lv_area_t coords;
    lv_area_copy(&coords, &obj->coords);

    int32_t end;
    int32_t start;
    
    if(curve->current == 0)
        end = curve->point_num - 1;
    else
        end = curve->current - 1;
   
    start = end - curve->point_wait_current;

    if(start == 0){
        coords.x1 = 0;
    }
    else{
        coords.x1 = (start - 1) * curve->x_step;
    }

    if(end == s - 1){
        coords.x2 = end * curve->x_step + 1;
    }
    else{
        coords.x2 = (end + 1) * curve->x_step + 1;
    }

    coords.x1 += x_ofs;
    coords.x2 += x_ofs;
    curve->point_wait_current = 0;
    lv_obj_invalidate_area(obj, &coords);
}

static void lv_curve_size_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_curve_t * curve = (lv_curve_t *)obj;
    
    int32_t width = lv_obj_get_width(obj);
    if(width <= 0) return;
    
    /* Вычисляем размер буфера */
    uint32_t new_point_num = width / curve->x_step;
    if(new_point_num < 2) new_point_num = 2;
    
    /* Если буфер не создан или размер изменился */
    if(curve->point_array == NULL || new_point_num != curve->point_num) {
        uint16_t * new_buffer = lv_realloc(curve->point_array, new_point_num * sizeof(uint16_t));
        if(new_buffer == NULL) return;
        
        curve->point_array = new_buffer;
        
        /* Инициализируем нулями */
        if(curve->point_num == 0) {
            lv_memset(curve->point_array, 0, new_point_num * sizeof(uint16_t));
        } else if(new_point_num > curve->point_num) {
            /* Буфер увеличился — инициализируем новые элементы */
            for(uint32_t i = curve->point_num; i < new_point_num; i++) {
                curve->point_array[i] = 0;
            }
        }
        
        curve->point_num = new_point_num;
        
        /* Сбрасываем текущую позицию, если она выходит за пределы */
        if(curve->current >= curve->point_num) {
            curve->current = 0;
            curve->full = 0;
        }
    }
    
    lv_obj_invalidate(obj);
}

static void lv_curve_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);
    
    /* Вызываем обработчик базового класса */
    lv_result_t res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        /* Увеличиваем область рисования для линий с закруглениями */
        int32_t line_width = lv_obj_get_style_line_width(obj, LV_PART_MAIN);
        int32_t * s = lv_event_get_param(e);
        if(*s < line_width) *s = line_width;
    }
    else if(code == LV_EVENT_GET_SELF_SIZE) {
        lv_curve_t * curve = (lv_curve_t *)obj;
        
        if(curve->point_num == 0 || curve->point_array == NULL) return;
        
        lv_point_t * p = lv_event_get_param(e);
        
        /* Размер определяется максимальным X и максимальным Y */
        int32_t width = curve->point_num * curve->x_step;
        int32_t height = 0;
        
        for(uint32_t i = 0; i < curve->point_num; i++) {
            if(curve->point_array[i] > height) {
                height = curve->point_array[i];
            }
        }
        
        p->x = width;
        p->y = height;
    }
    else if(code == LV_EVENT_DRAW_MAIN) {
        
        lv_curve_t * curve = (lv_curve_t *)obj;
        lv_layer_t * layer = lv_event_get_layer(e);
        
        uint32_t size = lv_curve_get_point_count(obj);
        if(size <= 1)
            return;
        
        // Инициализируем стиль линии
        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.base.layer = layer;
        lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);
        int32_t height = lv_obj_get_height(obj);

        int32_t start = 0;
        int32_t end = 0;
        lv_curve_area_to_pos(obj, &layer->_clip_area, &start, &end);

        lv_point_precise_t * points = lv_malloc((end - start) * sizeof(lv_point_precise_t));
        line_dsc.points = points;
        line_dsc.point_cnt = 0;

        for(uint32_t i = start; i < end; i++) {
            int32_t x = i * curve->x_step;
            int32_t y = curve->point_array[i];
            if(y > height) y = height;
            y = height - y;
            points[line_dsc.point_cnt].x = obj->coords.x1 + x;
            points[line_dsc.point_cnt].y = obj->coords.y1 + y;

            line_dsc.point_cnt++;
        }
        lv_draw_line(layer, &line_dsc);
        lv_free(points);
        
    }
}

static void invalidate_point(lv_obj_t * obj, uint32_t i)
{
    lv_curve_t * curve  = (lv_curve_t *)obj;
    uint16_t s = lv_curve_get_point_count(obj);
    if(s <= 1){
        return;
    }

    int32_t x_ofs = obj->coords.x1;


    lv_area_t coords;
    lv_area_copy(&coords, &obj->coords);

   
    if(i >= s - 1){
        coords.x1 = (i - 1) * curve->x_step;
        coords.x2 = i * curve->x_step + 1;
    }
    else if(i == 0){
        coords.x1 = 0;
        coords.x2 = (i + 1) * curve->x_step + 1;
    }
    else{
        coords.x1 = (i - 1) * curve->x_step;
        coords.x2 = (i + 1) * curve->x_step + 1;
    }
    coords.x1 += x_ofs;
    coords.x2 += x_ofs;
    lv_obj_invalidate_area(obj, &coords);
}

static void lv_curve_pos_to_area(lv_obj_t* obj, int32_t pos, lv_area_t* area){
    lv_curve_t * curve = (lv_curve_t *)obj;
    uint32_t height = lv_obj_get_height(obj);
    uint16_t size = lv_curve_get_point_count(obj);

    area->y1 = 0;
    area->y2 = height;

    //Обработка трех краевых случаев
    // 1) Добавилась точка в начало буфера
    // 2) Добавилась точка в конец буфера
    // 3) Добавилась точка на любую другую позицию
    if(pos == size - 1){
        area->x1 = (pos - 1) * curve->x_step;
        area->x2 = pos * curve->x_step + 1;
    }
    else if(pos == 0){
        area->x1 = 0;
        area->x2 = curve->x_step + 1;
    }
    else{
        area->x1 = (pos - 1) * curve->x_step;
        area->x2 = curve->current * curve->x_step + 1;
    }

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);
    lv_area_move(area, obj_area.x1, obj_area.y1);
}

static void lv_curve_area_to_pos(lv_obj_t* obj, const lv_area_t* area, int32_t* start, int32_t* end){
    lv_curve_t * curve = (lv_curve_t *)obj;
    if(curve->x_step == 0)
        return;


    uint16_t size = lv_curve_get_point_count(obj);

    lv_area_t src_area;
    lv_area_copy(&src_area, area);

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);
    lv_area_move(&src_area, -obj_area.x1, -obj_area.y1);

    *start = src_area.x1 / curve->x_step;
    *end = src_area.x2 / curve->x_step;

    if(*end >= size)
        *end = size - 1;
}

#endif
