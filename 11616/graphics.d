module common.graphics;

import os.syscall;
import common.utils;

extern(C) @nogc nothrow:

// Enhanced 2D graphics library for linuxab framebuffer
// Includes: SearchBar, InputField, Tabs, improved widgets

struct Color { ubyte r, g, b, a; }
struct Point { int x, y; }
struct Rect { int x, y, w, h; }

enum Colors {
    black       = Color(0, 0, 0, 255),
    white       = Color(255, 255, 255, 255),
    red         = Color(255, 0, 0, 255),
    green       = Color(0, 255, 0, 255),
    blue        = Color(0, 120, 255, 255),
    yellow      = Color(255, 255, 0, 255),
    cyan        = Color(0, 255, 255, 255),
    magenta     = Color(255, 0, 255, 255),
    gray        = Color(128, 128, 128, 255),
    dark        = Color(32, 32, 32, 255),
    darker      = Color(18, 18, 18, 255),
    orange      = Color(255, 165, 0, 255),
    purple      = Color(128, 0, 128, 255),
    lightgray   = Color(200, 200, 200, 255),
    panel       = Color(45, 45, 55, 255),
    highlight   = Color(60, 60, 80, 255),
    accent      = Color(0, 180, 255, 255),
    success     = Color(0, 200, 100, 255),
    danger      = Color(220, 50, 50, 255),
    warning     = Color(255, 180, 0, 255),
}

__gshared FBInfo g_fb;
__gshared bool g_fb_ready = false;

bool gfx_init() {
    if (g_fb_ready) return true;
    if (!fb_get_info(&g_fb)) return false;
    g_fb_ready = true;
    return true;
}

uint color_to_pixel(Color c) {
    return (cast(uint)c.a << 24) | (cast(uint)c.r << 16) | 
           (cast(uint)c.g << 8) | cast(uint)c.b;
}

void gfx_clear(Color c) {
    if (!g_fb_ready) return;
    uint pixel = color_to_pixel(c);
    uint* buf = g_fb.buffer;
    uint count = g_fb.width * g_fb.height;
    for (uint i = 0; i < count; i++) buf[i] = pixel;
}

void gfx_pixel(int x, int y, Color c) {
    if (!g_fb_ready || x < 0 || y < 0 || x >= g_fb.width || y >= g_fb.height) return;
    g_fb.buffer[y * g_fb.width + x] = color_to_pixel(c);
}

void gfx_rect(int x, int y, int w, int h, Color c) {
    if (!g_fb_ready) return;
    int x2 = x + w; int y2 = y + h;
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x2 > g_fb.width) x2 = g_fb.width;
    if (y2 > g_fb.height) y2 = g_fb.height;
    uint pixel = color_to_pixel(c);
    for (int row = y; row < y2; row++) {
        uint* line = &g_fb.buffer[row * g_fb.width + x];
        for (int col = x; col < x2; col++) line[col - x] = pixel;
    }
}

void gfx_rect_outline(int x, int y, int w, int h, Color c) {
    gfx_rect(x, y, w, 2, c);
    gfx_rect(x, y + h - 2, w, 2, c);
    gfx_rect(x, y, 2, h, c);
    gfx_rect(x + w - 2, y, 2, h, c);
}

void gfx_rect_rounded(int x, int y, int w, int h, int r, Color c) {
    gfx_rect(x + r, y, w - r*2, h, c);
    gfx_rect(x, y + r, w, h - r*2, c);
    gfx_circle(x + r, y + r, r, c);
    gfx_circle(x + w - r - 1, y + r, r, c);
    gfx_circle(x + r, y + h - r - 1, r, c);
    gfx_circle(x + w - r - 1, y + h - r - 1, r, c);
}

void gfx_line(int x0, int y0, int x1, int y1, Color c) {
    int dx = abs(x1 - x0); int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0); int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        gfx_pixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void gfx_circle(int cx, int cy, int r, Color c) {
    int x = r, y = 0, err = 0;
    while (x >= y) {
        gfx_pixel(cx + x, cy + y, c); gfx_pixel(cx + y, cy + x, c);
        gfx_pixel(cx - y, cy + x, c); gfx_pixel(cx - x, cy + y, c);
        gfx_pixel(cx - x, cy - y, c); gfx_pixel(cx - y, cy - x, c);
        gfx_pixel(cx + y, cy - x, c); gfx_pixel(cx + x, cy - y, c);
        if (err <= 0) { y += 1; err += 2*y + 1; }
        if (err > 0) { x -= 1; err -= 2*x + 1; }
    }
}

void gfx_gradient(int x, int y, int w, int h, Color top, Color bot) {
    for (int row = 0; row < h; row++) {
        float t = cast(float)row / h;
        Color c = Color(
            cast(ubyte)(top.r + (bot.r - top.r) * t),
            cast(ubyte)(top.g + (bot.g - top.g) * t),
            cast(ubyte)(top.b + (bot.b - top.b) * t),
            255
        );
        gfx_rect(x, y + row, w, 1, c);
    }
}

// 8x8 font
static immutable ubyte[95][8] FONT_8X8 = [
    [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],[0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00],
    [0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00],[0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00],
    [0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00],[0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00],
    [0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00],[0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00],
    [0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00],[0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00],
    [0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00],[0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00],
    [0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06],[0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00],
    [0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00],[0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00],
    [0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00],[0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00],
    [0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00],[0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00],
    [0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00],[0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00],
    [0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00],[0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00],
    [0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00],[0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00],
    [0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00],[0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06],
    [0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00],[0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00],
    [0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00],[0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00],
    [0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00],[0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00],
    [0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00],[0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00],
    [0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00],[0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00],
    [0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00],[0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00],
    [0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00],[0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00],
    [0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00],[0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00],
    [0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00],[0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00],
    [0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00],[0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00],
    [0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00],[0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00],
    [0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00],[0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00],
    [0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00],[0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00],
    [0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00],[0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00],
    [0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00],[0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00],
    [0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00],[0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00],
    [0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00],[0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00],
    [0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00],[0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF],
    [0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00],[0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00],
    [0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00],[0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00],
    [0x38,0x30,0x30,0x3e,0x33,0x33,0x6E,0x00],[0x00,0x00,0x1E,0x33,0x3f,0x03,0x1E,0x00],
    [0x1C,0x36,0x06,0x0f,0x06,0x06,0x0F,0x00],[0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F],
    [0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00],[0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00],
    [0x30,0x00,0x30,0x30,0x30,0x33,0x1E,0x00],[0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00],
    [0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00],[0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00],
    [0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00],[0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00],
    [0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F],[0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78],
    [0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00],[0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00],
    [0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00],[0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00],
    [0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00],[0x00,0x00,0x63,0x6B,0x7F,0x36,0x36,0x00],
    [0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00],[0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F],
    [0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00],[0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00],
    [0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00],[0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00],
    [0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00]
];

void gfx_text(int x, int y, const(char)* str, Color c) {
    if (!g_fb_ready) return;
    int cx = x; int cy = y;
    uint pixel = color_to_pixel(c);
    while (*str) {
        char ch = *str++;
        if (ch == '\\n') { cx = x; cy += 10; continue; }
        if (ch < 32 || ch > 126) ch = '?';
        int idx = ch - 32;
        for (int row = 0; row < 8; row++) {
            ubyte bits = FONT_8X8[idx][row];
            for (int col = 0; col < 8; col++) {
                if (bits & (1 << (7 - col))) {
                    int px = cx + col; int py = cy + row;
                    if (px >= 0 && py >= 0 && px < g_fb.width && py < g_fb.height)
                        g_fb.buffer[py * g_fb.width + px] = pixel;
                }
            }
        }
        cx += 8;
    }
}

void gfx_text_centered(int x, int y, int w, const(char)* str, Color c) {
    int tw = str_len(str) * 8;
    gfx_text(x + (w - tw) / 2, y, str, c);
}

// ===================== WIDGETS =====================

struct Button {
    Rect bounds;
    const(char)* label;
    Color bg, fg, border;
    bool hovered, pressed;
    bool visible;
}

bool button_update(Button* btn, int mx, int my, bool mdown) {
    if (!btn.visible) return false;
    btn.hovered = (mx >= btn.bounds.x && mx < btn.bounds.x + btn.bounds.w &&
                   my >= btn.bounds.y && my < btn.bounds.y + btn.bounds.h);
    btn.pressed = btn.hovered && mdown;
    return btn.hovered && !mdown;
}

void button_draw(Button* btn) {
    if (!btn.visible) return;
    Color bg = btn.pressed ? Colors.darker : (btn.hovered ? Colors.highlight : btn.bg);
    gfx_rect_rounded(btn.bounds.x, btn.bounds.y, btn.bounds.w, btn.bounds.h, 4, bg);
    gfx_rect_outline(btn.bounds.x, btn.bounds.y, btn.bounds.w, btn.bounds.h, btn.border);
    int tw = str_len(btn.label) * 8;
    int tx = btn.bounds.x + (btn.bounds.w - tw) / 2;
    int ty = btn.bounds.y + (btn.bounds.h - 8) / 2;
    gfx_text(tx, ty, btn.label, btn.fg);
}

struct InputField {
    Rect bounds;
    char[256] text;
    int len;
    bool active;
    bool changed;
    const(char)* placeholder;
}

void inputfield_draw(InputField* fld) {
    Color bg = fld.active ? Colors.panel : Colors.darker;
    gfx_rect_rounded(fld.bounds.x, fld.bounds.y, fld.bounds.w, fld.bounds.h, 4, bg);
    Color border = fld.active ? Colors.accent : Colors.gray;
    gfx_rect_outline(fld.bounds.x, fld.bounds.y, fld.bounds.w, fld.bounds.h, border);
    
    int tx = fld.bounds.x + 10;
    int ty = fld.bounds.y + (fld.bounds.h - 8) / 2;
    if (fld.len == 0 && !fld.active) {
        gfx_text(tx, ty, fld.placeholder, Colors.gray);
    } else {
        gfx_text(tx, ty, fld.text.ptr, Colors.white);
        if (fld.active) {
            int cx = tx + fld.len * 8;
            gfx_rect(cx, ty - 2, 8, 12, Colors.accent);
        }
    }
}

bool inputfield_click(InputField* fld, int mx, int my, bool mdown) {
    bool inside = (mx >= fld.bounds.x && mx < fld.bounds.x + fld.bounds.w &&
                   my >= fld.bounds.y && my < fld.bounds.y + fld.bounds.h);
    if (mdown && inside) { fld.active = true; return true; }
    if (mdown && !inside) { fld.active = false; }
    return false;
}

void inputfield_type(InputField* fld, char ch) {
    if (!fld.active) return;
    if (ch == 8 || ch == 127) { // backspace
        if (fld.len > 0) fld.len--;
        fld.text[fld.len] = 0;
        fld.changed = true;
    } else if (ch >= 32 && ch < 127 && fld.len < 255) {
        fld.text[fld.len] = ch;
        fld.len++;
        fld.text[fld.len] = 0;
        fld.changed = true;
    }
}

void inputfield_clear(InputField* fld) {
    fld.len = 0;
    fld.text[0] = 0;
    fld.changed = true;
}

struct ScrollList {
    Rect bounds;
    int item_h;
    int scroll_y;
    int item_count;
    int selected;
    bool visible;
}

void scrolllist_draw(ScrollList* list, const(char)** items, const(char)** subtitles, Color bg, Color sel, Color fg, Color subfg) {
    if (!list.visible) return;
    gfx_rect(list.bounds.x, list.bounds.y, list.bounds.w, list.bounds.h, bg);
    gfx_rect_outline(list.bounds.x, list.bounds.y, list.bounds.w, list.bounds.h, Colors.gray);
    
    int visible = list.bounds.h / list.item_h;
    int start = list.scroll_y / list.item_h;
    if (start < 0) start = 0;
    for (int i = start; i < list.item_count && i < start + visible + 1; i++) {
        int iy = list.bounds.y + (i * list.item_h) - list.scroll_y;
        if (iy + list.item_h < list.bounds.y || iy > list.bounds.y + list.bounds.h) continue;
        if (i == list.selected) {
            gfx_rect(list.bounds.x + 1, iy, list.bounds.w - 2, list.item_h, sel);
        }
        gfx_text(list.bounds.x + 12, iy + 6, items[i], fg);
        if (subtitles[i]) {
            gfx_text(list.bounds.x + 12, iy + 26, subtitles[i], subfg);
        }
    }
    
    // Scrollbar
    if (list.item_count > visible) {
        int sb_h = list.bounds.h * visible / list.item_count;
        int sb_y = list.bounds.y + list.scroll_y * list.bounds.h / (list.item_count * list.item_h);
        gfx_rect(list.bounds.x + list.bounds.w - 8, list.bounds.y + sb_y, 6, sb_h, Colors.gray);
    }
}

bool scrolllist_click(ScrollList* list, int mx, int my, bool mdown) {
    if (!list.visible || !mdown) return false;
    if (mx < list.bounds.x || mx >= list.bounds.x + list.bounds.w ||
        my < list.bounds.y || my >= list.bounds.y + list.bounds.h) return false;
    int idx = (my - list.bounds.y + list.scroll_y) / list.item_h;
    if (idx >= 0 && idx < list.item_count) {
        list.selected = idx;
        return true;
    }
    return false;
}

void scrolllist_scroll(ScrollList* list, int delta) {
    list.scroll_y += delta;
    int max_scroll = list.item_count * list.item_h - list.bounds.h;
    if (max_scroll < 0) max_scroll = 0;
    if (list.scroll_y < 0) list.scroll_y = 0;
    if (list.scroll_y > max_scroll) list.scroll_y = max_scroll;
}

struct ProgressBar {
    Rect bounds;
    int percent;
    Color bg, fill, border;
    bool visible;
}

void progressbar_draw(ProgressBar* bar) {
    if (!bar.visible) return;
    gfx_rect_rounded(bar.bounds.x, bar.bounds.y, bar.bounds.w, bar.bounds.h, 3, bar.bg);
    if (bar.percent > 0) {
        int fw = (bar.bounds.w - 8) * bar.percent / 100;
        gfx_rect_rounded(bar.bounds.x + 4, bar.bounds.y + 3, fw, bar.bounds.h - 6, 2, bar.fill);
    }
    gfx_rect_outline(bar.bounds.x, bar.bounds.y, bar.bounds.w, bar.bounds.h, bar.border);
}

struct TabBar {
    Rect bounds;
    const(char)*[8] labels;
    int count;
    int active;
}

void tabbar_draw(TabBar* tabs) {
    int tab_w = tabs.bounds.w / tabs.count;
    for (int i = 0; i < tabs.count; i++) {
        int x = tabs.bounds.x + i * tab_w;
        Color bg = (i == tabs.active) ? Colors.panel : Colors.darker;
        Color fg = (i == tabs.active) ? Colors.accent : Colors.lightgray;
        gfx_rect(x, tabs.bounds.y, tab_w, tabs.bounds.h, bg);
        if (i == tabs.active) {
            gfx_rect(x, tabs.bounds.y + tabs.bounds.h - 3, tab_w, 3, Colors.accent);
        }
        gfx_text_centered(x, tabs.bounds.y + (tabs.bounds.h - 8) / 2, tab_w, tabs.labels[i], fg);
    }
}

bool tabbar_click(TabBar* tabs, int mx, int my, bool mdown) {
    if (!mdown) return false;
    if (mx < tabs.bounds.x || mx >= tabs.bounds.x + tabs.bounds.w ||
        my < tabs.bounds.y || my >= tabs.bounds.y + tabs.bounds.h) return false;
    int tab_w = tabs.bounds.w / tabs.count;
    int idx = (mx - tabs.bounds.x) / tab_w;
    if (idx >= 0 && idx < tabs.count) {
        tabs.active = idx;
        return true;
    }
    return false;

    // ===================== FILE ICONS =====================

void draw_file_icon(int x, int y, int w, int h, Color paper, Color accent, const(char)* label, Color label_color) {
    // Paper sheet shape (rectangle with folded corner)
    int pw = w - 4; int ph = h - 4;
    int px = x + 2; int py = y + 2;
    
    // Main paper body
    gfx_rect(px, py, pw, ph, paper);
    // Folded corner (top-right triangle)
    int fold = 10;
    for (int i = 0; i < fold; i++) {
        gfx_line(px + pw - fold + i, py, px + pw, py + fold - i, Colors.lightgray);
    }
    gfx_rect(px + pw - fold, py, fold, fold, Colors.lightgray);
    // Outline
    gfx_rect_outline(px, py, pw, ph, Colors.gray);
    
    // Label text centered
    if (label && label[0]) {
        int tw = str_len(label) * 8;
        int tx = px + (pw - tw) / 2;
        int ty = py + (ph - 8) / 2 + 4;
        gfx_text(tx, ty, label, label_color);
    }
}

void draw_icon_exe(int x, int y, int w, int h) {
    // White page with "101010" binary text
    draw_file_icon(x, y, w, h, Colors.white, Colors.gray, "101010", Colors.dark);
}

void draw_icon_bin(int x, int y, int w, int h) {
    // White paper only, no text
    draw_file_icon(x, y, w, h, Colors.white, Colors.gray, null, Colors.white);
}

void draw_icon_c(int x, int y, int w, int h) {
    // Paper with blue "C"
    draw_file_icon(x, y, w, h, Colors.white, Colors.gray, "C", Colors.blue);
}

void draw_icon_rust(int x, int y, int w, int h) {
    // Paper with gear + "RS"
    int cx = x + w/2; int cy = y + h/2 + 2;
    // Gear outline (simplified as circle with teeth)
    gfx_circle(cx, cy - 2, 10, Colors.orange);
    gfx_circle(cx, cy - 2, 6, Colors.white);
    // RS text below
    gfx_text(cx - 10, cy + 12, "RS", Colors.orange);
    // Paper background behind
    gfx_rect(x + 2, y + 2, w - 4, h - 4, Color(250, 250, 250, 255));
    gfx_rect_outline(x + 2, y + 2, w - 4, h - 4, Colors.gray);
}

void draw_icon_d(int x, int y, int w, int h) {
    // Paper with red "D" (D language)
    draw_file_icon(x, y, w, h, Colors.white, Colors.gray, "D", Colors.red);
}

void draw_icon_json(int x, int y, int w, int h) {
    // Paper with yellow "{}"
    draw_file_icon(x, y, w, h, Colors.white, Colors.gray, "{}", Colors.yellow);
}

void draw_icon_txt(int x, int y, int w, int h) {
    // Paper with gray lines (text lines)
    int px = x + 4; int py = y + 4; int pw = w - 8; int ph = h - 8;
    gfx_rect(px, py, pw, ph, Colors.white);
    gfx_rect_outline(px, py, pw, ph, Colors.gray);
    // Text lines
    for (int i = 0; i < 4; i++) {
        gfx_rect(px + 4, py + 8 + i * 6, pw - 8, 2, Colors.lightgray);
    }
}

void draw_icon_dir(int x, int y, int w, int h) {
    // Folder icon (yellow rectangle with tab)
    int px = x + 4; int py = y + 8; int pw = w - 8; int ph = h - 12;
    // Tab
    gfx_rect(px + 4, py - 4, pw / 3, 4, Color(220, 200, 80, 255));
    // Body
    gfx_rect(px, py, pw, ph, Color(255, 220, 100, 255));
    gfx_rect_outline(px, py, pw, ph, Colors.gray);
}

void draw_icon_unknown(int x, int y, int w, int h) {
    // Gray page with "?"
    draw_file_icon(x, y, w, h, Colors.lightgray, Colors.gray, "?", Colors.dark);
}

void draw_file_icon_auto(int x, int y, int w, int h, const(char)* filename) {
    // Detect extension and draw appropriate icon
    int len = str_len(filename);
    const(char)* ext = null;
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') { ext = &filename[i+1]; break; }
    }
    
    if (!ext) {
        draw_icon_unknown(x, y, w, h);
        return;
    }
    
    // Check if directory (ends with /)
    if (len > 0 && filename[len-1] == '/') {
        draw_icon_dir(x, y, w, h);
        return;
    }
    
    // Lowercase extension for comparison
    char[8] ext_lower;
    for (int i = 0; i < 7 && ext[i]; i++) {
        char c = ext[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        ext_lower[i] = c;
        ext_lower[i+1] = 0;
    }
    
    if (str_eq(ext_lower.ptr, "exe")) {
        draw_icon_exe(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "bin")) {
        draw_icon_bin(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "c")) {
        draw_icon_c(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "rs")) {
        draw_icon_rust(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "d")) {
        draw_icon_d(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "json")) {
        draw_icon_json(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "txt") || str_eq(ext_lower.ptr, "md") || str_eq(ext_lower.ptr, "log")) {
        draw_icon_txt(x, y, w, h);
    } else if (str_eq(ext_lower.ptr, "sh")) {
        draw_file_icon(x, y, w, h, Colors.white, Colors.gray, "$_", Colors.green);
    } else {
        draw_icon_unknown(x, y, w, h);
    }
  }

}