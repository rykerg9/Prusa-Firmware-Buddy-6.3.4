// screen_reset_error.cpp

#include "screen_reset_error.hpp"
#include "config.h"
#include "ScreenHandler.hpp"
#include "sound.hpp"
#include <version/version.hpp>
#include "support_utils.h"
#include <str_utils.hpp>

ScreenResetError::ScreenResetError(const Rect16 &fw_version_rect)
    : screen_t()
    , sound_started(false)
    , fw_version_txt(this, fw_version_rect, is_multiline::no) {

    ClrMenuTimeoutClose();
    ClrOnSerialClose();
    start_sound();

    fw_version_txt.set_font(Font::small);

    fw_version_txt.SetAlignment(GuiDefaults::EnableDialogBigLayout ? Align_t::LeftTop() : Align_t::CenterTop());

    /// (fw version full string) [fw signed][appendix]
    const char *signed_str = signature_exist() ? " [S]" : "";
    const char *apendix_str = appendix_exist() ? " [A]" : "";
    StringBuilder(fw_version_str).append_printf("%s %s%s%s", PrinterModelInfo::current().id_str, version::project_version_full, signed_str, apendix_str);
    fw_version_txt.SetText(string_view_utf8::MakeRAM(fw_version_str.data()));
}

void ScreenResetError::start_sound() {
    if (!sound_started) {
        /// avoid collision of sounds
        Sound_Stop();
        Sound_Play(eSOUND_TYPE::CriticalAlert);
        sound_started = true;
    }
}

void ScreenResetError::windowEvent([[maybe_unused]] window_t *sender, GUI_event_t event, [[maybe_unused]] void *param) {
    switch (event) {
    case GUI_event_t::ENC_UP:
    case GUI_event_t::ENC_DN:
    case GUI_event_t::CLICK:
    case GUI_event_t::HOLD:
        Sound_Stop();
    default:
        break;
    }
}
