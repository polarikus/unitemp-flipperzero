#include "UnitempViews.h"
#include "unitemp_icons.h"
static View* view;

static const uint8_t temp_positions[3][2] = {{37, 24}, {37, 16}, {9, 16}};
static uint8_t sensor_index = 0;

static void _draw_noSensors(Canvas* canvas) {
    canvas_draw_str(canvas, 0, 24, "Sensors not found");
}

static void _draw_temp(Canvas* canvas, float temp, uint8_t pos) {
    //Рисование рамки
    canvas_draw_rframe(canvas, temp_positions[pos][0], temp_positions[pos][1], 54, 20, 3);
    canvas_draw_rframe(canvas, temp_positions[pos][0], temp_positions[pos][1], 54, 19, 3);
    int8_t temp_int = temp;
    int8_t temp_dec = abs((int16_t)(temp * 10) % 10);

    //Рисование иконки
    canvas_draw_icon(
        canvas, temp_positions[pos][0] + 3, temp_positions[pos][1] + 3, &I_temp_C_11x14);

    char buff[5];
    if((int8_t)temp == -128) {
        snprintf(buff, 5, "--");
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(
            canvas,
            temp_positions[pos][0] + 27,
            temp_positions[pos][1] + 10,
            AlignCenter,
            AlignCenter,
            buff);
        uint8_t int_len = canvas_string_width(canvas, buff);
        snprintf(buff, 4, ". -");
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas,
            temp_positions[pos][0] + 27 + int_len,
            temp_positions[pos][1] + 10 + 3,
            AlignRight,
            AlignCenter,
            buff);
        return;
    }
    //Целая часть температуры

    snprintf(buff, 5, "%d", temp_int);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas,
        temp_positions[pos][0] + 27 + ((temp_int <= -10) ? 5 : 0),
        temp_positions[pos][1] + 10,
        AlignCenter,
        AlignCenter,
        buff);
    //Печать дробной части температуры в диапазоне от -9 до 99 (когда два знака в числе)
    if(temp_int > -10 && temp_int <= 99) {
        uint8_t int_len = canvas_string_width(canvas, buff);
        snprintf(buff, 4, ".%d", temp_dec);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas,
            temp_positions[pos][0] + 27 + int_len + ((temp_int >= 0 && temp_int < 10) ? 5 : 0),
            temp_positions[pos][1] + 10 + 3,
            AlignRight,
            AlignCenter,
            buff);
    }
}

static void _draw_sensorsCarousel(Canvas* canvas) {
    //Рисование рамки
    canvas_draw_rframe(canvas, 3, 0, 122, 63, 7);
    canvas_draw_rframe(canvas, 3, 0, 122, 64, 7);

    //Печать имени
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 7, AlignCenter, AlignCenter, app->sensors[sensor_index]->name);
    //Подчёркивание
    uint8_t line_len = canvas_string_width(canvas, app->sensors[sensor_index]->name) + 2;
    canvas_draw_line(canvas, 64 - line_len / 2, 12, 64 + line_len / 2, 12);

    //Стрелка вправо
    if(unitemp_sensors_getTypesCount() > 0 && sensor_index < unitemp_sensors_getCount() - 1) {
        canvas_draw_icon(canvas, 64 + line_len / 2 + 4, 3, &I_arrow_right_5x9);
    }
    //Стрелка влево
    if(sensor_index > 0) {
        canvas_draw_icon(canvas, 64 - line_len / 2 - 8, 3, &I_arrow_left_5x9);
    }

    //Печать значения температуры
    _draw_temp(canvas, app->sensors[sensor_index]->temp, 0);
}

static void _draw_callback(Canvas* canvas, void* _model) {
    UNUSED(_model);

    app->sensors_ready = true;

    uint8_t sensors_count = unitemp_sensors_getCount();

    if(sensors_count == 0) {
        _draw_noSensors(canvas);
    }
    if(sensors_count > 0) {
        _draw_sensorsCarousel(canvas);
    }
}

static bool _input_callback(InputEvent* event, void* context) {
    Unitemp* app = context;

    //Выход по короткому нажатию "назад"
    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        app->processing = false;
    }
    //Пролистывание карусели по короткому нажатию "право"
    if(event->key == InputKeyRight && event->type == InputTypeShort) {
        if(++sensor_index >= unitemp_sensors_getCount()) sensor_index = 0;
    }
    //Пролистывание карусели по короткому нажатию "лево"
    if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        if(--sensor_index >= unitemp_sensors_getCount())
            sensor_index = unitemp_sensors_getCount() - 1;
    }
    //Вход в главное меню по короткому нажатию "Ок"
    if(event->key == InputKeyOk && event->type == InputTypeShort) {
        app->sensors_ready = false;
        unitemp_MainMenu_switch();
    }

    return true;
}

void unitemp_General_alloc(void) {
    view = view_alloc();
    view_set_context(view, app);
    view_set_draw_callback(view, _draw_callback);
    view_set_input_callback(view, _input_callback);

    view_dispatcher_add_view(app->view_dispatcher, GENERAL_VIEW, view);
}

void unitemp_General_switch(void) {
    app->sensors_ready = true;
    view_dispatcher_switch_to_view(app->view_dispatcher, GENERAL_VIEW);
}

void unitemp_General_free(void) {
    view_free(view);
}