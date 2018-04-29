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
  pinMode(16, OUTPUT);

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
    
    // buzz once with duration 0.5 second
    digitalWrite(16, HIGH);
    digitalWrite(16, LOW);
    delay(500);
    digitalWrite(16, HIGH);
    
  } else if (type == LONG_BUZZ) {
    
    // buzz once with duration 1 second
    digitalWrite(16, HIGH);
    digitalWrite(16, LOW);
    delay(1000);
    digitalWrite(16, HIGH);
    
  } else if (type == VISUAL_COUNTDOWN) {
    
  } else if (type == BEEP) {
    
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

  delay(5000);
  
}
