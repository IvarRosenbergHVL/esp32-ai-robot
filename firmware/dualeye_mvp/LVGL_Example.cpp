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
  lv_obj_t *marquee;
};

EyeView left_eye{};
EyeView right_eye{};
lv_timer_t *animation_timer = nullptr;
uint32_t frame_count = 0;
uint32_t next_blink_frame = 105;
int16_t gaze_x = 0;
int16_t gaze_y = 0;
uint32_t notice_until_frame = 0;
EyeEmotion current_emotion = EyeEmotion::Neutral;
uint32_t forced_blink_until_frame = 0;
uint8_t forced_blink_amount = 0;
uint32_t marquee_until_frame = 0;

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

  eye.marquee = lv_label_create(screen);
  lv_obj_set_width(eye.marquee, EXAMPLE_LCD_WIDTH - 28);
  lv_label_set_long_mode(eye.marquee, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(eye.marquee, "");
  lv_obj_set_style_text_color(eye.marquee, lv_color_white(), 0);
  lv_obj_set_style_bg_color(eye.marquee, lv_color_hex(0x241A17), 0);
  lv_obj_set_style_bg_opa(eye.marquee, LV_OPA_90, 0);
  lv_obj_set_style_pad_all(eye.marquee, 8, 0);
  lv_obj_align(eye.marquee, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(eye.marquee, LV_OBJ_FLAG_HIDDEN);
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
  if (current_emotion == EyeEmotion::Thinking && frame_count % 24 == 0) {
    gaze_x = -gaze_x;
    gaze_y = -10;
  }
  uint8_t blink = blink_for_frame(frame_count);
  if (frame_count < forced_blink_until_frame) blink = forced_blink_amount;
  if (current_emotion == EyeEmotion::Sleepy) blink = max(blink, static_cast<uint8_t>(55));
  if (current_emotion == EyeEmotion::Skeptical) blink = max(blink, static_cast<uint8_t>(25));
  position_eye(left_eye, gaze_x, gaze_y, blink);
  position_eye(right_eye, gaze_x, gaze_y, blink);
  const bool showMarquee = frame_count < marquee_until_frame;
  lv_obj_t *labels[] = {left_eye.marquee, right_eye.marquee};
  for (lv_obj_t *label : labels) {
    if (showMarquee) {
      lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
      lv_obj_move_foreground(label);
    } else {
      lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
  }
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

void Eye_ShowMarquee(const char *text, uint32_t durationMs) {
  if (!text || !left_eye.marquee || !right_eye.marquee) return;
  lv_label_set_text(left_eye.marquee, text);
  lv_label_set_text(right_eye.marquee, text);
  lv_obj_align(left_eye.marquee, LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(right_eye.marquee, LV_ALIGN_CENTER, 0, 0);
  marquee_until_frame = frame_count + max(static_cast<uint32_t>(1), durationMs / FRAME_MS);
}

void Eye_SetEmotion(EyeEmotion emotion) {
  current_emotion = emotion;
  notice_until_frame = frame_count + 25;
  switch (emotion) {
    case EyeEmotion::Attentive: gaze_x = 0; gaze_y = 0; break;
    case EyeEmotion::Happy: gaze_x = 0; gaze_y = -5; break;
    case EyeEmotion::Curious: gaze_x = 18; gaze_y = -8; break;
    case EyeEmotion::Thinking: gaze_x = -20; gaze_y = -12; break;
    case EyeEmotion::Surprised: gaze_x = 0; gaze_y = 0; forced_blink_amount = 0; break;
    case EyeEmotion::Skeptical: gaze_x = 18; gaze_y = 3; break;
    case EyeEmotion::Sleepy: gaze_x = 0; gaze_y = 12; break;
    case EyeEmotion::Error: gaze_x = 0; gaze_y = 15; break;
    default: gaze_x = gaze_y = 0; break;
  }
}

bool Eye_SetEmotion(const char *emotion) {
  if (!emotion) return false;
  if (!strcmp(emotion, "neutral")) Eye_SetEmotion(EyeEmotion::Neutral);
  else if (!strcmp(emotion, "attentive")) Eye_SetEmotion(EyeEmotion::Attentive);
  else if (!strcmp(emotion, "happy")) Eye_SetEmotion(EyeEmotion::Happy);
  else if (!strcmp(emotion, "curious")) Eye_SetEmotion(EyeEmotion::Curious);
  else if (!strcmp(emotion, "thinking")) Eye_SetEmotion(EyeEmotion::Thinking);
  else if (!strcmp(emotion, "surprised")) Eye_SetEmotion(EyeEmotion::Surprised);
  else if (!strcmp(emotion, "skeptical")) Eye_SetEmotion(EyeEmotion::Skeptical);
  else if (!strcmp(emotion, "sleepy")) Eye_SetEmotion(EyeEmotion::Sleepy);
  else if (!strcmp(emotion, "error")) Eye_SetEmotion(EyeEmotion::Error);
  else return false;
  return true;
}

bool Eye_PerformAction(const char *action) {
  if (!action) return false;
  if (!strcmp(action, "blink")) { forced_blink_amount = 100; forced_blink_until_frame = frame_count + 3; }
  else if (!strcmp(action, "double_blink")) { next_blink_frame = frame_count; }
  else if (!strcmp(action, "look_left")) { gaze_x = -28; gaze_y = 0; notice_until_frame = frame_count + 25; }
  else if (!strcmp(action, "look_right")) { gaze_x = 28; gaze_y = 0; notice_until_frame = frame_count + 25; }
  else if (!strcmp(action, "look_up")) { gaze_x = 0; gaze_y = -20; notice_until_frame = frame_count + 25; }
  else if (!strcmp(action, "look_down")) { gaze_x = 0; gaze_y = 20; notice_until_frame = frame_count + 25; }
  else if (!strcmp(action, "look_center") || !strcmp(action, "reset")) Eye_Notice();
  else if (!strcmp(action, "widen") || !strcmp(action, "startle")) Eye_SetEmotion(EyeEmotion::Surprised);
  else if (!strcmp(action, "squint")) Eye_SetEmotion(EyeEmotion::Skeptical);
  else if (!strcmp(action, "wink_left") || !strcmp(action, "wink_right")) {
    // The first MVP renders synchronized lids; keep wink requests as a short blink.
    forced_blink_amount = 100; forced_blink_until_frame = frame_count + 3;
  } else return false;
  return true;
}
