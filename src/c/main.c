#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "fctx-layer.h"
#include "fctx-text-layer.h"
#include "logging.h"

static Window *s_window;
static FctxLayer *s_root_layer;
static FctxLayer *s_grid_layer;
static FctxTextLayer *s_time_layer;

static EventHandle s_tick_timer_event_handle;

static void prv_fctx_draw_rect(FContext *fctx, GRect rect) {
    logf();
    fctx_begin_fill(fctx);
    fctx_move_to(fctx, FPoint(rect.origin.x, rect.origin.y));
    fctx_line_to(fctx, FPoint(rect.origin.x + rect.size.w, rect.origin.y));
    fctx_line_to(fctx, FPoint(rect.origin.x + rect.size.w, rect.origin.y + rect.size.h));
    fctx_line_to(fctx, FPoint(rect.origin.x, rect.origin.y + rect.size.h));
    fctx_close_path(fctx);
    fctx_end_fill(fctx);
}

static void prv_grid_layer_update_proc(FctxLayer *this, FContext *fctx) {
    logf();
    GRect line_rect = GRect(0, 0, 1, 1);
    fctx_set_scale(fctx, FPointOne, FPointI(PBL_DISPLAY_WIDTH, 2));

    fctx_set_offset(fctx, FPointI(0, 72));
    prv_fctx_draw_rect(fctx, line_rect);

    fctx_set_offset(fctx, FPointI(0, 72 + 50 + 2));
    prv_fctx_draw_rect(fctx, line_rect);

    fctx_set_offset(fctx, FPointI(0, 72 + 50 + 20 + 4));
    prv_fctx_draw_rect(fctx, line_rect);

    fctx_set_scale(fctx, FPointOne, FPointI(2, 42));
    fctx_set_offset(fctx, FPointI(PBL_DISPLAY_WIDTH / 2 - 1, PBL_DISPLAY_HEIGHT - 42));
    prv_fctx_draw_rect(fctx, line_rect);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    logf();
    static char s[8];
    strftime(s, sizeof(s), clock_is_24h_style() ? "%k:%M" : "%l:%M", tick_time);
    fctx_text_layer_set_text(s_time_layer, s);
}

static void prv_window_load(Window *window) {
    logf();
    s_root_layer = window_get_root_fctx_layer(window);

    s_grid_layer = fctx_layer_create(GPointZero);
    fctx_layer_set_update_proc(s_grid_layer, prv_grid_layer_update_proc);
    fctx_layer_add_child(s_root_layer, s_grid_layer);

    s_time_layer = fctx_text_layer_create(GPoint(PBL_DISPLAY_WIDTH / 2, 0));
    fctx_text_layer_set_font(s_time_layer, RESOURCE_ID_ROBOTO_REGULAR_FFONT);
    fctx_text_layer_set_alignment(s_time_layer, GTextAlignmentCenter);
    fctx_text_layer_set_anchor(s_time_layer, FTextAnchorTop);
    fctx_text_layer_set_color(s_time_layer, GColorWhite);
    fctx_text_layer_set_text_size(s_time_layer, 50);
    fctx_layer_add_child(s_root_layer, fctx_text_layer_get_fctx_layer(s_time_layer));

    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), MINUTE_UNIT);
    s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);

    window_set_background_color(window, GColorBlack);
}

static void prv_window_unload(Window *window) {
    logf();
    events_tick_timer_service_unsubscribe(s_tick_timer_event_handle);

    fctx_text_layer_destroy(s_time_layer);
    fctx_layer_destroy(s_grid_layer);
    fctx_layer_destroy(s_root_layer);
}

static void prv_init(void) {
    logf();
    setlocale(LC_ALL, "");

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = prv_window_load,
        .unload = prv_window_unload
    });
    window_stack_push(s_window, true);
}

static void prv_deinit(void) {
    logf();
    window_destroy(s_window);
}

int main(void) {
    logf();
    prv_init();
    app_event_loop();
    prv_deinit();
}
