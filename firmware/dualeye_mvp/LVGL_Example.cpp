#include "LVGL_Example.h"

namespace {
constexpr int16_t CENTER = EXAMPLE_LCD_WIDTH / 2;
constexpr int16_t IRIS_SIZE = 118;
constexpr int16_t PUPIL_SIZE = 54;
constexpr uint32_t FRAME_MS = 40;

struct EyeView {
  lv_obj_t *iris;
  lv_obj_t *pupil;
  lv_obj_t *glint;
  lv_obj_t *top_lid;
  lv_obj_t *bottom_lid;
};

EyeView left_eye{};
EyeView right_eye{};
lv_timer_t *animation_timer = nullptr;
uint32_t frame_count = 0;
uint32_t next_blink_frame = 105;
int16_t gaze_x = 0;
int16_t gaze_y = 0;
uint32_t notice_until_frame = 0;

lv_obj_t *circle(lv_obj_t *parent, int16_t size, lv_color_t color) {
  lv_obj_t *obj = lv_obj_create(parent);
  lv_obj_remove_style_all(obj);
  lv_obj_set_size(obj, size, size);
  lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(obj, color, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  return obj;
}

EyeView create_eye(lv_disp_t *display) {
  lv_obj_t *screen = lv_disp_get_scr_act(display);
  lv_obj_clean(screen);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0xF4E8D0), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  EyeView eye{};
  eye.iris = circle(screen, IRIS_SIZE, lv_color_hex(0x6B3F22));
  eye.pupil = circle(screen, PUPIL_SIZE, lv_color_hex(0x090706));
  eye.glint = circle(screen, 15, lv_color_white());

  eye.top_lid = lv_obj_create(screen);
  lv_obj_remove_style_all(eye.top_lid);
  lv_obj_set_size(eye.top_lid, EXAMPLE_LCD_WIDTH, EXAMPLE_LCD_HEIGHT / 2);
  lv_obj_set_style_bg_color(eye.top_lid, lv_color_hex(0x241A17), 0);
  lv_obj_set_style_bg_opa(eye.top_lid, LV_OPA_COVER, 0);

  eye.bottom_lid = lv_obj_create(screen);
  lv_obj_remove_style_all(eye.bottom_lid);
  lv_obj_set_size(eye.bottom_lid, EXAMPLE_LCD_WIDTH, EXAMPLE_LCD_HEIGHT / 2);
  lv_obj_set_style_bg_color(eye.bottom_lid, lv_color_hex(0x241A17), 0);
  lv_obj_set_style_bg_opa(eye.bottom_lid, LV_OPA_COVER, 0);
  return eye;
}

void position_eye(EyeView &eye, int16_t x, int16_t y, uint8_t blink) {
  lv_obj_set_pos(eye.iris, CENTER - IRIS_SIZE / 2 + x, CENTER - IRIS_SIZE / 2 + y);
  lv_obj_set_pos(eye.pupil, CENTER - PUPIL_SIZE / 2 + x, CENTER - PUPIL_SIZE / 2 + y);
  lv_obj_set_pos(eye.glint, CENTER - 10 + x, CENTER - 18 + y);

  const int16_t travel = (EXAMPLE_LCD_HEIGHT / 2) * blink / 100;
  lv_obj_set_pos(eye.top_lid, 0, -EXAMPLE_LCD_HEIGHT / 2 + travel);
  lv_obj_set_pos(eye.bottom_lid, 0, EXAMPLE_LCD_HEIGHT - travel);
  lv_obj_move_foreground(eye.top_lid);
  lv_obj_move_foreground(eye.bottom_lid);
}

uint8_t blink_for_frame(uint32_t frame) {
  if (frame < next_blink_frame) return 0;
  const uint32_t phase = frame - next_blink_frame;
  if (phase < 3) return static_cast<uint8_t>((phase + 1) * 33);
  if (phase < 6) return static_cast<uint8_t>((6 - phase) * 33);
  next_blink_frame = frame + 105 + random(0, 45);
  return 0;
}

void animation_timer_cb(lv_timer_t *) {
  ++frame_count;
  if (frame_count < notice_until_frame) {
    gaze_x = 0;
    gaze_y = 0;
  } else if (frame_count % 40 == 0) {
    gaze_x = random(-26, 27);
    gaze_y = random(-17, 18);
  }
  const uint8_t blink = blink_for_frame(frame_count);
  position_eye(left_eye, gaze_x, gaze_y, blink);
  position_eye(right_eye, gaze_x, gaze_y, blink);
}
}  // namespace

void Lvgl_Example1(void) {
  if (animation_timer != nullptr) lv_timer_del(animation_timer);
  randomSeed(esp_random());
  left_eye = create_eye(disp);
  right_eye = create_eye(disp2);
  position_eye(left_eye, 0, 0, 0);
  position_eye(right_eye, 0, 0, 0);
  animation_timer = lv_timer_create(animation_timer_cb, FRAME_MS, nullptr);
}

void LVGL_Backlight_adjustment(uint8_t backlight) {
  Set_Backlight(backlight);
}

void Eye_Notice(void) {
  notice_until_frame = frame_count + 50;
  gaze_x = 0;
  gaze_y = 0;
}
