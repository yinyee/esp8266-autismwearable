#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

enum NotificationType {
  SHORT_BUZZ = 0,
  LONG_BUZZ = 1,
  VISUAL_COUNTDOWN = 2,
  BEEP = 3,
  NONE = 4
};

NotificationType notif_type;

String ip;
int httpCode;
HTTPClient httpClient;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(16, OUTPUT); // motor
  pinMode(14, OUTPUT); // blue
  pinMode(12, OUTPUT); // green
  digitalWrite(16, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);

  // connect to wifi
  WiFi.begin("Reactor", "Open123!");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for connection...");
    delay(5000);
  }

  // find out the ip assigned to me
  ip = WiFi.localIP().toString();
  Serial.println(ip);

  // start http client
  notif_type = NONE;
  Serial.print("Initial notification type: ");
  Serial.printf("%d\n", notif_type); // should print 4
  httpClient.begin("http://young-bayou-55448.herokuapp.com/step");
}

void process(String input) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input);
  if(!root.success()) {
    Serial.println("Parsing failed");
  }

  const char* respRoot = root["notif-type"];
  Serial.println(respRoot);
  NotificationType new_notif_type(strToNotifType(respRoot));
  Serial.print("Notification type received: ");
  Serial.printf("%d\n", new_notif_type);

  if(new_notif_type != notif_type) {
    notif_type = new_notif_type;
    generateNotification(notif_type);
    Serial.print("Notification type changed: ");
    Serial.printf("%d\n", notif_type);
  } else {
    Serial.print("Notification type unchanged: ");
    Serial.printf("%d\n", notif_type);
  }
  
}

NotificationType strToNotifType(const char* strType) {
  
  NotificationType type;
  
  if (strcmp(strType, "SHORT_BUZZ") == 0) {   
    type = SHORT_BUZZ;
  } else if (strcmp(strType, "LONG_BUZZ") == 0) {
    type = LONG_BUZZ;
  } else if (strcmp(strType, "VISUAL_COUNTDOWN") == 0) {
    type = VISUAL_COUNTDOWN;
  } else if (strcmp(strType, "BEEP") == 0) {
    type = BEEP;
  } else {
    type = NONE;
  }
  return type;
}

void generateNotification(NotificationType type) {
  
  if (type == SHORT_BUZZ) {
    
    // short buzz once
    // and blink blue once
    digitalWrite(16, LOW);
    delay(500);
    digitalWrite(16, HIGH);
    digitalWrite(14, LOW);
    delay(250);
    digitalWrite(14, HIGH);
    
  } else if (type == LONG_BUZZ) {
    
    // long buzz once
    // and blink blue once and green once
    digitalWrite(16, LOW);
    delay(750);
    digitalWrite(16, HIGH);
    digitalWrite(14, LOW);
    delay(500);
    digitalWrite(14, HIGH);
    delay(250);
    digitalWrite(12, LOW);
    delay(500);
    digitalWrite(12, HIGH);
    
  } else if (type == VISUAL_COUNTDOWN) {

    // blink turquoise thrice
    int count = 0;
    while (count < 3) {
      int inner_count = 0;
      while (inner_count < 2) {
        digitalWrite(14, LOW);
        delay(50);
        digitalWrite(14, HIGH);
        digitalWrite(12, LOW);
        delay(50);
        digitalWrite(12, HIGH);
        inner_count++;
      }
      delay(500);
      count++;
    }
    
  } else if (type == BEEP) {

    // buzz quickly thrice
    // and blink green thrice
    int count = 0;
    while (count < 3) {
      digitalWrite(16, LOW);
      delay(200);
      digitalWrite(16, HIGH);
      digitalWrite(12, LOW);
      delay(200);
      digitalWrite(12, HIGH);
      count++;
    }
    
  } else if (type == NONE) {
    // do nothing
  }
  
}

void loop() {

  // reset http response code
  httpCode = -99;
  
  // poll the notification server
  while(httpCode < 0) {
    httpCode = httpClient.GET();
    if(httpCode != 200) {
       Serial.println(httpCode + " " + httpClient.errorToString(httpCode) + " " + httpClient.getString());
    }
  }

  // process response
  String response = httpClient.getString();
  Serial.println(response);
  process(response);

  delay(2000);
  
}
