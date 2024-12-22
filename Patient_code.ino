#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>

int button = 38;
int LED = 42;
int VIBRATE = 3;
bool buttonState = 0;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x24, 0xec, 0x4a, 0x07, 0x5b, 0x54};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  bool button_value;
} struct_message;

// Create a struct_message called myData
struct_message blink;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&blink, incomingData, sizeof(blink));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Button Value: ");
  Serial.println(blink.button_value);
  led_lightup();
  
}

void led_lightup() {
  if (blink.button_value != 1) { //while button is pressed
    buttonState = !buttonState;   //changes buttonState everytime button is pressed
 //button state only changes when button is pressed down, once u release whatever the button state is at will be saved
    if (buttonState == 1) {
      digitalWrite(LED, HIGH);
      digitalWrite(VIBRATE, HIGH);
    } else{
        digitalWrite(LED, LOW);
        digitalWrite(VIBRATE, LOW);
    }
    delay(100);
    
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  pinMode(VIBRATE, INPUT_PULLUP);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(VIBRATE, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "LOCATION HERE !!");
  u8g2.sendBuffer();
  
}
 
void loop() {
  // Set values to send
  blink.button_value = digitalRead(button);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &blink, sizeof(blink));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(1000);
}