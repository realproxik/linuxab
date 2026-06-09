module appstore.main;

import os.syscall;
import common.graphics;
import common.utils;

extern(C) @nogc nothrow:

// linuxab App Store v2.0
// Features: Search bar, Download manager, Installer, Games tab

enum {
    SCREEN_W = 1024,
    SCREEN_H = 768,
    MAX_APPS = 128,
    MAX_GAMES = 16,
    MAX_DOWNLOADS = 32,
    APP_NAME_LEN = 64,
    APP_DESC_LEN = 256,
    APP_URL_LEN = 512,
    APP_VER_LEN = 16,
    REPO_URL = "http://repo.linuxab.local/apps.json",
    GAMES_REPO_URL = "http://repo.linuxab.local/games.json",
    INSTALL_DIR = "/apps/",
    DOWNLOAD_DIR = "/downloads/",
    TEMP_FILE = "/tmp/download.bin",
}

// App states
enum AppState {
    NotInstalled = 0,
    Downloading = 1,
    Downloaded = 2,
    Installed = 3,
    UpdateAvailable = 4,
}

struct AppEntry {
    char[APP_NAME_LEN] name;
    char[APP_DESC_LEN] desc;
    char[APP_VER_LEN] version;
    char[APP_VER_LEN] installed_version;
    char[APP_URL_LEN] url;
    int size_kb;
    int state; // AppState
    int download_pct;
    bool is_game;
}

struct DownloadTask {
    bool active;
    int app_idx;
    long downloaded;
    long total;
    int pct;
    char[APP_NAME_LEN] name;
}

// Global data
__gshared AppEntry[MAX_APPS] g_apps;
__gshared int g_app_count = 0;
__gshared int g_filtered_count = 0;
__gshared int[128] g_filtered_idx; // indices into g_apps

__gshared DownloadTask[MAX_DOWNLOADS] g_downloads;
__gshared int g_download_count = 0;

__gshared char[256] g_status_msg;
__gshared char[4096] g_json_buf;
__gshared char[4096] g_net_buf;

// UI Tabs
enum TAB_APPS = 0;
enum TAB_GAMES = 1;
enum TAB_INSTALLED = 2;
enum TAB_DOWNLOADS = 3;
__gshared int g_tab = TAB_APPS;

// Widgets
__gshared InputField g_search;
__gshared Button g_btn_refresh;
__gshared Button g_btn_download;
__gshared Button g_btn_install;
__gshared Button g_btn_uninstall;
__gshared Button g_btn_launch;
__gshared Button g_btn_cancel;
__gshared Button g_btn_clear_search;
__gshared ScrollList g_list;
__gshared ProgressBar g_progress;
__gshared TabBar g_tabs;

// Mouse/keyboard
__gshared int g_mx, g_my;
__gshared bool g_mdown, g_mclick;
__gshared bool g_needs_refresh = true;

// ===================== INIT =====================

void init_ui() {
    // Search bar
    g_search.bounds = Rect(20, 70, 400, 36);
    g_search.placeholder = "Search apps and games...";
    g_search.text[0] = 0;
    g_search.len = 0;
    g_search.active = false;
    
    g_btn_clear_search.bounds = Rect(430, 70, 36, 36);
    g_btn_clear_search.label = "X";
    g_btn_clear_search.bg = Colors.darker;
    g_btn_clear_search.fg = Colors.lightgray;
    g_btn_clear_search.border = Colors.gray;
    g_btn_clear_search.visible = true;
    
    // Tabs
    g_tabs.bounds = Rect(20, 20, SCREEN_W - 40, 40);
    g_tabs.labels[0] = "Apps";
    g_tabs.labels[1] = "Games";
    g_tabs.labels[2] = "Installed";
    g_tabs.labels[3] = "Downloads";
    g_tabs.count = 4;
    g_tabs.active = 0;
    
    // App list
    g_list.bounds = Rect(20, 120, 600, 520);
    g_list.item_h = 70;
    g_list.item_count = 0;
    g_list.selected = -1;
    g_list.scroll_y = 0;
    g_list.visible = true;
    
    // Action buttons (right side)
    int bx = 660; int by = 120; int bw = 180; int bh = 44;
    
    g_btn_download.bounds = Rect(bx, by, bw, bh);
    g_btn_download.label = "Download";
    g_btn_download.bg = Colors.blue; g_btn_download.fg = Colors.white;
    g_btn_download.border = Colors.accent; g_btn_download.visible = false;
    
    g_btn_install.bounds = Rect(bx, by + 56, bw, bh);
    g_btn_install.label = "Install";
    g_btn_install.bg = Colors.success; g_btn_install.fg = Colors.white;
    g_btn_install.border = Colors.green; g_btn_install.visible = false;
    
    g_btn_launch.bounds = Rect(bx, by + 112, bw, bh);
    g_btn_launch.label = "Launch";
    g_btn_launch.bg = Colors.orange; g_btn_launch.fg = Colors.white;
    g_btn_launch.border = Colors.yellow; g_btn_launch.visible = false;
    
    g_btn_uninstall.bounds = Rect(bx, by + 168, bw, bh);
    g_btn_uninstall.label = "Uninstall";
    g_btn_uninstall.bg = Colors.danger; g_btn_uninstall.fg = Colors.white;
    g_btn_uninstall.border = Colors.red; g_btn_uninstall.visible = false;
    
    g_btn_cancel.bounds = Rect(bx, by, bw, bh);
    g_btn_cancel.label = "Cancel";
    g_btn_cancel.bg = Colors.danger; g_btn_cancel.fg = Colors.white;
    g_btn_cancel.border = Colors.red; g_btn_cancel.visible = false;
    
    g_btn_refresh.bounds = Rect(SCREEN_W - 140, 70, 120, 36);
    g_btn_refresh.label = "Refresh";
    g_btn_refresh.bg = Colors.panel; g_btn_refresh.fg = Colors.accent;
    g_btn_refresh.border = Colors.accent; g_btn_refresh.visible = true;
    
    // Progress bar
    g_progress.bounds = Rect(20, SCREEN_H - 60, 600, 24);
    g_progress.percent = 0;
    g_progress.bg = Colors.darker; g_progress.fill = Colors.accent;
    g_progress.border = Colors.gray; g_progress.visible = false;
}

// ===================== SEARCH & FILTER =====================

bool str_contains(const(char)* haystack, const(char)* needle) {
    if (!needle[0]) return true;
    int hlen = str_len(haystack);
    int nlen = str_len(needle);
    for (int i = 0; i <= hlen - nlen; i++) {
        bool match = true;
        for (int j = 0; j < nlen; j++) {
            char a = haystack[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) { match = false; break; }
        }
        if (match) return true;
    }
    return false;
}

void apply_filter() {
    g_filtered_count = 0;
    g_list.selected = -1;
    
    for (int i = 0; i < g_app_count; i++) {
        AppEntry* app = &g_apps[i];
        bool match = false;
        
        // Tab filter
        if (g_tab == TAB_APPS && app.is_game) continue;
        if (g_tab == TAB_GAMES && !app.is_game) continue;
        if (g_tab == TAB_INSTALLED && app.state != AppState.Installed && app.state != AppState.UpdateAvailable) continue;
        if (g_tab == TAB_DOWNLOADS && app.state != AppState.Downloading && app.state != AppState.Downloaded) continue;
        
        // Search filter
        if (g_search.len > 0) {
            if (str_contains(app.name.ptr, g_search.text.ptr) ||
                str_contains(app.desc.ptr, g_search.text.ptr)) {
                match = true;
            }
        } else {
            match = true;
        }
        
        if (match) {
            g_filtered_idx[g_filtered_count] = i;
            g_filtered_count++;
        }
    }
    
    g_list.item_count = g_filtered_count;
    if (g_filtered_count > 0 && g_list.selected < 0) g_list.selected = 0;
    g_list.scroll_y = 0;
}

// ===================== REPO FETCH =====================

bool fetch_repo() {
    str_copy(g_status_msg.ptr, "Fetching repository...", 256);
    
    long len = net_get(REPO_URL, g_json_buf.ptr, 4096);
    if (len <= 0) {
        str_copy(g_status_msg.ptr, "Failed to fetch repo!", 256);
        return false;
    }
    g_json_buf[len] = 0;
    
    g_app_count = 0;
    const(char)* p = g_json_buf.ptr;
    
    while (*p && g_app_count < MAX_APPS) {
        // Look for "name" field
        if (p[0] == '"' && p[1] == 'n' && p[2] == 'a' && p[3] == 'm' && p[4] == 'e') {
            AppEntry* app = &g_apps[g_app_count];
            mem_set(app, 0, AppEntry.sizeof);
            
            json_find_str(p, "name", app.name.ptr, APP_NAME_LEN);
            json_find_str(p, "description", app.desc.ptr, APP_DESC_LEN);
            json_find_str(p, "version", app.version.ptr, APP_VER_LEN);
            json_find_str(p, "url", app.url.ptr, APP_URL_LEN);
            json_find_int(p, "size_kb", &app.size_kb);
            
            // Check if game
            char[32] type_buf;
            if (json_find_str(p, "type", type_buf.ptr, 32) && str_eq(type_buf.ptr, "game")) {
                app.is_game = true;
            }
            
            // Check installed version
            char[128] path;
            str_copy(path.ptr, INSTALL_DIR, 128);
            str_cat(path.ptr, app.name.ptr, 128);
            long fd = sys_open(path.ptr, 0);
            if (fd >= 0) {
                app.state = AppState.Installed;
                sys_close(fd);
                // Check version file
                char[128] verpath;
                str_copy(verpath.ptr, INSTALL_DIR, 128);
                str_cat(verpath.ptr, ".", 128);
                str_cat(verpath.ptr, app.name.ptr, 128);
                str_cat(verpath.ptr, ".ver", 128);
                long vfd = sys_open(verpath.ptr, 0);
                if (vfd >= 0) {
                    char[32] vbuf;
                    long vlen = sys_read(vfd, vbuf.ptr, 32);
                    if (vlen > 0) {
                        vbuf[vlen] = 0;
                        str_copy(app.installed_version.ptr, vbuf.ptr, APP_VER_LEN);
                        if (!str_eq(app.version.ptr, app.installed_version.ptr)) {
                            app.state = AppState.UpdateAvailable;
                        }
                    }
                    sys_close(vfd);
                }
            }
            
            // Check downloaded
            char[128] dlpath;
            str_copy(dlpath.ptr, DOWNLOAD_DIR, 128);
            str_cat(dlpath.ptr, app.name.ptr, 128);
            str_cat(dlpath.ptr, ".bin", 128);
            long dfd = sys_open(dlpath.ptr, 0);
            if (dfd >= 0) {
                if (app.state == AppState.NotInstalled) app.state = AppState.Downloaded;
                sys_close(dfd);
            }
            
            g_app_count++;
        }
        p++;
    }
    
    char[32] count_buf;
    int_to_str(g_app_count, count_buf.ptr, 32);
    str_copy(g_status_msg.ptr, "Loaded ", 256);
    str_cat(g_status_msg.ptr, count_buf.ptr, 256);
    str_cat(g_status_msg.ptr, " apps", 256);
    
    apply_filter();
    return true;
}

// ===================== DOWNLOAD =====================

int find_download(int app_idx) {
    for (int i = 0; i < MAX_DOWNLOADS; i++) {
        if (g_downloads[i].active && g_downloads[i].app_idx == app_idx) return i;
    }
    return -1;
}

int alloc_download() {
    for (int i = 0; i < MAX_DOWNLOADS; i++) {
        if (!g_downloads[i].active) return i;
    }
    return -1;
}

bool start_download(int app_idx) {
    if (app_idx < 0 || app_idx >= g_app_count) return false;
    AppEntry* app = &g_apps[app_idx];
    
    if (app.state == AppState.Downloading) return false;
    if (app.state == AppState.Installed) return false;
    
    int didx = alloc_download();
    if (didx < 0) {
        str_copy(g_status_msg.ptr, "Too many downloads!", 256);
        return false;
    }
    
    app.state = AppState.Downloading;
    app.download_pct = 0;
    
    g_downloads[didx].active = true;
    g_downloads[didx].app_idx = app_idx;
    g_downloads[didx].downloaded = 0;
    g_downloads[didx].total = app.size_kb * 1024;
    g_downloads[didx].pct = 0;
    str_copy(g_downloads[didx].name.ptr, app.name.ptr, APP_NAME_LEN);
    
    str_copy(g_status_msg.ptr, "Downloading ", 256);
    str_cat(g_status_msg.ptr, app.name.ptr, 256);
    
    // Create download file
    char[128] dlpath;
    str_copy(dlpath.ptr, DOWNLOAD_DIR, 128);
    str_cat(dlpath.ptr, app.name.ptr, 128);
    str_cat(dlpath.ptr, ".bin", 128);
    long fd = sys_open(dlpath.ptr, 1);
    if (fd < 0) {
        app.state = AppState.NotInstalled;
        g_downloads[didx].active = false;
        str_copy(g_status_msg.ptr, "Failed to create download file!", 256);
        return false;
    }
    
    // Download in chunks
    long total = 0;
    long chunk_size = 4096;
    while (total < app.size_kb * 1024) {
        long to_read = chunk_size;
        if (app.size_kb * 1024 - total < chunk_size) to_read = app.size_kb * 1024 - total;
        
        long got = net_get(app.url.ptr, g_net_buf.ptr, to_read);
        if (got <= 0) break;
        
        sys_write(fd, g_net_buf.ptr, got);
        total += got;
        app.download_pct = cast(int)(total * 100 / (app.size_kb * 1024));
        g_downloads[didx].downloaded = total;
        g_downloads[didx].pct = app.download_pct;
        
        // Redraw to show progress
        draw_ui();
    }
    
    sys_close(fd);
    g_downloads[didx].active = false;
    
    if (total >= app.size_kb * 1024) {
        app.state = AppState.Downloaded;
        app.download_pct = 100;
        str_copy(g_status_msg.ptr, "Downloaded ", 256);
        str_cat(g_status_msg.ptr, app.name.ptr, 256);
    } else {
        app.state = AppState.NotInstalled;
        app.download_pct = 0;
        str_copy(g_status_msg.ptr, "Download failed: ", 256);
        str_cat(g_status_msg.ptr, app.name.ptr, 256);
    }
    
    apply_filter();
    return true;
}

void cancel_download(int app_idx) {
    int didx = find_download(app_idx);
    if (didx >= 0) {
        g_downloads[didx].active = false;
    }
    if (app_idx >= 0 && app_idx < g_app_count) {
        g_apps[app_idx].state = AppState.NotInstalled;
        g_apps[app_idx].download_pct = 0;
    }
    apply_filter();
}

// ===================== INSTALL / UNINSTALL / LAUNCH =====================

bool install_app(int app_idx) {
    if (app_idx < 0 || app_idx >= g_app_count) return false;
    AppEntry* app = &g_apps[app_idx];
    
    char[128] src;
    str_copy(src.ptr, DOWNLOAD_DIR, 128);
    str_cat(src.ptr, app.name.ptr, 128);
    str_cat(src.ptr, ".bin", 128);
    
    char[128] dst;
    str_copy(dst.ptr, INSTALL_DIR, 128);
    str_cat(dst.ptr, app.name.ptr, 128);
    
    // Copy file
    long sfd = sys_open(src.ptr, 0);
    long dfd = sys_open(dst.ptr, 1);
    if (sfd < 0 || dfd < 0) {
        if (sfd >= 0) sys_close(sfd);
        if (dfd >= 0) sys_close(dfd);
        str_copy(g_status_msg.ptr, "Install failed!", 256);
        return false;
    }
    
    char[1024] buf;
    long n;
    while ((n = sys_read(sfd, buf.ptr, 1024)) > 0) {
        sys_write(dfd, buf.ptr, n);
    }
    sys_close(sfd);
    sys_close(dfd);
    
    // Write version file
    char[128] verpath;
    str_copy(verpath.ptr, INSTALL_DIR, 128);
    str_cat(verpath.ptr, ".", 128);
    str_cat(verpath.ptr, app.name.ptr, 128);
    str_cat(verpath.ptr, ".ver", 128);
    long vfd = sys_open(verpath.ptr, 1);
    if (vfd >= 0) {
        sys_write(vfd, app.version.ptr, str_len(app.version.ptr));
        sys_close(vfd);
    }
    
    app.state = AppState.Installed;
    str_copy(app.installed_version.ptr, app.version.ptr, APP_VER_LEN);
    str_copy(g_status_msg.ptr, "Installed ", 256);
    str_cat(g_status_msg.ptr, app.name.ptr, 256);
    
    apply_filter();
    return true;
}

bool uninstall_app(int app_idx) {
    if (app_idx < 0 || app_idx >= g_app_count) return false;
    AppEntry* app = &g_apps[app_idx];
    
    char[128] path;
    str_copy(path.ptr, INSTALL_DIR, 128);
    str_cat(path.ptr, app.name.ptr, 128);
    fs_delete(path.ptr);
    
    char[128] verpath;
    str_copy(verpath.ptr, INSTALL_DIR, 128);
    str_cat(verpath.ptr, ".", 128);
    str_cat(verpath.ptr, app.name.ptr, 128);
    str_cat(verpath.ptr, ".ver", 128);
    fs_delete(verpath.ptr);
    
    app.state = AppState.NotInstalled;
    app.installed_version[0] = 0;
    str_copy(g_status_msg.ptr, "Uninstalled ", 256);
    str_cat(g_status_msg.ptr, app.name.ptr, 256);
    
    apply_filter();
    return true;
}

bool launch_app(int app_idx) {
    if (app_idx < 0 || app_idx >= g_app_count) return false;
    AppEntry* app = &g_apps[app_idx];
    if (app.state != AppState.Installed && app.state != AppState.UpdateAvailable) return false;
    
    char[128] path;
    str_copy(path.ptr, INSTALL_DIR, 128);
    str_cat(path.ptr, app.name.ptr, 128);
    syscall(SYS.exec, cast(long)path.ptr);
    return true;
}

// ===================== DRAWING =====================

void draw_header() {
    // Top bar gradient
    gfx_gradient(0, 0, SCREEN_W, 60, Color(40, 40, 55, 255), Color(25, 25, 35, 255));
    gfx_text(20, 18, "linuxab APP STORE", Colors.accent);
    
    // Status
    gfx_text(SCREEN_W - 300, 18, g_status_msg.ptr, Colors.lightgray);
}

void draw_app_details(int filtered_idx) {
    if (filtered_idx < 0 || filtered_idx >= g_filtered_count) return;
    int app_idx = g_filtered_idx[filtered_idx];
    AppEntry* app = &g_apps[app_idx];
    int x = 660, y = 120;
    
    // Panel background
    gfx_rect_rounded(x - 10, y - 10, 340, 400, 8, Colors.panel);
    gfx_rect_outline(x - 10, y - 10, 340, 400, Colors.gray);
    
    gfx_text(x, y, "=== DETAILS ===", Colors.accent); y += 24;
    gfx_text(x, y, app.name.ptr, Colors.white); y += 20;
    
    gfx_text(x, y, "Version: ", Colors.gray);
    gfx_text(x + 70, y, app.version.ptr, Colors.white); y += 18;
    
    if (app.installed_version[0]) {
        gfx_text(x, y, "Installed: ", Colors.gray);
        gfx_text(x + 80, y, app.installed_version.ptr, Colors.yellow); y += 18;
    }
    
    gfx_text(x, y, "Size: ", Colors.gray);
    char[32] sz;
    int_to_str(app.size_kb, sz.ptr, 32);
    gfx_text(x + 50, y, sz.ptr, Colors.white);
    gfx_text(x + 90, y, " KB", Colors.gray); y += 18;
    
    gfx_text(x, y, "Type: ", Colors.gray);
    gfx_text(x + 50, y, app.is_game ? "Game" : "Application", app.is_game ? Colors.orange : Colors.cyan); y += 24;
    
    // Description (word wrap roughly)
    gfx_text(x, y, app.desc.ptr, Colors.lightgray); y += 40;
    
    // State badge
    const(char)* state_str = "Unknown";
    Color state_col = Colors.gray;
    switch (app.state) {
        case AppState.NotInstalled: state_str = "NOT INSTALLED"; state_col = Colors.gray; break;
        case AppState.Downloading: state_str = "DOWNLOADING..."; state_col = Colors.blue; break;
        case AppState.Downloaded: state_str = "READY TO INSTALL"; state_col = Colors.warning; break;
        case AppState.Installed: state_str = "INSTALLED"; state_col = Colors.success; break;
        case AppState.UpdateAvailable: state_str = "UPDATE AVAILABLE"; state_col = Colors.orange; break;
        default: break;
    }
    gfx_text(x, y, state_str, state_col); y += 30;
    
    // Progress if downloading
    if (app.state == AppState.Downloading) {
        g_progress.bounds = Rect(x, y, 300, 20);
        g_progress.percent = app.download_pct;
        g_progress.visible = true;
        progressbar_draw(&g_progress);
        g_progress.visible = false;
        y += 30;
        
        char[32] pct_str;
        int_to_str(app.download_pct, pct_str.ptr, 32);
        gfx_text(x, y, pct_str.ptr, Colors.accent);
        gfx_text(x + 30, y, "%", Colors.gray);
    }
    
    // Update buttons visibility
    g_btn_download.visible = (app.state == AppState.NotInstalled || app.state == AppState.UpdateAvailable);
    g_btn_install.visible = (app.state == AppState.Downloaded);
    g_btn_launch.visible = (app.state == AppState.Installed || app.state == AppState.UpdateAvailable);
    g_btn_uninstall.visible = (app.state == AppState.Installed || app.state == AppState.UpdateAvailable);
    g_btn_cancel.visible = (app.state == AppState.Downloading);
    
    if (app.state == AppState.UpdateAvailable) {
        g_btn_download.label = "Update";
    } else {
        g_btn_download.label = "Download";
    }
    
    button_draw(&g_btn_download);
    button_draw(&g_btn_install);
    button_draw(&g_btn_launch);
    button_draw(&g_btn_uninstall);
    button_draw(&g_btn_cancel);
}

void draw_ui() {
    gfx_clear(Colors.black);
    
    draw_header();
    
    // Search bar
    inputfield_draw(&g_search);
    button_draw(&g_btn_clear_search);
    
    // Tabs
    tabbar_draw(&g_tabs);
    
    // Refresh button
    button_draw(&g_btn_refresh);
    
    // App list
    const(char)*[MAX_APPS] names;
    const(char)*[MAX_APPS] subtitles;
    for (int i = 0; i < g_filtered_count; i++) {
        int app_idx = g_filtered_idx[i];
        AppEntry* app = &g_apps[app_idx];
        names[i] = app.name.ptr;
        
        // Build subtitle with state
        static __gshared char[32][MAX_APPS] sub_bufs;
        char[32] st;
        switch (app.state) {
            case AppState.NotInstalled: str_copy(st.ptr, "Not installed", 32); break;
            case AppState.Downloading: 
                str_copy(st.ptr, "Downloading ", 32); 
                char[8] p; int_to_str(app.download_pct, p.ptr, 8);
                str_cat(st.ptr, p.ptr, 32); str_cat(st.ptr, "%", 32);
                break;
            case AppState.Downloaded: str_copy(st.ptr, "Ready to install", 32); break;
            case AppState.Installed: str_copy(st.ptr, "Installed", 32); break;
            case AppState.UpdateAvailable: str_copy(st.ptr, "Update available", 32); break;
            default: str_copy(st.ptr, "", 32); break;
        }
        str_copy(sub_bufs[i].ptr, st.ptr, 32);
        subtitles[i] = sub_bufs[i].ptr;
    }
    
    g_list.item_count = g_filtered_count;
    scrolllist_draw(&g_list, names.ptr, subtitles.ptr, Colors.darker, Colors.highlight, Colors.white, Colors.gray);
    
    // Details panel
    if (g_list.selected >= 0 && g_list.selected < g_filtered_count) {
        draw_app_details(g_list.selected);
    } else {
        // Hide buttons when nothing selected
        g_btn_download.visible = false;
        g_btn_install.visible = false;
        g_btn_launch.visible = false;
        g_btn_uninstall.visible = false;
        g_btn_cancel.visible = false;
    }
    
    fb_swap();
}

// ===================== INPUT =====================

void handle_input() {
    InputEvent ev;
    while (input_poll(&ev)) {
        if (ev.type == 1) { // mouse move
            g_mx = ev.x; g_my = ev.y;
        } else if (ev.type == 2) { // mouse click
            g_mdown = (ev.code == 1);
            if (ev.code == 0) g_mclick = true;
        } else if (ev.type == 0) { // keyboard
            if (ev.code == 103) { // UP
                if (g_list.selected > 0) g_list.selected--;
            } else if (ev.code == 108) { // DOWN
                if (g_list.selected < g_filtered_count - 1) g_list.selected++;
            } else if (ev.code == 14) { // BACKSPACE
                inputfield_type(&g_search, 8);
                if (g_search.changed) { apply_filter(); g_search.changed = false; }
            } else {
                // Typing
                if (ev.code >= 32 && ev.code < 127) {
                    inputfield_type(&g_search, cast(char)ev.code);
                    if (g_search.changed) { apply_filter(); g_search.changed = false; }
                }
            }
        }
    }
    
    if (g_mclick) {
        // Search bar
        inputfield_click(&g_search, g_mx, g_my, g_mdown);
        
        // Clear search
        if (button_update(&g_btn_clear_search, g_mx, g_my, g_mdown)) {
            inputfield_clear(&g_search);
            apply_filter();
        }
        
        // Tabs
        if (tabbar_click(&g_tabs, g_mx, g_my, g_mdown)) {
            g_tab = g_tabs.active;
            apply_filter();
        }
        
        // Refresh
        if (button_update(&g_btn_refresh, g_mx, g_my, g_mdown)) {
            fetch_repo();
        }
        
        // App list selection
        if (scrolllist_click(&g_list, g_mx, g_my, g_mdown)) {
            // selected updated
        }
        
        // Action buttons
        if (g_list.selected >= 0 && g_list.selected < g_filtered_count) {
            int app_idx = g_filtered_idx[g_list.selected];
            
            if (button_update(&g_btn_download, g_mx, g_my, g_mdown)) {
                start_download(app_idx);
            }
            if (button_update(&g_btn_install, g_mx, g_my, g_mdown)) {
                install_app(app_idx);
            }
            if (button_update(&g_btn_launch, g_mx, g_my, g_mdown)) {
                launch_app(app_idx);
            }
            if (button_update(&g_btn_uninstall, g_mx, g_my, g_mdown)) {
                uninstall_app(app_idx);
            }
            if (button_update(&g_btn_cancel, g_mx, g_my, g_mdown)) {
                cancel_download(app_idx);
            }
        }
        
        g_mclick = false;
    }
}

// ===================== MAIN =====================

void main() {
    if (!gfx_init()) {
        sys_exit(1);
    }
    
    init_ui();
    fetch_repo();
    
    while (true) {
        handle_input();
        draw_ui();
        
        // Frame limit ~60fps
        long start = sys_time_ms();
        while (sys_time_ms() - start < 16) {}
    }
}