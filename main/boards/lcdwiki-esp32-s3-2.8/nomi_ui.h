#ifndef NOMI_UI_H
#define NOMI_UI_H

#include <functional>

#include <lvgl.h>

#include "display/lcd_display.h"

// Actions the settings panel can trigger. Provided by the board so this
// module stays independent from board-specific classes.
struct NomiActions {
    std::function<int()> get_battery;        // 0..100, -1 when unknown
    std::function<int()> get_volume;         // 0..100
    std::function<void(int)> set_volume;     // 0..100
    std::function<void()> reconfigure_wifi;
    std::function<void()> reboot;
    std::function<void()> power_off;
};

// NOMI-style animated face + touch settings panel.
// All LVGL objects are created on the active screen / top layer; the face
// itself is an opaque rounded plate so it naturally covers the stock emoji.
class NomiUi {
public:
    NomiUi(lv_obj_t* parent, const NomiActions& actions);
    ~NomiUi();

    // Called from the display's SetEmotion(). Only stores an enum; the
    // animation timer applies it while holding the LVGL port lock.
    void SetEmotion(const char* emotion);

private:
    lv_obj_t* halo_ = nullptr;
    lv_obj_t* face_ = nullptr;
    lv_obj_t* eye_l_ = nullptr;
    lv_obj_t* eye_r_ = nullptr;
    lv_obj_t* pupil_l_ = nullptr;
    lv_obj_t* pupil_r_ = nullptr;
    lv_obj_t* mouth_ = nullptr;

    lv_obj_t* gear_ = nullptr;
    lv_obj_t* panel_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;
    lv_obj_t* volume_label_ = nullptr;

    NomiActions actions_;
    lv_timer_t* timer_ = nullptr;

    uint32_t tick_ = 0;
    int emotion_ = 0;        // 0 neutral / state-driven, 1 happy, 2 sad, 3 angry, 4 surprised, 5 thinking
    int emotion_ticks_ = 0;  // remaining ticks before returning to state-driven face

    void BuildFace(lv_obj_t* parent);
    void BuildSettings();
    void ApplyFrame();

    static void TimerCb(lv_timer_t* timer);
    static void GearCb(lv_event_t* e);
    static void CloseCb(lv_event_t* e);
    static void VolUpCb(lv_event_t* e);
    static void VolDownCb(lv_event_t* e);
    static void PowerCb(lv_event_t* e);
    static void RebootCb(lv_event_t* e);
    static void WifiCb(lv_event_t* e);
};

// Drop-in replacement for SpiLcdDisplay that installs the NOMI face.
// The stock emoji objects are hidden; SetEmotion() feeds the face instead.
class NomiLcdDisplay : public SpiLcdDisplay {
public:
    NomiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   const NomiActions& actions)
        : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y,
                        mirror_x, mirror_y, swap_xy),
          actions_(actions) {}

    virtual void SetupUI() override;
    virtual void SetEmotion(const char* emotion) override;

private:
    NomiActions actions_;
    NomiUi* ui_ = nullptr;
};

#endif  // NOMI_UI_H
