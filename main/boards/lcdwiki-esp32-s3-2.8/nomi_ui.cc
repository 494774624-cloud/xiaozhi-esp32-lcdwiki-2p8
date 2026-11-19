#include "nomi_ui.h"

#include "application.h"

#include <esp_log.h>
#include <esp_lvgl_port.h>

#include <freertos/FreeRTOS.h>

#include <cmath>
#include <cstdio>
#include <cstring>

#define TAG "NomiUi"

// Provided as a compile definition by main/CMakeLists.txt for this board.
LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

#define NOMI_GLOW lv_color_hex(0x5FE1FF)
#define NOMI_DARK lv_color_hex(0x0A1420)
#define NOMI_PANEL lv_color_hex(0x101A2A)
#define NOMI_BTN lv_color_hex(0x1E3A5F)

NomiUi::NomiUi(lv_obj_t* parent, const NomiActions& actions) : actions_(actions) {
    BuildFace(parent);
    BuildSettings();
    timer_ = lv_timer_create(TimerCb, 80, this);
    ESP_LOGI(TAG, "NOMI UI ready");
}

NomiUi::~NomiUi() {
    if (timer_) {
        lv_timer_delete(timer_);
        timer_ = nullptr;
    }
    // panel_ / gear_ live on lv_layer_top(); face lives on the screen.
    // They are deleted together with their parents at shutdown.
}

void NomiUi::BuildFace(lv_obj_t* parent) {
    // Outer glow ring
    halo_ = lv_obj_create(parent);
    lv_obj_remove_style_all(halo_);
    lv_obj_set_size(halo_, 196, 196);
    lv_obj_align(halo_, LV_ALIGN_CENTER, 0, -18);
    lv_obj_set_style_radius(halo_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(halo_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(halo_, 2, 0);
    lv_obj_set_style_border_color(halo_, NOMI_GLOW, 0);
    lv_obj_set_style_border_opa(halo_, LV_OPA_40, 0);
    lv_obj_remove_flag(halo_, LV_OBJ_FLAG_SCROLLABLE);

    // Opaque face plate - covers the stock emoji behind it
    face_ = lv_obj_create(parent);
    lv_obj_remove_style_all(face_);
    lv_obj_set_size(face_, 168, 168);
    lv_obj_align(face_, LV_ALIGN_CENTER, 0, -18);
    lv_obj_set_style_radius(face_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face_, NOMI_DARK, 0);
    lv_obj_set_style_bg_opa(face_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(face_, 3, 0);
    lv_obj_set_style_border_color(face_, NOMI_GLOW, 0);
    lv_obj_remove_flag(face_, LV_OBJ_FLAG_SCROLLABLE);

    eye_l_ = lv_obj_create(face_);
    lv_obj_remove_style_all(eye_l_);
    lv_obj_set_size(eye_l_, 38, 38);
    lv_obj_align(eye_l_, LV_ALIGN_CENTER, -34, -20);
    lv_obj_set_style_radius(eye_l_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_l_, NOMI_GLOW, 0);
    lv_obj_set_style_bg_opa(eye_l_, LV_OPA_COVER, 0);

    eye_r_ = lv_obj_create(face_);
    lv_obj_remove_style_all(eye_r_);
    lv_obj_set_size(eye_r_, 38, 38);
    lv_obj_align(eye_r_, LV_ALIGN_CENTER, 34, -20);
    lv_obj_set_style_radius(eye_r_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_r_, NOMI_GLOW, 0);
    lv_obj_set_style_bg_opa(eye_r_, LV_OPA_COVER, 0);

    pupil_l_ = lv_obj_create(eye_l_);
    lv_obj_remove_style_all(pupil_l_);
    lv_obj_set_size(pupil_l_, 14, 14);
    lv_obj_center(pupil_l_);
    lv_obj_set_style_radius(pupil_l_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_l_, NOMI_DARK, 0);
    lv_obj_set_style_bg_opa(pupil_l_, LV_OPA_COVER, 0);

    pupil_r_ = lv_obj_create(eye_r_);
    lv_obj_remove_style_all(pupil_r_);
    lv_obj_set_size(pupil_r_, 14, 14);
    lv_obj_center(pupil_r_);
    lv_obj_set_style_radius(pupil_r_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_r_, NOMI_DARK, 0);
    lv_obj_set_style_bg_opa(pupil_r_, LV_OPA_COVER, 0);

    mouth_ = lv_obj_create(face_);
    lv_obj_remove_style_all(mouth_);
    lv_obj_set_size(mouth_, 36, 8);
    lv_obj_align(mouth_, LV_ALIGN_CENTER, 0, 38);
    lv_obj_set_style_radius(mouth_, 6, 0);
    lv_obj_set_style_bg_color(mouth_, NOMI_GLOW, 0);
    lv_obj_set_style_bg_opa(mouth_, LV_OPA_COVER, 0);
}

static lv_obj_t* NomiButton(lv_obj_t* parent, const char* text, lv_event_cb_t cb, void* user_data) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_width(btn, 180);
    lv_obj_set_height(btn, 32);
    lv_obj_set_style_bg_color(btn, NOMI_BTN, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, NOMI_GLOW, 0);
    lv_obj_set_style_text_font(btn, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    return btn;
}

void NomiUi::BuildSettings() {
    // Small entry button at bottom-right of the top layer.
    gear_ = lv_btn_create(lv_layer_top());
    lv_obj_remove_style_all(gear_);
    lv_obj_set_size(gear_, 52, 28);
    lv_obj_align(gear_, LV_ALIGN_BOTTOM_RIGHT, -6, -6);
    lv_obj_set_style_bg_color(gear_, NOMI_PANEL, 0);
    lv_obj_set_style_bg_opa(gear_, LV_OPA_80, 0);
    lv_obj_set_style_radius(gear_, 8, 0);
    lv_obj_set_style_border_width(gear_, 1, 0);
    lv_obj_set_style_border_color(gear_, NOMI_GLOW, 0);
    lv_obj_set_style_text_font(gear_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(gear_, lv_color_white(), 0);
    {
        lv_obj_t* label = lv_label_create(gear_);
        lv_label_set_text(label, "设置");
        lv_obj_center(label);
    }
    lv_obj_add_event_cb(gear_, GearCb, LV_EVENT_CLICKED, this);

    panel_ = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(panel_);
    lv_obj_set_size(panel_, 210, 286);
    lv_obj_center(panel_);
    lv_obj_set_style_bg_color(panel_, NOMI_PANEL, 0);
    lv_obj_set_style_bg_opa(panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel_, 14, 0);
    lv_obj_set_style_border_width(panel_, 2, 0);
    lv_obj_set_style_border_color(panel_, NOMI_GLOW, 0);
    lv_obj_set_style_pad_all(panel_, 12, 0);
    lv_obj_set_style_pad_row(panel_, 7, 0);
    lv_obj_set_style_text_font(panel_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(panel_, lv_color_white(), 0);
    lv_obj_set_layout(panel_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(panel_, LV_OBJ_FLAG_HIDDEN);

    {
        lv_obj_t* title = lv_label_create(panel_);
        lv_label_set_text(title, "NOMI 设置");
        lv_obj_set_style_text_color(title, NOMI_GLOW, 0);
    }

    battery_label_ = lv_label_create(panel_);
    lv_label_set_text(battery_label_, "电量 --");

    {
        lv_obj_t* row = lv_obj_create(panel_);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, 180, 32);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        NomiButton(row, "-", VolDownCb, this);
        volume_label_ = lv_label_create(row);
        lv_label_set_text(volume_label_, "音量 --");
        NomiButton(row, "+", VolUpCb, this);

        // Override the fixed 180px width from NomiButton for the +/- buttons.
        lv_obj_t* first = lv_obj_get_child(row, 0);
        lv_obj_t* last = lv_obj_get_child(row, lv_obj_get_child_cnt(row) - 1);
        if (first) lv_obj_set_width(first, 44);
        if (last) lv_obj_set_width(last, 44);
    }

    NomiButton(panel_, "关机", PowerCb, this);
    NomiButton(panel_, "重启", RebootCb, this);
    NomiButton(panel_, "重新配网", WifiCb, this);
    NomiButton(panel_, "返回", CloseCb, this);
}

void NomiUi::SetEmotion(const char* emotion) {
    if (emotion == nullptr) return;
    if (strcmp(emotion, "neutral") == 0) {
        emotion_ = 0;
        emotion_ticks_ = 0;
        return;
    }
    if (strcmp(emotion, "happy") == 0 || strcmp(emotion, "laughing") == 0 ||
        strcmp(emotion, "funny") == 0 || strcmp(emotion, "loving") == 0) {
        emotion_ = 1;
    } else if (strcmp(emotion, "sad") == 0 || strcmp(emotion, "crying") == 0) {
        emotion_ = 2;
    } else if (strcmp(emotion, "angry") == 0) {
        emotion_ = 3;
    } else if (strcmp(emotion, "surprised") == 0) {
        emotion_ = 4;
    } else if (strcmp(emotion, "thinking") == 0 || strcmp(emotion, "confused") == 0) {
        emotion_ = 5;
    } else {
        emotion_ = 0;
    }
    emotion_ticks_ = 50;  // roughly 4 seconds
}

void NomiUi::ApplyFrame() {
    tick_++;
    DeviceState state = Application::GetInstance().GetDeviceState();
    int mood = emotion_;
    if (emotion_ticks_ > 0) {
        emotion_ticks_--;
        if (emotion_ticks_ == 0) emotion_ = 0;
    }

    if (!lvgl_port_lock(portMAX_DELAY)) return;

    int eye_w = 38, eye_h = 38;
    int px = 0, py = 0;
    int mouth_w = 36, mouth_h = 8;
    lv_color_t glow = NOMI_GLOW;
    bool blink = false;

    if (mood == 0) {
        switch (state) {
            case kDeviceStateListening:
                eye_w = 42; eye_h = 42; mouth_w = 16; mouth_h = 18;
                break;
            case kDeviceStateSpeaking:
                mouth_w = 28; mouth_h = 8 + (tick_ % 3) * 6;
                break;
            case kDeviceStateConnecting:
            case kDeviceStateWifiConfiguring:
            case kDeviceStateActivating:
                px = (int)(11.0f * sinf(tick_ * 0.35f));
                py = (int)(11.0f * cosf(tick_ * 0.35f));
                mouth_w = 24; mouth_h = 5;
                break;
            default:
                // Idle: glance left / right / centre, and blink now and then.
                {
                    int phase = (tick_ / 32) % 4;
                    px = (phase == 0) ? -7 : (phase == 2) ? 7 : 0;
                    blink = (tick_ % 60) < 3;
                }
                break;
        }
    } else if (mood == 1) {       // happy
        eye_h = 12; py = 4; mouth_w = 52; mouth_h = 10;
    } else if (mood == 2) {       // sad
        eye_h = 26; py = 5; mouth_w = 26; mouth_h = 6;
    } else if (mood == 3) {       // angry
        eye_h = 16; mouth_w = 34; mouth_h = 5;
    } else if (mood == 4) {       // surprised
        eye_w = 46; eye_h = 46; mouth_w = 16; mouth_h = 20;
    } else if (mood == 5) {       // thinking
        px = -8; py = -6; mouth_w = 14; mouth_h = 6;
    }

    if (blink) eye_h = 4;

    lv_obj_set_style_border_color(halo_, glow, 0);
    lv_obj_set_style_border_color(face_, glow, 0);
    lv_obj_set_style_bg_color(eye_l_, glow, 0);
    lv_obj_set_style_bg_color(eye_r_, glow, 0);
    lv_obj_set_style_bg_color(mouth_, glow, 0);

    lv_obj_set_size(eye_l_, eye_w, eye_h);
    lv_obj_set_size(eye_r_, eye_w, eye_h);
    lv_obj_align(eye_l_, LV_ALIGN_CENTER, -34, -20);
    lv_obj_align(eye_r_, LV_ALIGN_CENTER, 34, -20);

    lv_obj_set_size(pupil_l_, 14, blink ? 4 : 14);
    lv_obj_set_size(pupil_r_, 14, blink ? 4 : 14);
    lv_obj_align(pupil_l_, LV_ALIGN_CENTER, px, py);
    lv_obj_align(pupil_r_, LV_ALIGN_CENTER, px, py);

    lv_obj_set_size(mouth_, mouth_w, mouth_h);
    lv_obj_align(mouth_, LV_ALIGN_CENTER, 0, 38);

    // Refresh the two readouts only while the panel is open.
    if (panel_ != nullptr && !lv_obj_has_flag(panel_, LV_OBJ_FLAG_HIDDEN) && (tick_ % 12 == 0)) {
        char buf[48];
        int level = actions_.get_battery ? actions_.get_battery() : -1;
        if (level >= 0) {
            snprintf(buf, sizeof(buf), "电量 %d%%", level);
        } else {
            snprintf(buf, sizeof(buf), "电量 --");
        }
        lv_label_set_text(battery_label_, buf);

        int volume = actions_.get_volume ? actions_.get_volume() : -1;
        if (volume >= 0) {
            snprintf(buf, sizeof(buf), "音量 %d", volume);
        } else {
            snprintf(buf, sizeof(buf), "音量 --");
        }
        lv_label_set_text(volume_label_, buf);
    }

    lvgl_port_unlock();
}

void NomiUi::TimerCb(lv_timer_t* timer) {
    NomiUi* self = static_cast<NomiUi*>(lv_timer_get_user_data(timer));
    if (self != nullptr) self->ApplyFrame();
}

void NomiUi::GearCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self == nullptr || self->panel_ == nullptr) return;
    if (lv_obj_has_flag(self->panel_, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(self->panel_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(self->panel_);
    } else {
        lv_obj_add_flag(self->panel_, LV_OBJ_FLAG_HIDDEN);
    }
}

void NomiUi::CloseCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self != nullptr && self->panel_ != nullptr) {
        lv_obj_add_flag(self->panel_, LV_OBJ_FLAG_HIDDEN);
    }
}

void NomiUi::VolUpCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self == nullptr || !self->actions_.get_volume || !self->actions_.set_volume) return;
    int next = self->actions_.get_volume() + 10;
    self->actions_.set_volume(next > 100 ? 100 : next);
}

void NomiUi::VolDownCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self == nullptr || !self->actions_.get_volume || !self->actions_.set_volume) return;
    int next = self->actions_.get_volume() - 10;
    self->actions_.set_volume(next < 0 ? 0 : next);
}

void NomiUi::PowerCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self != nullptr && self->actions_.power_off) self->actions_.power_off();
}

void NomiUi::RebootCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self != nullptr && self->actions_.reboot) self->actions_.reboot();
}

void NomiUi::WifiCb(lv_event_t* e) {
    NomiUi* self = static_cast<NomiUi*>(lv_event_get_user_data(e));
    if (self != nullptr && self->actions_.reconfigure_wifi) self->actions_.reconfigure_wifi();
}

void NomiLcdDisplay::SetupUI() {
    SpiLcdDisplay::SetupUI();
    if (ui_ != nullptr) return;  // base guards duplicate calls, so do we

    DisplayLockGuard lock(this);
    if (emoji_box_ != nullptr) lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
    if (emoji_label_ != nullptr) lv_obj_add_flag(emoji_label_, LV_OBJ_FLAG_HIDDEN);
    if (emoji_image_ != nullptr) lv_obj_add_flag(emoji_image_, LV_OBJ_FLAG_HIDDEN);

    ui_ = new NomiUi(lv_screen_active(), actions_);
}

void NomiLcdDisplay::SetEmotion(const char* emotion) {
    if (ui_ != nullptr) ui_->SetEmotion(emotion);
}
