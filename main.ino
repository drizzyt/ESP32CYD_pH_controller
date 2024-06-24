#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include <TFT_eSPI.h>  // Ensure you have the correct display driver
#include <WiFi.h>

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

// WiFi credentials
const char* ssid = "";
const char* password = "";// wifi must be hardcoded for now

// Firmware version
const char* firmwareVersion = "V_0.3.1"; 
/*CHANGELOG
-  Any firmware > 0.3.1 is running LVGL_9
- added wifi connectivity icon
- changed from delay to millis
*/
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static lv_obj_t * main_screen;
static lv_obj_t * wifi_icon;

static void update_wifi_icon() {
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
  } else {
    lv_label_set_text(wifi_icon, LV_SYMBOL_WARNING);
  }
}

static void event_handler_home(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    LV_LOG_USER("Home button clicked");
    lv_scr_load(main_screen);
  }
}

static void display_wifi_networks(lv_obj_t * parent) {
  // Scan for WiFi networks
  int num_networks = WiFi.scanNetworks();

  // Create a table
  lv_obj_t * table = lv_table_create(parent);
  lv_obj_set_size(table, SCREEN_WIDTH, SCREEN_HEIGHT - 70); // Adjust the size as needed
  lv_obj_align(table, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Set the table's column descriptors
  lv_table_set_col_cnt(table, 2);
  lv_table_set_col_width(table, 0, SCREEN_WIDTH / 2); // Adjust the column width as needed
  lv_table_set_col_width(table, 1, SCREEN_WIDTH / 2); // Adjust the column width as needed

  // Set the table's header
  lv_table_set_cell_value(table, 0, 0, "SSID");
  lv_table_set_cell_value(table, 0, 1, "Signal Strength (dBm)");

  // Populate the table with WiFi network data
  for (int i = 0; i < num_networks; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    String rssi_str = String(rssi) + " dBm";

    lv_table_set_cell_value(table, i + 1, 0, ssid.c_str());
    lv_table_set_cell_value(table, i + 1, 1, rssi_str.c_str());
  }
}

static void event_handler_wifi(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    LV_LOG_USER("WiFi button clicked");

    // Create a new screen for WiFi
    lv_obj_t * wifi_screen = lv_obj_create(NULL);

    // Create a home button
    lv_obj_t * home_btn = lv_btn_create(wifi_screen);
    lv_obj_add_event_cb(home_btn, event_handler_home, LV_EVENT_ALL, NULL);
    lv_obj_set_size(home_btn, 50, 50);
    lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t * home_label = lv_label_create(home_btn);
    lv_label_set_text(home_label, LV_SYMBOL_HOME);
    lv_obj_center(home_label);

    // Display the WiFi networks
    display_wifi_networks(wifi_screen);

    // Load the new screen
    lv_scr_load(wifi_screen);
  }
}

static void event_handler_btn_settings(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    LV_LOG_USER("Settings button clicked");

    // Create a new screen for Settings
    lv_obj_t * settings_screen = lv_obj_create(NULL);
    lv_obj_t * label = lv_label_create(settings_screen);
    lv_label_set_text(label, "");
    lv_obj_center(label);

    // Create a home button
    lv_obj_t * home_btn = lv_btn_create(settings_screen);
    lv_obj_add_event_cb(home_btn, event_handler_home, LV_EVENT_ALL, NULL);
    lv_obj_set_size(home_btn, 50, 50);
    lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t * home_label = lv_label_create(home_btn);
    lv_label_set_text(home_label, LV_SYMBOL_HOME);
    lv_obj_center(home_label);

    // Create a WiFi button
    lv_obj_t * wifi_btn = lv_btn_create(settings_screen);
    lv_obj_add_event_cb(wifi_btn, event_handler_wifi, LV_EVENT_ALL, NULL);
    lv_obj_set_size(wifi_btn, 100, 50);
    lv_obj_align_to(wifi_btn, home_btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t * wifi_label = lv_label_create(wifi_btn);
    lv_label_set_text(wifi_label, "WiFi");
    lv_obj_center(wifi_label);

    // Display firmware version
    lv_obj_t * fw_label = lv_label_create(settings_screen);
    lv_label_set_text_fmt(fw_label, "Firmware Version: %s", firmwareVersion);
    lv_obj_align(fw_label, LV_ALIGN_TOP_LEFT, 10, 70);

    // Display WiFi SSID
    lv_obj_t * wifi_ssid_label = lv_label_create(settings_screen);
    lv_label_set_text_fmt(wifi_ssid_label, "WiFi SSID: %s", ssid);
    lv_obj_align(wifi_ssid_label, LV_ALIGN_TOP_LEFT, 10, 100);

    // Display MAC Address
    lv_obj_t * mac_label = lv_label_create(settings_screen);
    lv_label_set_text_fmt(mac_label, "MAC Address: %s", WiFi.macAddress().c_str());
    lv_obj_align(mac_label, LV_ALIGN_TOP_LEFT, 10, 130);

    // Display LVGL version
    lv_obj_t * lvgl_version_label = lv_label_create(settings_screen);
    lv_label_set_text_fmt(lvgl_version_label, "LVGL Version: %d.%d.%d", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    lv_obj_align(lvgl_version_label, LV_ALIGN_TOP_LEFT, 10, 160);

    // Load the new screen
    lv_scr_load(settings_screen);
  }
}


static void event_handler_btn_ph(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    LV_LOG_USER("pH button clicked");

    // Create a new screen for pH
    lv_obj_t * ph_screen = lv_obj_create(NULL);
    lv_obj_t * label = lv_label_create(ph_screen);
    lv_label_set_text(label, "pH Screen");
    lv_obj_center(label);

    // Create a home button
    lv_obj_t * home_btn = lv_btn_create(ph_screen);
    lv_obj_add_event_cb(home_btn, event_handler_home, LV_EVENT_ALL, NULL);
    lv_obj_set_size(home_btn, 50, 50);
    lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t * home_label = lv_label_create(home_btn);
    lv_label_set_text(home_label, LV_SYMBOL_HOME);
    lv_obj_center(home_label);

    // Load the new screen
    lv_scr_load(ph_screen);
  }
}

void lv_create_main_gui(void) {
  lv_obj_t * btn_label;

  // Create a container for the top buttons
  lv_obj_t * btn_cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(btn_cont, SCREEN_WIDTH, 40); // Adjust height as needed
  lv_obj_align(btn_cont, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Make the container non-scrollable
  lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);

  // Create the home button
  lv_obj_t * home_btn = lv_btn_create(btn_cont);
  lv_obj_add_event_cb(home_btn, event_handler_home, LV_EVENT_ALL, NULL);
  lv_obj_set_size(home_btn, 50, 50);
  lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 10, 10);

  btn_label = lv_label_create(home_btn);
  lv_label_set_text(btn_label, LV_SYMBOL_HOME);
  lv_obj_center(btn_label);

  // Create the pH button
  lv_obj_t * btn_ph = lv_btn_create(btn_cont);
  lv_obj_add_event_cb(btn_ph, event_handler_btn_ph, LV_EVENT_ALL, NULL);
  lv_obj_set_width(btn_ph, 100); // Adjust the width as needed

  btn_label = lv_label_create(btn_ph);
  lv_label_set_text(btn_label, "pH");
  lv_obj_center(btn_label);

  // Create the Settings button
  lv_obj_t * btn_settings = lv_btn_create(btn_cont);
  lv_obj_add_event_cb(btn_settings, event_handler_btn_settings, LV_EVENT_ALL, NULL);
  lv_obj_set_width(btn_settings, 100); // Adjust the width as needed

  btn_label = lv_label_create(btn_settings);
  lv_label_set_text(btn_label, "Settings");
  lv_obj_center(btn_label);

  // Create the WiFi status icon
  wifi_icon = lv_label_create(lv_scr_act());
  lv_label_set_text(wifi_icon, LV_SYMBOL_WARNING);
  lv_obj_align(wifi_icon, LV_ALIGN_BOTTOM_LEFT, 10, -10);
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Print firmware version to serial port
  Serial.println("Firmware Version: " + String(firmwareVersion));

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 1: touchscreen.setRotation(1);
  touchscreen.setRotation(3);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Create the main screen
  main_screen = lv_scr_act();

  // Function to draw the GUI (buttons)
  lv_create_main_gui();

  // Initial update of the WiFi icon
  update_wifi_icon();
}

unsigned long previousMillis = 0;
const long interval = 5; // interval in milliseconds
unsigned long wifiCheckMillis = 0;
const long wifiInterval = 10000; // interval to check WiFi status in milliseconds

void loop() {
  unsigned long currentMillis = millis();

  // LVGL task handler with non-blocking delay
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lv_task_handler();  // let the GUI do its work
    lv_tick_inc(interval); // tell LVGL how much time has passed
  }

  // Check WiFi status periodically
  if (currentMillis - wifiCheckMillis >= wifiInterval) {
    wifiCheckMillis = currentMillis;
    update_wifi_icon();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.reconnect();
    }
  }
}
