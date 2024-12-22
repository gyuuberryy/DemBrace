#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int button = 38;
int LED = 42;
int VIBRATE = 3;
bool buttonState = 0;

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x24, 0xec, 0x4a, 0x0e, 0xb1, 0x40};

// Structure example to send data
typedef struct struct_message {
  bool button_value;
} struct_message;

// Create a struct_message called myData
struct_message blink;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  memcpy(&blink, incomingData, sizeof(blink));
  Serial.print("OnDataRecv called. Bytes received: ");
  Serial.println(len);
  Serial.print("Button Value: ");
  Serial.println(blink.button_value);
  led_lightup();
}



// LED and vibration motor activation function
void led_lightup() {
  Serial.println("led_lightup is called");
  if (blink.button_value != 1) {
    buttonState = !buttonState;
    if (buttonState == 1) {
      digitalWrite(LED, HIGH);
      digitalWrite(VIBRATE, HIGH);
      delay(1000);
      
      // Clear the display and force an update
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10, 20);
      display.println(F("Lat: 1.28119 Long: 103.85034"));
      display.display(); // This ensures the display is updated with the new content
      delay(1000);
      
      Serial.println("LED and vibrate activated");
    } else {
      digitalWrite(LED, LOW);
      digitalWrite(VIBRATE, LOW);
      Serial.println("LED and vibrate deactivated");
      
      // Clear display when deactivated
      display.clearDisplay();
      display.display(); // Force the display update to clear the screen
      delay(1000); // Optionally, add a delay if you need some pause after clearing
    }
  }
}



void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
  pinMode(button, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(VIBRATE, OUTPUT);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);  // Stop the program if OLED fails
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register for receive callback
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Setup Complete");
}

// Main loop to send data
void loop() {
  // Read button state
  blink.button_value = digitalRead(button);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &blink, sizeof(blink));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
  
  
  
  delay(1000);
}