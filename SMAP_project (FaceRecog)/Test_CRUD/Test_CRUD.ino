#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "ArrayList.h"

template<typename T>
void printArray(ArrayList<T> &list) {
  for (int i = 0; i < list.size(); i++) {
    Serial.print("[" + String(i) + "]: ");
    Serial.println(list[i]);
  }
  Serial.println();
}


const char* ssid = "MichalP";
const char* password = "palavaaa";

//String serverName = "https://facerecog-gate-be.herokuapp.com";
String serverName = "172.20.10.10";   // OR REPLACE WITH YOUR DOMAIN NAME
//String serverName = "192.168.31.100";   // localhost

String serverPath = "/api/v1/users/images/check";     // The default serverPath should be upload.php

//const int serverPort = 80;
const int serverPort = 8000;

int serverStatusOk = 200;

int timoutTimer = 30000;

WiFiClient client;



#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LIGHT        4


#define ledG 2
#define ledR 14
#define btn 15

String line;
bool readyTake = true;
int maxPhotos = 5; // max 5 for now

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  pinMode(ledG, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(btn, INPUT);
  digitalWrite(ledG, LOW);
  digitalWrite(ledR, HIGH);
  //pinMode(relay, OUTPUT);
  //digitalWrite(relay, LOW);
  pinMode(FLASH_LIGHT, OUTPUT);
  digitalWrite(FLASH_LIGHT, LOW);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Wifi connected!");
  Serial.println();
  delay(500);
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  config.frame_size = FRAMESIZE_XGA; //VGA, SVGA, XGA
  config.jpeg_quality = 4;  //0-63 lower number means higher quality
  config.fb_count = maxPhotos;
  delay(500);
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  digitalWrite(ledR, LOW);
}

void loop() {
  if ((digitalRead(btn) == LOW) && readyTake == true) {
    readyTake = false;
    Serial.println("Zapinam foceni!!!");
    bool result = sendPhoto();
    Serial.print("Výsledek je: ");
    Serial.print(result);
    Serial.println();
    if (result == true) {
      digitalWrite(ledR , LOW);
      Serial.println("Otevírám dveře!");
      digitalWrite(ledG , HIGH);
      delay(5000);
      digitalWrite(ledG , LOW);
      Serial.println("Zavírám dveře!");

    }
    else {
      digitalWrite(ledG , LOW);
      Serial.println("Neotevřu");
      digitalWrite(ledR , HIGH);
      delay(5000);
      digitalWrite(ledR , LOW);
    }
    readyTake = true;
    Serial.println("----------------------------");
  }
  if (maxPhotos == 0) {
    ESP.restart();
  }
}

bool sendPhoto() {
  String getAll;
  String getBody;
  String lineRes;
  bool result;

  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");
    maxPhotos -= 1;
    digitalWrite(FLASH_LIGHT, HIGH);
    delay(1000);

    camera_fb_t * fb = NULL;

    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get(); // get fresh image
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
    }
    delay(500);
    digitalWrite(FLASH_LIGHT, LOW);
    String head = "--FaceRecoGate\r\nContent-Disposition: form-data; name=\"file\"; filename=\"check.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--FaceRecoGate--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=FaceRecoGate");
    client.println();
    client.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }
    client.print(tail);


    long startTimer = millis();
    boolean state = true;

    Serial.println("Jedeme tečky");
    while ((startTimer + timoutTimer) > millis() && state) {
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (c != '\n') {
          lineRes += String(c);
        }
        else {
          client.stop();
          state = false;
          break;
        }
      }
    }
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
  if (!client.connected()) {
    Serial.println("disconnected");
    client.stop();
  }
  Serial.println(lineRes);
  Serial.println(lineRes.substring(9, 12));
  int status_code = lineRes.substring(9, 12).toInt();

  if (status_code == serverStatusOk) {
    Serial.println("Otevřeno!");
    result = true;
  }
  else {
    Serial.println("Nepustím!");
    result = false;
  }


  return result;
}
