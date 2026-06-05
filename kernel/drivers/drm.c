// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/gpu/drm.c
 * Simplified DRM implementation
 */

#include "drm.h"
#include "printk.h"

int drm_edid_parse(uint8_t *edid, struct drm_display_mode *modes, int max_modes)
{
    if (!edid || edid[0] != 0x00 || edid[7] != 0x00)
        return 0;

    /* Checksum */
    uint8_t sum = 0;
    for (int i = 0; i < 128; i++) sum += edid[i];
    if (sum != 0) {
        printk(KERN_WARNING "DRM: EDID checksum failed\n");
        return 0;
    }

    int num = 0;
    /* Parse standard timing descriptors (bytes 0x36-0x7D) */
    for (int i = 0; i < 4 && num < max_modes; i++) {
        uint8_t *d = &edid[0x36 + i * 18];
        if (d[0] == 0 && d[1] == 0) continue;

        uint16_t xres = (d[2] + 31) * 8;
        uint16_t yres = xres; /* Simplified aspect ratio */
        uint32_t clock = (d[0] * 256 + d[1]) * 10000;

        memset(&modes[num], 0, sizeof(modes[num]));
        snprintf(modes[num].name, sizeof(modes[num].name), "%dx%d", xres, yres);
        modes[num].clock = clock / 1000;
        modes[num].hdisplay = xres;
        modes[num].vdisplay = yres;
        modes[num].htotal = xres + 100;
        modes[num].vtotal = yres + 10;
        modes[num].hsync_start = xres + 10;
        modes[num].hsync_end = xres + 50;
        modes[num].vsync_start = yres + 2;
        modes[num].vsync_end = yres + 6;
        modes[num].type = DRM_MODE_TYPE_DRIVER;
        num++;
    }

    /* Parse detailed timing descriptors */
    for (int i = 0; i < 4 && num < max_modes; i++) {
        uint8_t *d = &edid[0x36 + i * 18];
        if (d[0] == 0 && d[1] == 0) continue;
        if (d[3] == 0xFD) continue; /* Monitor range */
        if (d[3] == 0xFC) continue; /* Monitor name */
        if (d[3] == 0xFE) continue; /* Unspecified text */
        if (d[3] == 0xFF) continue; /* Serial number */

        uint32_t pixclk = (d[0] + (d[1] << 8)) * 10000; /* Actually 10kHz units */
        uint16_t ha = d[2] + ((d[4] & 0xF0) << 4);
        uint16_t hbl = d[3] + ((d[4] & 0x0F) << 8);
        uint16_t va = d[5] + ((d[7] & 0xF0) << 4);
        uint16_t vbl = d[6] + ((d[7] & 0x0F) << 8);
        uint8_t hso = d[8] + ((d[11] & 0xC0) << 2);
        uint8_t hsw = d[9] + ((d[11] & 0x30) << 4);
        uint8_t vso = ((d[10] & 0xF0) >> 4) + ((d[11] & 0x0C) << 2);
        uint8_t vsw = (d[10] & 0x0F) + ((d[11] & 0x03) << 4);

        memset(&modes[num], 0, sizeof(modes[num]));
        snprintf(modes[num].name, sizeof(modes[num].name), "EDID:%dx%d", ha, va);
        modes[num].clock = pixclk / 1000;
        modes[num].hdisplay = ha;
        modes[num].htotal = ha + hbl;
        modes[num].hsync_start = ha + hso;
        modes[num].hsync_end = ha + hso + hsw;
        modes[num].vdisplay = va;
        modes[num].vtotal = va + vbl;
        modes[num].vsync_start = va + vso;
        modes[num].vsync_end = va + vso + vsw;
        modes[num].type = DRM_MODE_TYPE_PREFERRED | DRM_MODE_TYPE_DRIVER;
        num++;
    }

    printk(KERN_INFO "DRM: Parsed %d modes from EDID\n", num);
    return num;
}

int drm_framebuffer_init(struct drm_device *dev, struct drm_framebuffer *fb,
                           uint32_t width, uint32_t height, uint32_t bpp)
{
    fb->width = width;
    fb->height = height;
    fb->bpp = bpp;
    fb->depth = bpp;
    fb->pitch = ((width * bpp / 8) + 63) & ~63; /* Align to 64 bytes */
    fb->size = fb->pitch * height;
    fb->base = (uint64_t)kmalloc_page(); /* Simplified: allocate one page for now */
    if (!fb->base) return -1;

    memset((void *)fb->base, 0, fb->size);
    fb->obj_id = 1;
    dev->fb = fb;
    return 0;
}

void drm_framebuffer_clear(struct drm_framebuffer *fb, uint32_t color)
{
    if (!fb || !fb->base) return;
    uint32_t *p = (uint32_t *)fb->base;
    uint32_t pixels = fb->pitch * fb->height / 4;
    for (uint32_t i = 0; i < pixels; i++)
        p[i] = color;
}

int drm_mode_set(struct drm_device *dev, uint32_t crtc_id,
                 struct drm_display_mode *mode,
                 struct drm_framebuffer *fb)
{
    if (crtc_id >= dev->num_crtcs)
        return -1;

    struct drm_crtc *crtc = &dev->crtcs[crtc_id];
    crtc->mode = mode;
    crtc->fb = fb;
    crtc->enabled = true;

    if (dev->set_mode)
        dev->set_mode(crtc, mode);

    if (dev->fb_set)
        dev->fb_set(crtc, fb);

    printk(KERN_INFO "DRM: Mode set on CRTC %d: %s @ %dx%d\n",
           crtc_id, mode->name, mode->hdisplay, mode->vdisplay);
    return 0;
}

int drm_init(struct drm_device *dev)
{
    if (!dev) return -1;
    memset(dev->crtcs, 0, sizeof(dev->crtcs));
    memset(dev->connectors, 0, sizeof(dev->connectors));
    dev->num_crtcs = 0;
    dev->num_connectors = 0;
    dev->fb = NULL;
    dev->initialized = true;
    printk(KERN_INFO "DRM: Initialized\n");
    return 0;
}
