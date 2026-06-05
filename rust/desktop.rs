#![no_std]

use core::sync::atomic::{AtomicU32, Ordering};
use crate::gpu::{Framebuffer, Color};
use crate::terminal::Terminal;
use crate::input::{MouseState, KeyEvent};

// Desktop dimensions
const SCREEN_WIDTH: u32 = 1024;
const SCREEN_HEIGHT: u32 = 768;
const TASKBAR_HEIGHT: u32 = 40;
const ICON_SIZE: u32 = 48;
const ICON_TEXT_HEIGHT: u32 = 20;
const GRID_SPACING: u32 = 80;

// Colors
const DESKTOP_BG: Color = Color(0x1E, 0x1E, 0x1E);
const TASKBAR_BG: Color = Color(0x2D, 0x2D, 0x2D);
const ICON_TEXT_COLOR: Color = Color(0xFF, 0xFF, 0xFF);
const ICON_HIGHLIGHT: Color = Color(0x00, 0x78, 0xD4);
const WINDOW_BG: Color = Color(0xFF, 0xFF, 0xFF);
const WINDOW_TITLE_BAR: Color = Color(0x00, 0x78, 0xD4);
const CONTEXT_MENU_BG: Color = Color(0x2D, 0x2D, 0x2D);
const CONTEXT_MENU_HOVER: Color = Color(0x00, 0x78, 0xD4);
const BUTTON_BG: Color = Color(0xE0, 0xE0, 0xE0);
const BUTTON_HOVER: Color = Color(0xC0, 0xC0, 0xC0);

// Application types
#[derive(Clone, Copy, PartialEq)]
pub enum AppType {
    Terminal,
    Browser,
    Store,
    FileManager,
    Settings,
    Calculator,
    TextEditor,
    ImageViewer,
    MusicPlayer,
    VideoPlayer,
    GameLauncher,
    RecycleBin,
    Custom(&'static str),
}

// Desktop icon
pub struct DesktopIcon {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
    pub label: &'static str,
    pub app_type: AppType,
    pub icon_data: &'static [u8], // 48x48 RGBA icon
    pub selected: bool,
    pub double_click_timer: u32,
}

// Window state
pub struct Window {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
    pub title: &'static str,
    pub app_type: AppType,
    pub minimized: bool,
    pub maximized: bool,
    pub focused: bool,
    pub content: WindowContent,
    pub close_hover: bool,
    pub minimize_hover: bool,
    pub maximize_hover: bool,
    pub drag_offset_x: i32,
    pub drag_offset_y: i32,
    pub is_dragging: bool,
}

pub enum WindowContent {
    Terminal(Terminal),
    Browser(BrowserState),
    Store(StoreState),
    FileManager(FileManagerState),
    RecycleBin(RecycleBinState),
    Properties(PropertiesState),
    Empty,
}

// Browser state
pub struct BrowserState {
    pub url: [u8; 2048],
    pub url_len: usize,
    pub page_content: &'static str, // Simplified HTML rendering
    pub scroll_y: u32,
    pub loading: bool,
    pub back_history: [[u8; 2048]; 16],
    pub back_count: usize,
    pub forward_history: [[u8; 2048]; 16],
    pub forward_count: usize,
    pub address_bar_focused: bool,
}

// App Store state
pub struct StoreState {
    pub categories: [StoreCategory; 6],
    pub selected_category: usize,
    pub apps: [StoreApp; 64],
    pub app_count: usize,
    pub scroll_y: u32,
    pub search_query: [u8; 64],
    pub search_len: usize,
    pub installing_app: Option<<usize>,
    pub install_progress: u8,
}

pub struct StoreCategory {
    pub name: &'static str,
    pub icon: &'static [u8],
}

pub struct StoreApp {
    pub name: &'static str,
    pub description: &'static str,
    pub category: usize,
    pub icon: &'static [u8],
    pub rating: u8, // 0-50 (0-5.0 stars)
    pub downloads: u32,
    pub installed: bool,
    pub size_mb: u16,
    pub is_game: bool,
}

// File Manager state
pub struct FileManagerState {
    pub current_path: [u8; 256],
    pub path_len: usize,
    pub files: [FileEntry; 128],
    pub file_count: usize,
    pub selected_file: i32, // -1 = none
    pub scroll_y: u32,
    pub view_mode: ViewMode,
}

pub enum ViewMode {
    Icons,
    List,
    Details,
}

pub struct FileEntry {
    pub name: [u8; 128],
    pub name_len: usize,
    pub is_dir: bool,
    pub size: u64,
    pub modified: u64,
    pub icon: &'static [u8],
}

// Recycle Bin state
pub struct RecycleBinState {
    pub deleted_files: [DeletedFile; 256],
    pub file_count: usize,
    pub selected_file: i32,
    pub scroll_y: u32,
}

pub struct DeletedFile {
    pub original_path: [u8; 256],
    pub path_len: usize,
    pub name: [u8; 128],
    pub name_len: usize,
    pub size: u64,
    pub deleted_time: u64,
    pub icon: &'static [u8],
}

// Properties dialog state
pub struct PropertiesState {
    pub target_name: [u8; 128],
    pub name_len: usize,
    pub target_path: [u8; 256],
    pub path_len: usize,
    pub file_size: u64,
    pub is_directory: bool,
    pub created: u64,
    pub modified: u64,
    pub owner: &'static str,
    pub permissions: u16,
    pub is_safe: bool,
    pub app_type: AppType,
}

// Context menu
pub struct ContextMenu {
    pub visible: bool,
    pub x: u32,
    pub y: u32,
    pub target_icon: Option<<usize>, // Desktop icon index
    pub target_file: Option<<usize>, // File manager index
    pub items: [ContextMenuItem; 8],
    pub item_count: usize,
    pub hover_index: i32,
}

#[derive(Clone, Copy)]
pub struct ContextMenuItem {
    pub label: &'static str,
    pub icon: &'static [u8],
    pub action: ContextAction,
    pub separator: bool,
}

#[derive(Clone, Copy)]
pub enum ContextAction {
    Open,
    OpenWithTerminal,
    RunAsAdmin,
    Rename,
    Delete,
    Properties,
    Restore, // For recycle bin
    EmptyRecycleBin,
    Cut,
    Copy,
    Paste,
    None,
}

// Taskbar
pub struct Taskbar {
    pub start_menu_open: bool,
    pub start_hover: bool,
    pub search_hover: bool,
    pub taskbar_apps: [TaskbarApp; 16],
    pub app_count: usize,
    pub clock_time: [u8; 9], // "HH:MM:SS\0"
    pub system_tray: [SystemTrayIcon; 8],
    pub tray_count: usize,
}

pub struct TaskbarApp {
    pub window_index: usize,
    pub icon: &'static [u8],
    pub label: &'static str,
    pub active: bool,
    pub hover: bool,
}

pub struct SystemTrayIcon {
    pub icon: &'static [u8],
    pub tooltip: &'static str,
}

// Desktop manager
pub struct Desktop {
    pub framebuffer: Framebuffer,
    pub icons: [DesktopIcon; 32],
    pub icon_count: usize,
    pub windows: [Window; 16],
    pub window_count: usize,
    pub focused_window: i32, // -1 = none
    pub context_menu: ContextMenu,
    pub taskbar: Taskbar,
    pub wallpaper: &'static [u8], // Background image
    pub mouse: MouseState,
    pub last_mouse: MouseState,
    pub drag_start_x: u32,
    pub drag_start_y: u32,
    pub is_dragging: bool,
    pub selection_rect: Option<(u32, u32, u32, u32)>, // x, y, w, h
    pub rename_dialog: Option<RenameDialog>,
    pub admin_dialog: Option<<AdminDialog>,
    pub shutdown_dialog: bool,
    pub needs_redraw: bool,
}

pub struct RenameDialog {
    pub x: u32,
    pub y: u32,
    pub target_icon: usize,
    pub new_name: [u8; 128],
    pub name_len: usize,
    pub cursor_pos: usize,
}

pub struct AdminDialog {
    pub x: u32,
    pub y: u32,
    pub target_icon: usize,
    pub password: [u8; 64],
    pub pass_len: usize,
    pub cursor_pos: usize,
    pub show_password: bool,
}

// AMD XDNA / Accelerator state
pub struct AmdXdnaState {
    pub enabled: bool,
    pub device_count: u8,
    pub devices: [AmdXdnaDevice; 4],
}

pub struct AmdXdnaDevice {
    pub name: &'static str,
    pub type: AmdXdnaType,
    pub memory_mb: u32,
    pub clock_mhz: u32,
    pub temperature: u8,
    pub utilization: u8,
}

pub enum AmdXdnaType {
    Npu,  // Neural Processing Unit
    Gpu,
    Dsp,
}

// Global desktop instance
static mut DESKTOP: Option<<Desktop> = None;
static mut AMD_XDNA: AmdXdnaState = AmdXdnaState {
    enabled: false,
    device_count: 0,
    devices: [AmdXdnaDevice {
        name: "AMD Ryzen AI",
        type: AmdXdnaType::Npu,
        memory_mb: 4096,
        clock_mhz: 1500,
        temperature: 45,
        utilization: 0,
    }; 4],
};

// Icon bitmap data (simplified 48x48 icons as RGBA)
// Terminal icon - black screen with >_ prompt
static TERMINAL_ICON: [u8; 48*48*4] = [
    // Generated pixel data - black bg with green >_
    0x00, 0x00, 0x00, 0xFF, // ... (simplified, actual would be full 48x48)
];

// Chrome browser icon - circular with red, yellow, green, blue colors
static CHROME_ICON: [u8; 48*48*4] = [
    0xFF, 0x00, 0x00, 0xFF, // Red
    0xFF, 0xFF, 0x00, 0xFF, // Yellow
    0x00, 0xFF, 0x00, 0xFF, // Green
    0x00, 0x00, 0xFF, 0xFF, // Blue
    // ... full 48x48 circular icon
];

// Store icon - shopping bag
static STORE_ICON: [u8; 48*48*4] = [
    0x00, 0x78, 0xD4, 0xFF, // Microsoft blue
    // ... shopping bag shape
];

// File Manager icon - folder
static FOLDER_ICON: [u8; 48*48*4] = [
    0xFF, 0xD7, 0x00, 0xFF, // Yellow folder
    // ... folder shape
];

// Recycle Bin icon - trash can
static RECYCLE_ICON: [u8; 48*48*4] = [
    0x80, 0x80, 0x80, 0xFF, // Gray trash
    // ... trash can shape
];

// Settings icon - gear
static SETTINGS_ICON: [u8; 48*48*4] = [
    0x80, 0x80, 0x80, 0xFF, // Gray gear
    // ... gear shape
];

// Game icon - controller
static GAME_ICON: [u8; 48*48*4] = [
    0xFF, 0x00, 0xFF, 0xFF, // Purple controller
    // ... gamepad shape
];

// AMD XDNA icon - chip
static AMD_XDNA_ICON: [u8; 48*48*4] = [
    0xED, 0x1C, 0x24, 0xFF, // AMD red
    // ... chip shape
];

// Accelerator icon - lightning bolt
static ACCEL_ICON: [u8; 48*48*4] = [
    0x00, 0xFF, 0x00, 0xFF, // Green lightning
    // ... lightning shape
];

impl Desktop {
    pub fn init(fb: Framebuffer) -> &'static mut Desktop {
        let desktop = unsafe {
            DESKTOP = Some(Desktop {
                framebuffer: fb,
                icons: [DesktopIcon {
                    x: 0, y: 0, width: ICON_SIZE, height: ICON_SIZE,
                    label: "", app_type: AppType::Custom(""),
                    icon_data: &[], selected: false, double_click_timer: 0,
                }; 32],
                icon_count: 0,
                windows: [Window {
                    x: 100, y: 100, width: 800, height: 600,
                    title: "", app_type: AppType::Custom(""),
                    minimized: false, maximized: false, focused: false,
                    content: WindowContent::Empty,
                    close_hover: false, minimize_hover: false, maximize_hover: false,
                    drag_offset_x: 0, drag_offset_y: 0, is_dragging: false,
                }; 16],
                window_count: 0,
                focused_window: -1,
                context_menu: ContextMenu {
                    visible: false, x: 0, y: 0,
                    target_icon: None, target_file: None,
                    items: [ContextMenuItem {
                        label: "", icon: &[], action: ContextAction::None, separator: false,
                    }; 8],
                    item_count: 0, hover_index: -1,
                },
                taskbar: Taskbar {
                    start_menu_open: false, start_hover: false, search_hover: false,
                    taskbar_apps: [TaskbarApp {
                        window_index: 0, icon: &[], label: "", active: false, hover: false,
                    }; 16],
                    app_count: 0,
                    clock_time: [0; 9],
                    system_tray: [SystemTrayIcon {
                        icon: &[], tooltip: "",
                    }; 8],
                    tray_count: 0,
                },
                wallpaper: &[], // Default solid color
                mouse: MouseState::default(),
                last_mouse: MouseState::default(),
                drag_start_x: 0,
                drag_start_y: 0,
                is_dragging: false,
                selection_rect: None,
                rename_dialog: None,
                admin_dialog: None,
                shutdown_dialog: false,
                needs_redraw: true,
            });
            DESKTOP.as_mut().unwrap()
        };
        
        desktop.setup_default_icons();
        desktop.setup_taskbar();
        desktop.init_amd_xdna();
        
        desktop
    }
    
    fn setup_default_icons(&mut self) {
        // Row 1
        self.add_icon(20, 20, "Terminal", AppType::Terminal, &TERMINAL_ICON);
        self.add_icon(20, 120, "Chrome", AppType::Browser, &CHROME_ICON);
        self.add_icon(20, 220, "Store", AppType::Store, &STORE_ICON);
        self.add_icon(20, 320, "Files", AppType::FileManager, &FOLDER_ICON);
        
        // Row 2
        self.add_icon(120, 20, "Recycle Bin", AppType::RecycleBin, &RECYCLE_ICON);
        self.add_icon(120, 120, "Settings", AppType::Settings, &SETTINGS_ICON);
        self.add_icon(120, 220, "Games", AppType::GameLauncher, &GAME_ICON);
        self.add_icon(120, 320, "AMD XDNA", AppType::Custom("AMD XDNA"), &AMD_XDNA_ICON);
        
        // Row 3
        self.add_icon(220, 20, "Accelerator", AppType::Custom("Accel"), &ACCEL_ICON);
    }
    
    fn add_icon(&mut self, x: u32, y: u32, label: &'static str, app_type: AppType, icon: &'static [u8]) {
        if self.icon_count >= 32 { return; }
        
        let idx = self.icon_count;
        self.icons[idx] = DesktopIcon {
            x, y, width: ICON_SIZE, height: ICON_SIZE,
            label, app_type, icon_data: icon,
            selected: false, double_click_timer: 0,
        };
        self.icon_count += 1;
    }
    
    fn setup_taskbar(&mut self) {
        // Add system tray icons
        self.taskbar.system_tray[0] = SystemTrayIcon {
            icon: &[], // Network icon
            tooltip: "Network Connected",
        };
        self.taskbar.system_tray[1] = SystemTrayIcon {
            icon: &[], // Volume icon
            tooltip: "Volume: 100%",
        };
        self.taskbar.system_tray[2] = SystemTrayIcon {
            icon: &[], // Battery icon
            tooltip: "Battery: 85%",
        };
        self.taskbar.tray_count = 3;
    }
    
    fn init_amd_xdna(&mut self) {
        unsafe {
            AMD_XDNA.enabled = true;
            AMD_XDNA.device_count = 1;
            AMD_XDNA.devices[0] = AmdXdnaDevice {
                name: "AMD Ryzen AI 9 HX 370",
                type: AmdXdnaType::Npu,
                memory_mb: 32768,
                clock_mhz: 2000,
                temperature: 42,
                utilization: 15,
            };
        }
    }
    
    // Main render loop
    pub fn render(&mut self) {
        if !self.needs_redraw { return; }
        
        // Clear desktop background
        self.framebuffer.fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TASKBAR_HEIGHT, DESKTOP_BG);
        
        // Draw wallpaper if available, else gradient
        if self.wallpaper.is_empty() {
            self.draw_gradient_background();
        } else {
            // Draw wallpaper image
        }
        
        // Draw desktop icons
        self.draw_icons();
        
        // Draw selection rectangle
        if let Some((x, y, w, h)) = self.selection_rect {
            self.draw_selection_rect(x, y, w, h);
        }
        
        // Draw windows (back to front)
        self.draw_windows();
        
        // Draw context menu
        if self.context_menu.visible {
            self.draw_context_menu();
        }
        
        // Draw dialogs
        if let Some(ref dialog) = self.rename_dialog {
            self.draw_rename_dialog(dialog);
        }
        if let Some(ref dialog) = self.admin_dialog {
            self.draw_admin_dialog(dialog);
        }
        if self.shutdown_dialog {
            self.draw_shutdown_dialog();
        }
        
        // Draw taskbar
        self.draw_taskbar();
        
        // Draw mouse cursor
        self.draw_mouse_cursor();
        
        self.needs_redraw = false;
    }
    
    fn draw_gradient_background(&mut self) {
        // Simple vertical gradient from dark blue to dark purple
        for y in 0..SCREEN_HEIGHT - TASKBAR_HEIGHT {
            let t = y as f32 / (SCREEN_HEIGHT - TASKBAR_HEIGHT) as f32;
            let r = (30.0 * (1.0 - t) + 20.0 * t) as u8;
            let g = (30.0 * (1.0 - t) + 10.0 * t) as u8;
            let b = (60.0 * (1.0 - t) + 40.0 * t) as u8;
            self.framebuffer.draw_hline(0, y, SCREEN_WIDTH, Color(r, g, b));
        }
    }
    
    fn draw_icons(&mut self) {
        for i in 0..self.icon_count {
            let icon = &self.icons[i];
            
            // Draw selection highlight
            if icon.selected {
                self.framebuffer.fill_rect(
                    icon.x - 4, icon.y - 4,
                    icon.width + 8, icon.height + ICON_TEXT_HEIGHT + 8,
                    ICON_HIGHLIGHT
                );
            }
            
            // Draw icon image (48x48)
            self.draw_icon_image(icon.x, icon.y, icon.icon_data);
            
            // Draw label
            let label_y = icon.y + icon.height + 4;
            self.framebuffer.draw_text_centered(
                icon.x, label_y, icon.width, ICON_TEXT_HEIGHT,
                icon.label, ICON_TEXT_COLOR, 12
            );
        }
    }
    
    fn draw_icon_image(&mut self, x: u32, y: u32, data: &[u8]) {
        // Draw 48x48 RGBA icon
        if data.len() < 48*48*4 { return; }
        
        for row in 0..48 {
            for col in 0..48 {
                let idx = ((row * 48) + col) * 4;
                let color = Color(data[idx], data[idx+1], data[idx+2]);
                let alpha = data[idx+3];
                if alpha > 128 {
                    self.framebuffer.put_pixel(x + col, y + row, color);
                }
            }
        }
    }
    
    fn draw_selection_rect(&mut self, x: u32, y: u32, w: u32, h: u32) {
        // Dashed rectangle
        let dash = 4;
        // Top edge
        let mut dx = x;
        while dx < x + w {
            let len = core::cmp::min(dash, x + w - dx);
            self.framebuffer.draw_hline(dx, y, len, Color(0x00, 0x78, 0xD4));
            dx += dash * 2;
        }
        // Bottom edge
        dx = x;
        while dx < x + w {
            let len = core::cmp::min(dash, x + w - dx);
            self.framebuffer.draw_hline(dx, y + h, len, Color(0x00, 0x78, 0xD4));
            dx += dash * 2;
        }
        // Left edge
        let mut dy = y;
        while dy < y + h {
            let len = core::cmp::min(dash, y + h - dy);
            self.framebuffer.draw_vline(x, dy, len, Color(0x00, 0x78, 0xD4));
            dy += dash * 2;
        }
        // Right edge
        dy = y;
        while dy < y + h {
            let len = core::cmp::min(dash, y + h - dy);
            self.framebuffer.draw_vline(x + w, dy, len, Color(0x00, 0x78, 0xD4));
            dy += dash * 2;
        }
    }
    
    fn draw_windows(&mut self) {
        // Draw from back to front (skip minimized)
        for i in 0..self.window_count {
            if self.windows[i].minimized { continue; }
            self.draw_window(i);
        }
        
        // Draw focused window last (on top)
        if self.focused_window >= 0 {
            let fw = self.focused_window as usize;
            if fw < self.window_count && !self.windows[fw].minimized {
                self.draw_window(fw);
            }
        }
    }
    
    fn draw_window(&mut self, idx: usize) {
        let win = &self.windows[idx];
        
        // Window shadow
        self.framebuffer.fill_rect(win.x + 4, win.y + 4, win.width, win.height, Color(0x00, 0x00, 0x00));
        
        // Window background
        self.framebuffer.fill_rect(win.x, win.y, win.width, win.height, WINDOW_BG);
        
        // Title bar
        let title_color = if win.focused { WINDOW_TITLE_BAR } else { Color(0x80, 0x80, 0x80) };
        self.framebuffer.fill_rect(win.x, win.y, win.width, 30, title_color);
        
        // Title text
        self.framebuffer.draw_text(win.x + 8, win.y + 8, win.title, Color(0xFF, 0xFF, 0xFF), 14);
        
        // Window controls (right side)
        let btn_y = win.y + 5;
        
        // Minimize button
        let min_color = if win.minimize_hover { BUTTON_HOVER } else { BUTTON_BG };
        self.framebuffer.fill_rect(win.x + win.width - 90, btn_y, 25, 20, min_color);
        self.framebuffer.draw_text(win.x + win.width - 82, btn_y + 4, "_", Color(0x00, 0x00, 0x00), 14);
        
        // Maximize button
        let max_color = if win.maximize_hover { BUTTON_HOVER } else { BUTTON_BG };
        self.framebuffer.fill_rect(win.x + win.width - 60, btn_y, 25, 20, max_color);
        self.framebuffer.draw_text(win.x + win.width - 55, btn_y + 4, "□", Color(0x00, 0x00, 0x00), 14);
        
        // Close button
        let close_color = if win.close_hover { Color(0xFF, 0x00, 0x00) } else { Color(0xE0, 0x00, 0x00) };
        self.framebuffer.fill_rect(win.x + win.width - 30, btn_y, 25, 20, close_color);
        self.framebuffer.draw_text(win.x + win.width - 22, btn_y + 4, "×", Color(0xFF, 0xFF, 0xFF), 14);
        
        // Content area
        match &mut self.windows[idx].content {
            WindowContent::Terminal(term) => self.draw_terminal_content(win, term),
            WindowContent::Browser(browser) => self.draw_browser_content(win, browser),
            WindowContent::Store(store) => self.draw_store_content(win, store),
            WindowContent::FileManager(fm) => self.draw_file_manager_content(win, fm),
            WindowContent::RecycleBin(rb) => self.draw_recycle_bin_content(win, rb),
            WindowContent::Properties(props) => self.draw_properties_content(win, props),
            WindowContent::Empty => {},
        }
    }
    
    fn draw_terminal_content(&mut self, win: &Window, term: &mut Terminal) {
        // Terminal background
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, win.height - 30, Color(0x1E, 0x1E, 0x1E));
        
        // Terminal text area
        let text_x = win.x + 4;
        let text_y = win.y + 34;
        let text_w = win.width - 8;
        let text_h = win.height - 38;
        
        // Draw terminal content (simplified - actual would render buffer)
        self.framebuffer.draw_text(text_x, text_y, "root@myos:~# ", Color(0x00, 0xFF, 0x00), 14);
        
        // Cursor blink
        // TODO: Blink cursor at current position
    }
    
    fn draw_browser_content(&mut self, win: &Window, browser: &mut BrowserState) {
        // Address bar
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, 40, Color(0xF0, 0xF0, 0xF0));
        
        // Back button
        self.framebuffer.fill_rect(win.x + 5, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + 12, win.y + 42, "←", Color(0x00, 0x00, 0x00), 16);
        
        // Forward button
        self.framebuffer.fill_rect(win.x + 40, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + 47, win.y + 42, "→", Color(0x00, 0x00, 0x00), 16);
        
        // Refresh button
        self.framebuffer.fill_rect(win.x + 75, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + 82, win.y + 42, "↻", Color(0x00, 0x00, 0x00), 16);
        
        // URL bar
        let url_bar_color = if browser.address_bar_focused { Color(0xFF, 0xFF, 0xFF) } else { Color(0xFF, 0xFF, 0xFF) };
        self.framebuffer.fill_rect(win.x + 110, win.y + 35, win.width - 220, 30, url_bar_color);
        self.framebuffer.draw_rect(win.x + 110, win.y + 35, win.width - 220, 30, Color(0xC0, 0xC0, 0xC0));
        
        // URL text
        let url = core::str::from_utf8(&browser.url[..browser.url_len]).unwrap_or("about:blank");
        self.framebuffer.draw_text(win.x + 115, win.y + 42, url, Color(0x00, 0x00, 0x00), 12);
        
        // Chrome menu (three dots)
        self.framebuffer.fill_rect(win.x + win.width - 40, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + win.width - 30, win.y + 42, "⋮", Color(0x00, 0x00, 0x00), 16);
        
        // Page content area
        self.framebuffer.fill_rect(win.x, win.y + 70, win.width, win.height - 70, Color(0xFF, 0xFF, 0xFF));
        
        if browser.loading {
            // Loading indicator
            self.framebuffer.draw_text(
                win.x + win.width/2 - 50, win.y + win.height/2,
                "Loading...", Color(0x80, 0x80, 0x80), 16
            );
        } else {
            // Simple HTML rendering (very basic)
            self.render_simple_html(win, browser);
        }
    }
    
    fn render_simple_html(&mut self, win: &Window, browser: &BrowserState) {
        let content = browser.page_content;
        let mut y = win.y + 80 + browser.scroll_y;
        let x = win.x + 10;
        let max_y = win.y + win.height - 10;
        
        // Very basic HTML parser - just text for now
        let lines = content.lines();
        for line in lines {
            if y > max_y { break; }
            if y > win.y + 70 {
                self.framebuffer.draw_text(x, y, line, Color(0x00, 0x00, 0x00), 14);
            }
            y += 20;
        }
    }
    
    fn draw_store_content(&mut self, win: &Window, store: &mut StoreState) {
        // Store header
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, 60, Color(0x00, 0x78, 0xD4));
        self.framebuffer.draw_text(win.x + 10, win.y + 40, "MyOS Store", Color(0xFF, 0xFF, 0xFF), 24);
        
        // Search bar
        self.framebuffer.fill_rect(win.x + 200, win.y + 40, win.width - 400, 30, Color(0xFF, 0xFF, 0xFF));
        self.framebuffer.draw_rect(win.x + 200, win.y + 40, win.width - 400, 30, Color(0xC0, 0xC0, 0xC0));
        
        let search = core::str::from_utf8(&store.search_query[..store.search_len]).unwrap_or("");
        self.framebuffer.draw_text(win.x + 205, win.y + 48, search, Color(0x80, 0x80, 0x80), 12);
        
        // Categories
        let cat_y = win.y + 100;
        for i in 0..store.categories.len() {
            let cat_x = win.x + 10 + (i as u32 * 120);
            let cat_color = if i == store.selected_category { Color(0x00, 0x78, 0xD4) } else { Color(0xE0, 0xE0, 0xE0) };
            self.framebuffer.fill_rect(cat_x, cat_y, 110, 30, cat_color);
            self.framebuffer.draw_text(cat_x + 5, cat_y + 8, store.categories[i].name, Color(0x00, 0x00, 0x00), 12);
        }
        
        // Apps grid
        let grid_y = cat_y + 40;
        let mut app_idx = 0;
        for row in 0..4 {
            for col in 0..4 {
                if app_idx >= store.app_count { break; }
                
                let app = &store.apps[app_idx];
                if app.category == store.selected_category || store.selected_category == 0 {
                    let app_x = win.x + 10 + (col as u32 * 240);
                    let app_y = grid_y + (row as u32 * 100) - store.scroll_y;
                    
                    if app_y > win.y + 130 && app_y < win.y + win.height - 10 {
                        // App card
                        self.framebuffer.fill_rect(app_x, app_y, 230, 90, Color(0xF8, 0xF8, 0xF8));
                        self.framebuffer.draw_rect(app_x, app_y, 230, 90, Color(0xE0, 0xE0, 0xE0));
                        
                        // App icon
                        self.draw_icon_image(app_x + 5, app_y + 5, app.icon);
                        
                        // App name
                        self.framebuffer.draw_text(app_x + 60, app_y + 10, app.name, Color(0x00, 0x00, 0x00), 14);
                        
                        // Description (truncated)
                        self.framebuffer.draw_text(app_x + 60, app_y + 30, app.description, Color(0x80, 0x80, 0x80), 11);
                        
                        // Rating stars
                        let stars = app.rating / 10;
                        let mut star_str = [b'★'; 5];
                        for s in stars..5 { star_str[s as usize] = b'☆'; }
                        self.framebuffer.draw_text(app_x + 60, app_y + 50, core::str::from_utf8(&star_str).unwrap_or(""), Color(0xFF, 0xA5, 0x00), 12);
                        
                        // Download count
                        let dl_text = format_u32(app.downloads);
                        self.framebuffer.draw_text(app_x + 60, app_y + 70, &dl_text, Color(0x80, 0x80, 0x80), 10);
                        
                        // Install button or Installed label
                        if app.installed {
                            self.framebuffer.fill_rect(app_x + 160, app_y + 60, 60, 25, Color(0xE0, 0xE0, 0xE0));
                            self.framebuffer.draw_text(app_x + 165, app_y + 67, "Installed", Color(0x80, 0x80, 0x80), 10);
                        } else if store.installing_app == Some(app_idx) {
                            // Progress bar
                            self.framebuffer.fill_rect(app_x + 160, app_y + 60, 60, 25, Color(0xE0, 0xE0, 0xE0));
                            let progress_width = (store.install_progress as u32 * 60) / 100;
                            self.framebuffer.fill_rect(app_x + 160, app_y + 60, progress_width, 25, Color(0x00, 0x78, 0xD4));
                            self.framebuffer.draw_text(app_x + 165, app_y + 67, "Installing...", Color(0xFF, 0xFF, 0xFF), 9);
                        } else {
                            self.framebuffer.fill_rect(app_x + 160, app_y + 60, 60, 25, Color(0x00, 0x78, 0xD4));
                            self.framebuffer.draw_text(app_x + 170, app_y + 67, "Install", Color(0xFF, 0xFF, 0xFF), 11);
                        }
                    }
                }
                app_idx += 1;
            }
        }
    }
    
    fn draw_file_manager_content(&mut self, win: &Window, fm: &mut FileManagerState) {
        // Toolbar
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, 40, Color(0xF0, 0xF0, 0xF0));
        
        // Back/Forward buttons
        self.framebuffer.fill_rect(win.x + 5, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + 12, win.y + 42, "←", Color(0x00, 0x00, 0x00), 16);
        
        // Path bar
        let path = core::str::from_utf8(&fm.current_path[..fm.path_len]).unwrap_or("/");
        self.framebuffer.fill_rect(win.x + 40, win.y + 35, win.width - 80, 30, Color(0xFF, 0xFF, 0xFF));
        self.framebuffer.draw_rect(win.x + 40, win.y + 35, win.width - 80, 30, Color(0xC0, 0xC0, 0xC0));
        self.framebuffer.draw_text(win.x + 45, win.y + 42, path, Color(0x00, 0x00, 0x00), 12);
        
        // View mode buttons
        self.framebuffer.fill_rect(win.x + win.width - 35, win.y + 35, 30, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + win.width - 28, win.y + 42, "☰", Color(0x00, 0x00, 0x00), 16);
        
        // File list
        let list_y = win.y + 75;
        for i in 0..fm.file_count {
            let item_y = list_y + (i as u32 * 30) - fm.scroll_y;
            if item_y < win.y + 75 || item_y > win.y + win.height - 10 { continue; }
            
            let bg_color = if fm.selected_file == i as i32 { Color(0x00, 0x78, 0xD4) } else { Color(0xFF, 0xFF, 0xFF) };
            self.framebuffer.fill_rect(win.x + 5, item_y, win.width - 10, 28, bg_color);
            
            let file = &fm.files[i];
            let name = core::str::from_utf8(&file.name[..file.name_len]).unwrap_or("?");
            
            // Icon
            self.draw_icon_image(win.x + 10, item_y + 2, file.icon);
            
            // Name
            let text_color = if fm.selected_file == i as i32 { Color(0xFF, 0xFF, 0xFF) } else { Color(0x00, 0x00, 0x00) };
            self.framebuffer.draw_text(win.x + 45, item_y + 8, name, text_color, 12);
            
            // Size
            if !file.is_dir {
                let size_str = format_size(file.size);
                self.framebuffer.draw_text(win.x + win.width - 150, item_y + 8, &size_str, text_color, 11);
            }
        }
    }
    
    fn draw_recycle_bin_content(&mut self, win: &Window, rb: &mut RecycleBinState) {
        // Header
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, 40, Color(0xF0, 0xF0, 0xF0));
        self.framebuffer.draw_text(win.x + 10, win.y + 40, "Recycle Bin", Color(0x00, 0x00, 0x00), 18);
        
        // Empty button
        self.framebuffer.fill_rect(win.x + win.width - 120, win.y + 35, 100, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(win.x + win.width - 110, win.y + 42, "Empty Bin", Color(0x00, 0x00, 0x00), 12);
        
        // Deleted files list
        let list_y = win.y + 80;
        for i in 0..rb.file_count {
            let item_y = list_y + (i as u32 * 30) - rb.scroll_y;
            if item_y < win.y + 80 || item_y > win.y + win.height - 10 { continue; }
            
            let file = &rb.deleted_files[i];
            let name = core::str::from_utf8(&file.name[..file.name_len]).unwrap_or("?");
            
            let bg_color = if rb.selected_file == i as i32 { Color(0x00, 0x78, 0xD4) } else { Color(0xFF, 0xFF, 0xFF) };
            self.framebuffer.fill_rect(win.x + 5, item_y, win.width - 10, 28, bg_color);
            
            // Icon
            self.draw_icon_image(win.x + 10, item_y + 2, file.icon);
            
            // Name
            let text_color = if rb.selected_file == i as i32 { Color(0xFF, 0xFF, 0xFF) } else { Color(0x00, 0x00, 0x00) };
            self.framebuffer.draw_text(win.x + 45, item_y + 8, name, text_color, 12);
            
            // Original path
            let path = core::str::from_utf8(&file.original_path[..file.path_len]).unwrap_or("?");
            self.framebuffer.draw_text(win.x + 200, item_y + 8, path, Color(0x80, 0x80, 0x80), 10);
            
            // Size
            let size_str = format_size(file.size);
            self.framebuffer.draw_text(win.x + win.width - 100, item_y + 8, &size_str, text_color, 11);
        }
    }
    
    fn draw_properties_content(&mut self, win: &Window, props: &mut PropertiesState) {
        // Title
        self.framebuffer.fill_rect(win.x, win.y + 30, win.width, 40, Color(0xF0, 0xF0, 0xF0));
        let name = core::str::from_utf8(&props.target_name[..props.name_len]).unwrap_or("Properties");
        self.framebuffer.draw_text(win.x + 10, win.y + 40, name, Color(0x00, 0x00, 0x00), 18);
        
        // Properties list
        let mut y = win.y + 80;
        let line_height = 25;
        
        // Type
        self.framebuffer.draw_text(win.x + 10, y, "Type:", Color(0x80, 0x80, 0x80), 12);
        let type_str = if props.is_directory { "Folder" } else { "File" };
        self.framebuffer.draw_text(win.x + 150, y, type_str, Color(0x00, 0x00, 0x00), 12);
        y += line_height;
        
        // Location
        self.framebuffer.draw_text(win.x + 10, y, "Location:", Color(0x80, 0x80, 0x80), 12);
        let path = core::str::from_utf8(&props.target_path[..props.path_len]).unwrap_or("/");
        self.framebuffer.draw_text(win.x + 150, y, path, Color(0x00, 0x00, 0x00), 12);
        y += line_height;
        
        // Size
        self.framebuffer.draw_text(win.x + 10, y, "Size:", Color(0x80, 0x80, 0x80), 12);
        let size_str = format_size(props.file_size);
        self.framebuffer.draw_text(win.x + 150, y, &size_str, Color(0x00, 0x00, 0x00), 12);
        y += line_height;
        
        // Owner
        self.framebuffer.draw_text(win.x + 10, y, "Owner:", Color(0x80, 0x80, 0x80), 12);
        self.framebuffer.draw_text(win.x + 150, y, props.owner, Color(0x00, 0x00, 0x00), 12);
        y += line_height;
        
        // Permissions
        self.framebuffer.draw_text(win.x + 10, y, "Permissions:", Color(0x80, 0x80, 0x80), 12);
        let perm_str = format_permissions(props.permissions);
        self.framebuffer.draw_text(win.x + 150, y, &perm_str, Color(0x00, 0x00, 0x00), 12);
        y += line_height;
        
        // Safety check
        self.framebuffer.draw_text(win.x + 10, y, "Safety:", Color(0x80, 0x80, 0x80), 12);
        let safe_str = if props.is_safe { "✓ Verified Safe" } else { "⚠ Unknown Source" };
        let safe_color = if props.is_safe { Color(0x00, 0x80, 0x00) } else { Color(0xFF, 0x80, 0x00) };
        self.framebuffer.draw_text(win.x + 150, y, safe_str, safe_color, 12);
        y += line_height + 10;
        
        // OK button
        let btn_x = win.x + win.width / 2 - 40;
        self.framebuffer.fill_rect(btn_x, y, 80, 30, Color(0x00, 0x78, 0xD4));
        self.framebuffer.draw_text(btn_x + 30, y + 8, "OK", Color(0xFF, 0xFF, 0xFF), 14);
    }
    
    fn draw_context_menu(&mut self) {
        let menu = &self.context_menu;
        if !menu.visible { return; }
        
        let menu_width = 200;
        let item_height = 28;
        let menu_height = menu.item_count as u32 * item_height + 10;
        
        // Menu background
        self.framebuffer.fill_rect(menu.x, menu.y, menu_width, menu_height, CONTEXT_MENU_BG);
        self.framebuffer.draw_rect(menu.x, menu.y, menu_width, menu_height, Color(0x80, 0x80, 0x80));
        
        // Menu items
        for i in 0..menu.item_count {
            let item_y = menu.y + 5 + (i as u32 * item_height);
            let bg_color = if menu.hover_index == i as i32 { CONTEXT_MENU_HOVER } else { CONTEXT_MENU_BG };
            
            if menu.items[i].separator {
                // Separator line
                self.framebuffer.draw_hline(menu.x + 10, item_y + item_height/2, menu_width - 20, Color(0x60, 0x60, 0x60));
            } else {
                self.framebuffer.fill_rect(menu.x + 2, item_y, menu_width - 4, item_height - 2, bg_color);
                
                // Icon
                if !menu.items[i].icon.is_empty() {
                    self.draw_icon_image(menu.x + 8, item_y + 2, menu.items[i].icon);
                }
                
                // Label
                let text_color = if menu.hover_index == i as i32 { Color(0xFF, 0xFF, 0xFF) } else { Color(0xE0, 0xE0, 0xE0) };
                self.framebuffer.draw_text(menu.x + 35, item_y + 6, menu.items[i].label, text_color, 12);
            }
        }
    }
    
    fn draw_rename_dialog(&mut self, dialog: &RenameDialog) {
        // Dim background
        self.framebuffer.fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color(0x00, 0x00, 0x00));
        // Semi-transparent overlay would be better but we use solid for now
        
        // Dialog box
        let dw = 400;
        let dh = 150;
        let dx = SCREEN_WIDTH/2 - dw/2;
        let dy = SCREEN_HEIGHT/2 - dh/2;
        
        self.framebuffer.fill_rect(dx, dy, dw, dh, Color(0xF0, 0xF0, 0xF0));
        self.framebuffer.draw_rect(dx, dy, dw, dh, Color(0x80, 0x80, 0x80));
        
        // Title
        self.framebuffer.draw_text(dx + 10, dy + 10, "Rename", Color(0x00, 0x00, 0x00), 16);
        
        // Current name
        let name = core::str::from_utf8(&dialog.new_name[..dialog.name_len]).unwrap_or("");
        self.framebuffer.fill_rect(dx + 10, dy + 40, dw - 20, 30, Color(0xFF, 0xFF, 0xFF));
        self.framebuffer.draw_rect(dx + 10, dy + 40, dw - 20, 30, Color(0xC0, 0xC0, 0xC0));
        self.framebuffer.draw_text(dx + 15, dy + 48, name, Color(0x00, 0x00, 0x00), 12);
        
        // Cursor
        let cursor_x = dx + 15 + (dialog.cursor_pos as u32 * 7);
        self.framebuffer.draw_vline(cursor_x, dy + 42, 26, Color(0x00, 0x00, 0x00));
        
        // Buttons
        self.framebuffer.fill_rect(dx + dw - 180, dy + dh - 40, 80, 30, Color(0x00, 0x78, 0xD4));
        self.framebuffer.draw_text(dx + dw - 160, dy + dh - 32, "OK", Color(0xFF, 0xFF, 0xFF), 14);
        
        self.framebuffer.fill_rect(dx + dw - 90, dy + dh - 40, 80, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(dx + dw - 75, dy + dh - 32, "Cancel", Color(0x00, 0x00, 0x00), 14);
    }
    
    fn draw_admin_dialog(&mut self, dialog: &AdminDialog) {
        let dw = 400;
        let dh = 200;
        let dx = SCREEN_WIDTH/2 - dw/2;
        let dy = SCREEN_HEIGHT/2 - dh/2;
        
        self.framebuffer.fill_rect(dx, dy, dw, dh, Color(0xF0, 0xF0, 0xF0));
        self.framebuffer.draw_rect(dx, dy, dw, dh, Color(0x80, 0x80, 0x80));
        
        // Shield icon (UAC style)
        self.framebuffer.fill_rect(dx + 10, dy + 10, 48, 48, Color(0x00, 0x78, 0xD4));
        self.framebuffer.draw_text(dx + 20, dy + 20, "🛡", Color(0xFF, 0xFF, 0xFF), 24);
        
        // Title
        self.framebuffer.draw_text(dx + 70, dy + 15, "Run as Administrator", Color(0x00, 0x00, 0x00), 16);
        self.framebuffer.draw_text(dx + 70, dy + 35, "Enter password to continue", Color(0x80, 0x80, 0x80), 12);
        
        // Password field
        let pass_display = if dialog.show_password {
            core::str::from_utf8(&dialog.password[..dialog.pass_len]).unwrap_or("")
        } else {
            // Show asterisks
            let mut stars = [b'*'; 64];
            let star_str = core::str::from_utf8(&stars[..dialog.pass_len]).unwrap_or("");
            star_str
        };
        
        self.framebuffer.fill_rect(dx + 70, dy + 60, dw - 80, 30, Color(0xFF, 0xFF, 0xFF));
        self.framebuffer.draw_rect(dx + 70, dy + 60, dw - 80, 30, Color(0xC0, 0xC0, 0xC0));
        self.framebuffer.draw_text(dx + 75, dy + 68, pass_display, Color(0x00, 0x00, 0x00), 12);
        
        // Show password checkbox
        self.framebuffer.draw_rect(dx + 70, dy + 100, 16, 16, Color(0x80, 0x80, 0x80));
        if dialog.show_password {
            self.framebuffer.fill_rect(dx + 73, dy + 103, 10, 10, Color(0x00, 0x78, 0xD4));
        }
        self.framebuffer.draw_text(dx + 92, dy + 100, "Show password", Color(0x00, 0x00, 0x00), 12);
        
        // Buttons
        self.framebuffer.fill_rect(dx + dw - 180, dy + dh - 40, 80, 30, Color(0x00, 0x78, 0xD4));
        self.framebuffer.draw_text(dx + dw - 160, dy + dh - 32, "Yes", Color(0xFF, 0xFF, 0xFF), 14);
        
        self.framebuffer.fill_rect(dx + dw - 90, dy + dh - 40, 80, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(dx + dw - 75, dy + dh - 32, "No", Color(0x00, 0x00, 0x00), 14);
    }
    
    fn draw_shutdown_dialog(&mut self) {
        let dw = 300;
        let dh = 200;
        let dx = SCREEN_WIDTH/2 - dw/2;
        let dy = SCREEN_HEIGHT/2 - dh/2;
        
        self.framebuffer.fill_rect(dx, dy, dw, dh, Color(0xF0, 0xF0, 0xF0));
        self.framebuffer.draw_rect(dx, dy, dw, dh, Color(0x80, 0x80, 0x80));
        
        self.framebuffer.draw_text(dx + 10, dy + 10, "Shut Down MyOS", Color(0x00, 0x00, 0x00), 16);
        
        // Options
        let options = ["Shut down", "Restart", "Sleep", "Hibernate", "Sign out"];
        for (i, opt) in options.iter().enumerate() {
            let oy = dy + 40 + (i as u32 * 30);
            self.framebuffer.fill_rect(dx + 10, oy, dw - 20, 28, Color(0xE0, 0xE0, 0xE0));
            self.framebuffer.draw_text(dx + 20, oy + 6, opt, Color(0x00, 0x00, 0x00), 12);
        }
        
        // Cancel
        self.framebuffer.fill_rect(dx + dw - 90, dy + dh - 40, 80, 30, Color(0xE0, 0xE0, 0xE0));
        self.framebuffer.draw_text(dx + dw - 75, dy + dh - 32, "Cancel", Color(0x00, 0x00, 0x00), 14);
    }
    
    fn draw_taskbar(&mut self) {
        let tb_y = SCREEN_HEIGHT - TASKBAR_HEIGHT;
        
        // Taskbar background
        self.framebuffer.fill_rect(0, tb_y, SCREEN_WIDTH, TASKBAR_HEIGHT, TASKBAR_BG);
        self.framebuffer.draw_hline(0, tb_y, SCREEN_WIDTH, Color(0x40, 0x40, 0x40));
        
        // Start button
        let start_color = if self.taskbar.start_hover { Color(0x40, 0x40, 0x40) } else { TASKBAR_BG };
        self.framebuffer.fill_rect(5, tb_y + 5, 80, 30, start_color);
        self.framebuffer.draw_text(15, tb_y + 12, "MyOS", Color(0x00, 0xFF, 0xFF), 14);
        
        // Start menu (if open)
        if self.taskbar.start_menu_open {
            self.draw_start_menu();
        }
        
        // Taskbar apps (center area)
        let mut app_x = 100;
        for i in 0..self.taskbar.app_count {
            let app = &self.taskbar.taskbar_apps[i];
            let app_color = if app.active { Color(0x40, 0x40, 0x40) } else if app.hover { Color(0x50, 0x50, 0x50) } else { TASKBAR_BG };
            
            self.framebuffer.fill_rect(app_x, tb_y + 5, 40, 30, app_color);
            if !app.icon.is_empty() {
                self.draw_icon_image(app_x + 4, tb_y + 7, app.icon);
            }
            
            app_x += 45;
        }
        
        // System tray (right side)
        let mut tray_x = SCREEN_WIDTH - 150;
        for i in 0..self.taskbar.tray_count {
            let tray = &self.taskbar.system_tray[i];
            if !tray.icon.is_empty() {
                self.draw_icon_image(tray_x, tb_y + 8, tray.icon);
            }
            tray_x += 25;
        }
        
        // Clock
        let clock = core::str::from_utf8(&self.taskbar.clock_time).unwrap_or("00:00:00");
        self.framebuffer.draw_text(SCREEN_WIDTH - 80, tb_y + 12, clock, Color(0xFF, 0xFF, 0xFF), 12);
    }
    
    fn draw_start_menu(&mut self) {
        let sm_x = 5;
        let sm_y = SCREEN_HEIGHT - TASKBAR_HEIGHT - 400;
        let sm_w = 300;
        let sm_h = 400;
        
        self.framebuffer.fill_rect(sm_x, sm_y, sm_w, sm_h, Color(0x2D, 0x2D, 0x2D));
        self.framebuffer.draw_rect(sm_x, sm_y, sm_w, sm_h, Color(0x40, 0x40, 0x40));
        
        // User section
        self.framebuffer.fill_rect(sm_x, sm_y, sm_w, 60, Color(0x1E, 0x1E, 0x1E));
        self.framebuffer.draw_text(sm_x + 10, sm_y + 20, "root", Color(0xFF, 0xFF, 0xFF), 14);
        
        // App list
        let apps = [
            ("Terminal", &TERMINAL_ICON),
            ("Chrome", &CHROME_ICON),
            ("Store", &STORE_ICON),
            ("Files", &FOLDER_ICON),
            ("Settings", &SETTINGS_ICON),
            ("Recycle Bin", &RECYCLE_ICON),
        ];
        
        for (i, (name, icon)) in apps.iter().enumerate() {
            let ay = sm_y + 70 + (i as u32 * 40);
            self.framebuffer.fill_rect(sm_x + 5, ay, sm_w - 10, 38, Color(0x2D, 0x2D, 0x2D));
            self.draw_icon_image(sm_x + 10, ay + 5, icon);
            self.framebuffer.draw_text(sm_x + 50, ay + 12, name, Color(0xFF, 0xFF, 0xFF), 12);
        }
        
        // Power button
        self.framebuffer.fill_rect(sm_x + 5, sm_y + sm_h - 45, sm_w - 10, 40, Color(0x1E, 0x1E, 0x1E));
        self.framebuffer.draw_text(sm_x + 15, sm_y + sm_h - 35, "⚡ Power", Color(0xFF, 0xFF, 0xFF), 12);
    }
    
    fn draw_mouse_cursor(&mut self) {
        let mx = self.mouse.x;
        let my = self.mouse.y;
        
        // Simple arrow cursor
        let cursor_color = Color(0xFF, 0xFF, 0xFF);
        let border_color = Color(0x00, 0x00, 0x00);
        
        // Arrow shape
        for i in 0..16 {
            self.framebuffer.put_pixel(mx + i, my + i, cursor_color);
            self.framebuffer.put_pixel(mx + i + 1, my + i, border_color);
            self.framebuffer.put_pixel(mx + i, my + i + 1, border_color);
        }
        // Arrow head
        for i in 0..8 {
            self.framebuffer.put_pixel(mx + i, my + 16 - i, cursor_color);
            self.framebuffer.put_pixel(mx + 16 - i, my + i, cursor_color);
        }
    }
    
    // ==================== INPUT HANDLING ====================
    
    pub fn handle_mouse(&mut self, mouse: MouseState) {
        let old_mouse = self.mouse;
        self.mouse = mouse;
        self.last_mouse = old_mouse;
        
        let mx = mouse.x;
        let my = mouse.y;
        let left_pressed = mouse.left && !old_mouse.left;
        let left_released = !mouse.left && old_mouse.left;
        let right_pressed = mouse.right && !old_mouse.right;
        let middle_pressed = mouse.middle && !old_mouse.middle;
        
        // Handle dialogs first (modal)
        if self.rename_dialog.is_some() {
            self.handle_rename_dialog(mx, my, left_pressed);
            return;
        }
        if self.admin_dialog.is_some() {
            self.handle_admin_dialog(mx, my, left_pressed);
            return;
        }
        if self.shutdown_dialog {
            self.handle_shutdown_dialog(mx, my, left_pressed);
            return;
        }
        
        // Context menu
        if self.context_menu.visible {
            if left_pressed || right_pressed {
                if self.is_point_in_context_menu(mx, my) {
                    self.handle_context_menu_click(mx, my);
                } else {
                    self.context_menu.visible = false;
                }
            } else {
                self.update_context_menu_hover(mx, my);
            }
            self.needs_redraw = true;
            return;
        }
        
        // Check taskbar
        if my >= SCREEN_HEIGHT - TASKBAR_HEIGHT {
            self.handle_taskbar_click(mx, my, left_pressed, right_pressed);
            return;
        }
        
        // Check windows
        if self.focused_window >= 0 {
            let fw = self.focused_window as usize;
            if fw < self.window_count && self.is_point_in_window(mx, my, fw) {
                self.handle_window_click(fw, mx, my, left_pressed, right_pressed, middle_pressed);
                return;
            }
        }
        
        // Check desktop icons
        if left_pressed {
            let clicked_icon = self.find_icon_at(mx, my);
            if let Some(idx) = clicked_icon {
                self.handle_icon_click(idx, mx, my);
            } else {
                // Start selection rectangle
                self.drag_start_x = mx;
                self.drag_start_y = my;
                self.is_dragging = true;
                self.selection_rect = Some((mx, my, 0, 0));
                
                // Deselect all
                for i in 0..self.icon_count {
                    self.icons[i].selected = false;
                }
            }
        } else if right_pressed {
            let clicked_icon = self.find_icon_at(mx, my);
            self.show_context_menu(mx, my, clicked_icon, None);
        } else if self.is_dragging && mouse.left {
            // Update selection rectangle
            let x1 = core::cmp::min(self.drag_start_x, mx);
            let y1 = core::cmp::min(self.drag_start_y, my);
            let w = if mx > self.drag_start_x { mx - self.drag_start_x } else { self.drag_start_x - mx };
            let h = if my > self.drag_start_y { my - self.drag_start_y } else { self.drag_start_y - my };
            self.selection_rect = Some((x1, y1, w, h));
            
            // Select icons in rectangle
            for i in 0..self.icon_count {
                let icon = &self.icons[i];
                let ix = icon.x;
                let iy = icon.y;
                let iw = icon.width;
                let ih = icon.height + ICON_TEXT_HEIGHT;
                
                let intersects = !(x1 + w < ix || ix + iw < x1 || y1 + h < iy || iy + ih < y1);
                self.icons[i].selected = intersects;
            }
        } else if left_released && self.is_dragging {
            self.is_dragging = false;
            self.selection_rect = None;
        }
        
        self.needs_redraw = true;
    }
    
    fn handle_icon_click(&mut self, idx: usize, mx: u32, my: u32) {
        let icon = &self.icons[idx];
        
        if icon.double_click_timer > 0 {
            // Double click - open app
            self.open_app(icon.app_type, icon.label);
            self.icons[idx].double_click_timer = 0;
        } else {
            // Single click - select
            for i in 0..self.icon_count {
                self.icons[i].selected = i == idx;
            }
            self.icons[idx].double_click_timer = 30; // 30 frames for double click
        }
    }
    
    fn show_context_menu(&mut self, x: u32, y: u32, icon_idx: Option<<usize>, file_idx: Option<<usize>) {
        let mut menu = ContextMenu {
            visible: true, x, y,
            target_icon: icon_idx,
            target_file: file_idx,
            items: [ContextMenuItem {
                label: "", icon: &[], action: ContextAction::None, separator: false,
            }; 8],
            item_count: 0,
            hover_index: -1,
        };
        
        if let Some(idx) = icon_idx {
            let app_type = self.icons[idx].app_type;
            
            // Different menu for different types
            match app_type {
                AppType::RecycleBin => {
                    menu.add_item("Open", &FOLDER_ICON, ContextAction::Open);
                    menu.add_item("Empty Recycle Bin", &RECYCLE_ICON, ContextAction::EmptyRecycleBin);
                    menu.add_separator();
                    menu.add_item("Properties", &SETTINGS_ICON, ContextAction::Properties);
                }
                _ => {
                    menu.add_item("Open", &FOLDER_ICON, ContextAction::Open);
                    menu.add_item("Open in Terminal", &TERMINAL_ICON, ContextAction::OpenWithTerminal);
                    menu.add_item("Run as Administrator", &SETTINGS_ICON, ContextAction::RunAsAdmin);
                    menu.add_separator();
                    menu.add_item("Rename", &FOLDER_ICON, ContextAction::Rename);
                    menu.add_item("Delete", &RECYCLE_ICON, ContextAction::Delete);
                    menu.add_separator();
                    menu.add_item("Properties", &SETTINGS_ICON, ContextAction::Properties);
                }
            }
        } else {
            // Desktop background context menu
            menu.add_item("View", &FOLDER_ICON, ContextAction::None);
            menu.add_item("Sort by", &FOLDER_ICON, ContextAction::None);
            menu.add_item("Refresh", &FOLDER_ICON, ContextAction::None);
            menu.add_separator();
            menu.add_item("New", &FOLDER_ICON, ContextAction::None);
            menu.add_separator();
            menu.add_item("Display settings", &SETTINGS_ICON, ContextAction::None);
            menu.add_item("Personalize", &SETTINGS_ICON, ContextAction::None);
        }
        
        self.context_menu = menu;
    }
    
    fn handle_context_menu_click(&mut self, mx: u32, my: u32) {
        let menu = &self.context_menu;
        let item_height = 28;
        let rel_y = my - menu.y - 5;
        let idx = (rel_y / item_height) as usize;
        
        if idx < menu.item_count && !menu.items[idx].separator {
            let action = menu.items[idx].action;
            let target = menu.target_icon;
            
            self.context_menu.visible = false;
            
            match action {
                ContextAction::Open => {
                    if let Some(t) = target {
                        self.open_app(self.icons[t].app_type, self.icons[t].label);
                    }
                }
                ContextAction::OpenWithTerminal => {
                    if let Some(t) = target {
                        self.open_app_with_terminal(self.icons[t].label);
                    }
                }
                ContextAction::RunAsAdmin => {
                    if let Some(t) = target {
                        self.show_admin_dialog(t);
                    }
                }
                ContextAction::Rename => {
                    if let Some(t) = target {
                        self.show_rename_dialog(t);
                    }
                }
                ContextAction::Delete => {
                    if let Some(t) = target {
                        self.move_to_recycle_bin(t);
                    }
                }
                ContextAction::Properties => {
                    if let Some(t) = target {
                        self.show_properties(t);
                    }
                }
                ContextAction::EmptyRecycleBin => {
                    self.empty_recycle_bin();
                }
                _ => {}
            }
        }
    }
    
    fn update_context_menu_hover(&mut self, mx: u32, my: u32) {
        let menu = &mut self.context_menu;
        if !self.is_point_in_context_menu(mx, my) {
            menu.hover_index = -1;
            return;
        }
        
        let item_height = 28;
        let rel_y = my - menu.y - 5;
        let idx = (rel_y / item_height) as i32;
        
        if idx >= 0 && (idx as usize) < menu.item_count && !menu.items[idx as usize].separator {
            menu.hover_index = idx;
        } else {
            menu.hover_index = -1;
        }
    }
    
    fn is_point_in_context_menu(&self, x: u32, y: u32) -> bool {
        let menu = &self.context_menu;
        x >= menu.x && x < menu.x + 200 &&
        y >= menu.y && y < menu.y + (menu.item_count as u32 * 28 + 10)
    }
    
    fn open_app(&mut self, app_type: AppType, label: &'static str) {
        match app_type {
            AppType::Terminal => {
                let mut term = Terminal::new();
                term.init();
                self.create_window(label, 800, 600, WindowContent::Terminal(term));
            }
            AppType::Browser => {
                let browser = BrowserState {
                    url: [0; 2048],
                    url_len: 0
                }
            }
        }
    }