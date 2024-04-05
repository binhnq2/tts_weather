#include<HTTPClient.h>
#include<ArduinoJson.h>
#include <WiFiUdp.h>
#include<Adafruit_SSD1306.h>
#include<Adafruit_GFX.h>
#include<SPI.h>
#include <time.h>
#include <Audio.h>
#include "weather_icon.h"

const char* ssid = "Tang2new";      // Enter your SSID
const char* pass = "123456789";               // Enter Password


// Variables to save date and time
char ngayThangNam [10];
char gioPhutGiay [5];

/* Configuration of NTP */
// choose the best fitting NTP server pool for your country
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "<+07>-7"

time_t now;                          // this are the seconds since Epoch (1970) - UTC
tm tm;

String api_head = "http://api.weatherbit.io/v2.0/current?city=";      

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

Adafruit_SSD1306 display(128, 64, &Wire, -1);


void setup() {

  // Displays all the weather icons 
  Serial.begin(115200);
  //setupHienThi();
  setupWifi();
  setupAmThanh();
  //setupThoiGian();

  audio.connecttospeech("Hôm nay nóng vãi ra!!!", "vi"); // Google TTS
  delay(5000);
}

// Thiết lập kết nối wifi
void setupWifi() {
  // displays ssid of the router to be connected
  // display.invertDisplay(0);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Dang ket noi Wifi:");
  display.println(ssid);
  //display.display();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)   // wait for connection
  {
    Serial.print(".");
    delay(500);
  }
  // display.setCursor(0, 48);
  // display.setTextSize(2);
  // display.setTextColor(BLACK, WHITE);
  // display.print("Da ket noi !");
  // display.display();
  // delay(500);
  // display.clearDisplay();
  // display.setTextColor(WHITE);
}

void setupHienThi() {
  // Thiết lập cổng giao tiếp
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  delay(300);

  // Hiển thị khởi tạo tên sản phẩm trên màn hình
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println("OLED");
  display.println("WEATHER");
  display.println("MOTINORING");
  display.display();
  delay(1000);
  display.invertDisplay(1);
  delay(500);
}

void setupThoiGian() {
  configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
  tzset();
}

void loop() {

  audio.loop();
  // display.clearDisplay();
  // HienThiThoiGian();
  
  // KetNoiAPI();
  
  // display.display();
}

// HIỂN THỊ THỜI GIAN //
void HienThiThoiGian() {
  time(&now); // read the current time
  localtime_r(&now, &tm);             // update the structure tm with the current time

  sprintf(gioPhutGiay, "%02d:%02d    %02d-%02d-%02d", tm.tm_hour, tm.tm_min, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

    
  display.setCursor(5, 54);
    display.setTextSize(1);
    display.println(gioPhutGiay);
  delay(1000);
}

// THỜI TIÊT //
String payload;

void KetNoiAPI() {
  HTTPClient http;
  // String main_head = "";
  // main_head = api_head + "GOA";
  // main_head = main_head + "&country=IN&units=M&key=";
  // main_head = main_head + api_key;
  http.begin("https://jsonblob.com/api/jsonBlob/1225042321866612736");
  http.addHeader("Content-Type", "application/json");
  int httpcode = http.GET();
  if (httpcode != 200)    // Not an OK response
  {
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.println("Please Wait.....");
    display.println("");
    display.print("Is internet available??");
    display.display();
    delay(1000);
    return;
  }

  // Success response
  else
  {
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
    float curr_temp = current_obs["temp"];
    int air_quality = current_obs["aqi"];
    float app_temp = current_obs["app_temp"];
    float wind_spd = current_obs["wind_spd"];
    const char* wind_cdir = current_obs["wind_cdir"];
    const char* sunrise = current_obs["sunrise"];
    const char* sunset = current_obs["sunset"];
    const char* desc = current_desc["description"];
    int code = current_desc["code"];

    HienThiIcon(code);
   

    display.setCursor(62, 10);
    display.setTextSize(2);
    display.print(curr_temp);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");

    display.setTextSize(1);
    display.setCursor(62, 30);
    display.println(desc);
    display.drawLine(0, 48, 127, 48, WHITE);

  
    //audio.connecttospeech(desc, "vi");
    
  }
  http.end();
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

// ÂM THANH
void setupAmThanh() {
    //SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // default 0...21
}

void PhatThongBao(const char* thongbao) {
  audio.connecttospeech(thongbao, "vi"); // Google TTS
}