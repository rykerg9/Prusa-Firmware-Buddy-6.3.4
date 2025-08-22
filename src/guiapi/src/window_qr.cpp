#include "window_qr.hpp"

#include "display.hpp"
#include "qrcodegen.h"

void draw_qr(const DrawQROptions &options) {
    /// Defines maximal size of QR code and buffers needed for generating. Keep it low.
    static constexpr uint8_t qr_version_max = 9;
    static constexpr int border = 2; /// border size in pixels; same for all sides
    static constexpr int px_per_module = 2; /// width/height of module (single colored square)
    static constexpr bool scale = true; /// changes px_per_module so the QR code is the biggest that fits in the rect

    const Rect16::Width_t width = options.rect.Width();
    const Rect16::Height_t height = options.rect.Height();
    const Align_t align = options.align;

    /// Drawn QR code, 353 B on stack
    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(qr_version_max)];

    { // Create QR code
        /// Temporary buffer, 353 B using display buffer
        display::BorrowBuffer buffer;
        assert(display::buffer_pixel_size() >= qrcodegen_BUFFER_LEN_FOR_VERSION(qr_version_max));
        if (!qrcodegen_encodeText(options.data, buffer, qrcode, qrcodegen_Ecc_LOW, 1, qr_version_max, qrcodegen_Mask_AUTO, true)) {
            return;
        }
    }

    uint8_t ppm = px_per_module; /// pixels per module
    const uint16_t size = qrcodegen_getSize(qrcode);

    uint16_t x0 = 0;
    uint16_t y0 = 0;

    /// scale QR code
    if (scale) {
        const uint16_t size_w_bord = size + 2 * border;
        ppm = std::max(1, (int)floor(std::min(uint16_t(height), uint16_t(width)) / float(size_w_bord)));
    }
    const uint16_t px_size = ppm * (2 * border + qrcodegen_getSize(qrcode));

    /// alignment
    if (align.Horizontal() == Align_t::horizontal::center) {
        x0 = std::max(0, (width - px_size)) / 2;
    } else if (align.Horizontal() == Align_t::horizontal::right) {
        x0 = std::max(0, (width - px_size));
    }

    if (align.Vertical() == Align_t::vertical::center) {
        y0 = std::max(0, (height - px_size)) / 2;
    } else if (align.Vertical() == Align_t::vertical::bottom) {
        y0 = std::max(0, (height - px_size));
    }

    /// move to window location
    x0 += options.rect.Left() + border * ppm;
    y0 += options.rect.Top() + border * ppm;

    /// FIXME paint border at once (fill_between_rect) - it's faster
    /// paint QR code
    for (int y = -border; y < (size + border); ++y) {
        for (int x = -border; x < (size + border); ++x) {
            display::fill_rect(Rect16(x0 + x * ppm, y0 + y * ppm, ppm, ppm), ((qrcodegen_getModule(qrcode, x, y) ? COLOR_BLACK : COLOR_WHITE)));
        }
    }
}
