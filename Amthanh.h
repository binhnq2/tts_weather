#include "Arduino.h"
#include "WiFi.h"
#include <Audio.h>

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

void setupAmThanh() {
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // default 0...21
}

void PhatThongBao(String thongbao) {
  audio.connecttospeech(thongbao, "vi"); // Google TTS
}