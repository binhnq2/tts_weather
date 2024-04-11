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

// const char* ssid = "Tang2new";      // Enter your SSID
// const char* pass = "123456789";               // Enter Password

// Time connect API
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 60000;

// thong tin thoi tiet
float nhietDo ;
const char* thongtinThoiTiet ;
int maThoiTiet ;
const char* noidungCanhBao ;

// cài đặt thời gian
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "<+07>-7"
char ngayThangNam [10];
char gioPhutGiay [5];
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
// định nghĩa cho smartwifi
#define LENGTH(x) (strlen(x) + 1)  
#define EEPROM_SIZE 200             
#define WIFI_RESET 0   
String ssid;                       
String password;       
unsigned long millis_RESET;
bool isFirstTime = true;

Audio audio;

Adafruit_SSD1306 display(128, 64, &Wire, -1);


void setup() {

  // Displays all the weather icons 
  Serial.begin(115200);
  setupHienThi();
  setupWifi();
  //setupAmThanh();
  Serial.println("Setup thoi gian");
  setupThoiGian();
  
  display.clearDisplay();
Serial.println("KN API");
  //audio.connecttospeech("Hôm nay nóng vãi ra!!!", "vi"); // Google TTS
  delay(500);
  startMillis = millis();
  Serial.println("Setup xong");
}

// Thiết lập kết nối wifi
void setupWifi() {
  // displays ssid of the router to be connected
  // display.invertDisplay(0);
  display.clearDisplay();
  display.setCursor(0, 0);
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
    display.display();
    display.clearDisplay();
    display.println("Dang cho WiFi");
    display.display();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    display.clearDisplay();
    Serial.println("WiFi Connected.");

    //Serial.print("IP Address: ");
    //Serial.println(WiFi.localIP());
    
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
    display.println("WiFi Connected");
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
  if (digitalRead(WIFI_RESET) == HIGH)
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
      delay(500);
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
  //display.invertDisplay(1);
  //delay(500);
}

void setupThoiGian() {
  configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
  tzset();
}

void loop() {

Serial.println("Loop...");
  currentMillis = millis();
  millis_RESET = millis();
  // audio.loop();
  // audio.connecttohost("https://github.com/binhnq2/tts_weather/raw/main/1h_mua.mp3");
  display.clearDisplay();

  
  KetNoiAPI();
  display.display();
  //audio.connecttospeech("Hôm nay nóng vãi ra!!!", "vi"); // Google TTS
  delay(5000);
  //resetWifi();
}

// HIỂN THỊ THỜI GIAN //
void HienThiThoiGian() {
  time(&now); // read the current time
  localtime_r(&now, &tm);             // update the structure tm with the current time

  sprintf(gioPhutGiay, "%02d:%02d    %02d-%02d-%02d", tm.tm_hour, tm.tm_min, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

  // Nếu không có nội dung cảnh báo thì hiển thị thời gian
  Serial.println("OK time");
  display.setCursor(5, 54);
  display.setTextSize(1);
  display.println(gioPhutGiay);
}

// THỜI TIÊT //
String payload;

void KetNoiAPI() {
    HTTPClient http;
    http.begin("https://jsonblob.com/api/jsonBlob/1225042321866612736");
    http.addHeader("Content-Type", "application/json");
    int httpcode = http.GET();
    Serial.println("Ket noi");
    if (httpcode != 200)
    {
      display.setTextSize(1);
      display.setCursor(0, 32);
      display.println("Vui long doi.....");
      display.println("");
      display.print("Hay Kiem Tra Internet !!!");
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
      nhietDo = current_obs["temp"];
      thongtinThoiTiet = current_desc["description"];
      maThoiTiet = current_desc["code"];

      // Kiểm tra có thông tin cảnh báo
      JsonArray arrayAlert = object["alerts"];
      if (!arrayAlert.isNull()) {
        noidungCanhBao = arrayAlert[0]["description"];
      } else {
        noidungCanhBao = "";
      }
      
    }
    http.end();
    startMillis = currentMillis;
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
  display.println((String)thongtinThoiTiet);
  display.drawLine(0, 48, 127, 48, WHITE);
  // Nếu có thông tin cảnh báo thì hiển thị
  if (strlen(noidungCanhBao) > 0) {
    Serial.println(noidungCanhBao);
    display.setCursor(0, 56);
    display.print(noidungCanhBao);
    display.print("\t");
    display.print("");
    display.startscrollleft(0x07, 0x07);
  } else {
    display.stopscroll();
    HienThiThoiGian();
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

// ÂM THANH
void setupAmThanh() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // default 0...21
}

void PhatThongBao(const char* thongbao) {
  audio.connecttospeech(thongbao, "vi"); // Google TTS
}
