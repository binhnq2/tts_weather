#include<HTTPClient.h>
#include<ArduinoJson.h>
#include "WiFi.h"
#include <WiFiUdp.h>
#include<Adafruit_SSD1306.h>
#include<Adafruit_GFX.h>
#include<SPI.h>
#include <time.h>
#include <Audio.h>
#include "EEPROM.h"
#include "weather_icon.h"
#include "utils.h"

// Định nghĩa biến cho màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Định nghĩa biến cho wifi
String ssid;                       
String password; 
// Định nghĩa cho smartwifi
#define LENGTH(x) (strlen(x) + 1)  
#define EEPROM_SIZE 200             
#define WIFI_RESET 13
unsigned long millis_RESET;

// Định nghĩa biến để thời gian chờ kết nội API thông tin thời tiết
unsigned long startConnectApiMillis;
unsigned long currentMillis;
const unsigned long connectApiPeriod = 10000; // Tương ứng 10 giây sẽ gọi API thông tin thời tiết

// Định nghĩa biến để thời gian chờ phát thông tin cảnh báo
unsigned long startAlertMillis;
const unsigned long alertPeriod = 15000; // Tương ứng 15 giây sẽ phát thông tin cảnh báo nếu có thông tin cảnh báo

// Biến lưu trữ thông tin cảnh báo thời tiết
const char* noidungCanhBaoHienThi;

// Định nghĩa biến để thiết lập thông tin thời gian từ WIFI
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "<+07>-7"
char thoigian [5];
time_t now;
tm tm;

// Định nghĩa cổng cho âm thanh
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26
Audio audio;

// Biến tốc độ chạy chữ trên màn hình
#define SCROLL_SPEED 1 // Điều chỉnh tốc độ scroll text trên màn hình

void setup() {
  Serial.begin(115200);
  setupHienThi();
  setupWifi();
  setupAmThanh();
  setupThoiGian();
  
  display.clearDisplay();
  delay(500);
  startConnectApiMillis = millis();
  startAlertMillis = millis();
  
  String text = HienThiThoiTiet();
  HienThiCanhBaoThoiTiet();
  PhatThongBao(text.c_str());
}

void loop() {

  currentMillis = millis();
  millis_RESET = millis();

  KetNoiAPI();
  display.display();
  //resetWifi();
}

/*
// THIẾT LẬP CÀI ĐẶT 
**/
// Thiết lập kết nối wifi
void setupWifi() {
  // displays ssid of the router to be connected
  // display.invertDisplay(0);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  
  pinMode(WIFI_RESET, INPUT);
  if (!EEPROM.begin(EEPROM_SIZE)) { 
    Serial.println("Failed to init EEPROM");
    delay(1000);
  }
  else
  {
    ssid = read_flash(0); 
    Serial.print("SSID = ");
    Serial.println(ssid);
    password = read_flash(40); 
    Serial.print("Password = ");
    Serial.println(password);
  }
  display.println("Dang ket noi wifi...");
  WiFi.begin(ssid.c_str(), password.c_str());
  display.display();

  delay(3000);   

  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();
    display.clearDisplay();
    display.println("Dang cho SmartConfig.");
    display.display();
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }
    display.clearDisplay();
    display.println("Da ket noi SmartConfig.");
    delay(500);
    display.display();
    display.clearDisplay();
    display.println("Dang cho ket noi WiFi...");
    display.display();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    display.clearDisplay();
    Serial.println("Da ket noi WiFi thanh cong !");

    
    ssid = WiFi.SSID();
    password = WiFi.psk();
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("password:");
    Serial.println(password);
    Serial.println("Store SSID & password in Flash");
    write_flash(ssid.c_str(), 0); 
    write_flash(password.c_str(), 40); 
  }
  else
  {
    display.clearDisplay();
    display.println("Da ket noi WiFi thanh cong !");
  }

   display.display();
   delay(500);
}
void write_flash(const char* toStore, int startAddr) {
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}

String read_flash(int startAddr) {
  char in[128]; 
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}

void resetWifi()
{
  while (digitalRead(WIFI_RESET) == HIGH)
  {
    if (millis() - millis_RESET >= 5000)
    {
      Serial.println("Reseting the WiFi credentials");
      write_flash("", 0); 
      write_flash("", 40); 
      Serial.println("Wifi credentials erased");
      display.clearDisplay();
      display.println("Dang khoi dong lai...");
      display.display();
      delay(2000);
      ESP.restart();            
    }
  }
}

void setupHienThi() {
  // Thiết lập cổng giao tiếp
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  delay(300);

  // Hiển thị khởi tạo tên sản phẩm trên màn hình
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.setTextSize(2);
  display.println("DU BAO");
  display.setCursor(10, 30);
  display.println("THOI TIET");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void setupThoiGian() {
  configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
  tzset();
}

/*
// THIẾT LẬP HIỂN THỊ
**/
// THỜI TIÊT //
String payload;

void KetNoiAPI() {
  if (currentMillis - startConnectApiMillis >= connectApiPeriod)
  {
    HienThiThoiTiet();
    startConnectApiMillis = currentMillis;
  }
  HienThiCanhBaoThoiTiet();

  // Thời gian phát cảnh báo theo cài đặt
  if (currentMillis - startAlertMillis >= alertPeriod) {
    String text = "Chú ý !" + (String)noidungCanhBaoHienThi;
    PhatThongBao(text.c_str());
    startAlertMillis = currentMillis;
  }
}

// Hiển thị icon thời tiết
void HienThiIcon(int code_no)
{
  if (200 <= code_no and code_no <= 202)
    display.drawBitmap(0, 0, Thurnderstorm_rain, ICON_W, ICON_H, BLACK, WHITE);
  else if (230 <= code_no and code_no <= 233)
    display.drawBitmap(0, 0, Thunderstorm, ICON_W, ICON_H, BLACK, WHITE);
  else if (300 <= code_no and code_no <= 302)
    display.drawBitmap(0, 0, drizzle, ICON_W, ICON_H, BLACK, WHITE);
  else if (500 <= code_no and code_no <= 522)
    display.drawBitmap(0, 0, rain, ICON_W, ICON_H, BLACK, WHITE);
  // else if ((600 <= code_no and code_no <= 610) or(621 <= code_no and code_no <= 623))
  //   display.drawBitmap(0, 0, snow, ICON_W, ICON_H, BLACK, WHITE);
  else if (611<= code_no and code_no <= 612)
    display.drawBitmap(0, 0, slet, ICON_W, ICON_H, BLACK, WHITE);
  else if (700 <= code_no and code_no <= 751)
    display.drawBitmap(0, 0, mist, ICON_W, ICON_H, BLACK, WHITE);
  else if (801 <= code_no and code_no <= 804)
    display.drawBitmap(0, 0, cloudy, ICON_W, ICON_H, BLACK, WHITE);
  else if (code_no == 900)
    display.drawBitmap(0, 0, unknown_prep, ICON_W, ICON_H, BLACK, WHITE);
  else
    display.fillCircle(30, 30, 20, WHITE);   // sunny day

}
// HIỂN THỊ THỜI GIAN //
void HienThiThoiGian() {
  time(&now); // read the current time
  localtime_r(&now, &tm);             // update the structure tm with the current time

  sprintf(thoigian, "%02d:%02d    %02d-%02d-%02d", tm.tm_hour, tm.tm_min, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

  // Nếu không có nội dung cảnh báo thì hiển thị thời gian
  display.setCursor(5, 54);
  display.setTextSize(1);
  display.println(thoigian);
}

// HIỂN THỊ THỜI TIẾT //
String HienThiThoiTiet() {
  String text = "";
  HTTPClient http;
  http.begin("https://jsonblob.com/api/jsonBlob/1225042321866612736");
  http.addHeader("Content-Type", "application/json");
  int httpcode = http.GET();
  Serial.println("Ket noi");
  if (httpcode != 200)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.println("Vui long doi.....");
    display.println("");
    display.print("Hay Kiem Tra Internet !!!");
    display.display();
    delay(1000);
    return text;
  }

  // Success response
  else
  {
    display.clearDisplay();
    payload=http.getString();
    Serial.println(payload);

    // Parsing
    JsonDocument root;
    DeserializationError error = deserializeJson(root, payload);
    JsonObject object = root.as<JsonObject>();
    JsonArray arrayData = object["data"];
    JsonObject current_obs = arrayData[0];
    JsonObject current_desc = current_obs["weather"].as<JsonObject>();


    const char* city_name = current_obs["city_name"];
    float nhietDo = current_obs["temp"];
    const char* thongtinThoiTiet = current_desc["description"];
    int maThoiTiet = current_desc["code"];

    // Kiểm tra có thông tin cảnh báo
    JsonArray arrayAlert = object["alerts"];
    if (!arrayAlert.isNull()) {
      noidungCanhBaoHienThi = arrayAlert[0]["description"];
    } else {
      noidungCanhBaoHienThi= "";
    }
    
    // Hien thi thong tin
    HienThiIcon(maThoiTiet);
    display.setCursor(62, 10);
    display.setTextSize(2);
    display.print((int)nhietDo);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
    display.setTextSize(1);
    display.setCursor(62, 30);
    String thoitiet = convertVietnameseString((String)thongtinThoiTiet);
    display.println(thoitiet);
    display.drawLine(0, 48, 127, 48, WHITE);

    text = "Thời tiết hôm nay " + (String)thongtinThoiTiet + ", nhiệt độ khoảng " + (int)nhietDo + " độ C";
  }
  http.end();
  return text;
}

// HIỂN THỊ CẢNH BÁO THỜI TIẾT //
void HienThiCanhBaoThoiTiet() {
  // Nếu có thông tin cảnh báo thì hiển thị
  display.setCursor(0, 56);
  display.fillRect(0, 56, 64, 16, SSD1306_BLACK);
  if (strlen(noidungCanhBaoHienThi) > 0) {
    HienThiScroll();
  } else {
    display.stopscroll();
    HienThiThoiGian();
  }
}

// HIỂN THỊ SCROLL TEXT //
void HienThiScroll() {
  static int16_t x = SCREEN_WIDTH;
  String noidung = removeDiacritics((String)noidungCanhBaoHienThi);
  Serial.println(noidung);

  while (x >= -display.width()) {
    display.setCursor(0, 56);
    display.fillRect(x, 56, 128, 16, SSD1306_BLACK);
    display.setCursor(x, 56);
    display.println(noidung);
    display.display();
    delay(50); // Adjust scroll speed
    x -= SCROLL_SPEED;
  }

  // Reset x position for next iteration
  x = SCREEN_WIDTH;
}
// ÂM THANH
void setupAmThanh() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // default 0...21
}

void PhatThongBao(const char* thongbao) {
  if (strlen(thongbao) > 0) {
    audio.connecttospeech(thongbao, "vi"); // Google TTS
    while(audio.isRunning()) {
      audio.loop();
    }
  }
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
