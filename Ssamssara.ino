#include "WIFIManager.h"  // WIFI
#include "MQTTManager.h"  // MQTT 통신
#include "TH11.h"         // 온습도

int operatingMode = 1;
int operatingPin = D2;
int btnState;
unsigned long currentMillis = 0;
unsigned long preMillis = 0;

// WIFI 없어도 온습도 data 유지
float temp = 0;
float hum = 0;

void setup() {
  pinMode(operatingPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  randomSeed(micros());

  EEPROM.begin(255);
  dht.begin();
  Serial.begin(115200);  // Wemos 의 최소 baud rate
  Serial.println("Start WiFi");
  Serial.println("Start AP");

  long startPoint = micros();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  long endPoint = micros();
  long delta = endPoint - startPoint;

  Serial.print("WiFi 켜는데 소요된 시간 ");
  Serial.println(delta);

  delay(1000);

  IPAddress myIP = WiFi.softAPIP();
  // Wemos 자체 IP 주소
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // 주소 mapping
  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  Serial.println("start server");
  server.begin();
  delay(1000);

  // Wemos ROM 에 저장된 WiFi 정보 가져오기
  String SSID_temp = getSSIDEEPROM();
  String Password = getPWEEPROM();

  Serial.println("ROM Load");
  if (SSID_temp == "" && Password == "") {
    Serial.println("ROM is empty.");
    delay(1000);
  } else {
    Serial.println(SSID_temp);
    Serial.println(Password);
  }
}


void loop() {
  currentMillis = millis();
  int btnState = digitalRead(operatingPin);
  // Serial.println(btnState);

  // 최초 1회에 대해 WiFi 정보 생성
  if (operatingMode == 2 && btnState) {
    Serial.println("버튼 1회 작동 : ");
    Serial.println(operatingMode);
    operatingMode = 1;
    // Wemos Board 자체적으로 WiFi 구축
    // 숙주 WiFi 에 기생하기 위한 첫 걸음

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    server.begin();
    delay(1000);
    Serial.println("Switching STA >>>>> AP");
  } else if (operatingMode == 1 && btnState) {
    // 2회 실행부터 이미 ROM 에 숙주 WiFi 정보 있으므로 가져옴
    String SSID_temp = getSSIDEEPROM();
    String Password = getPWEEPROM();

    Serial.println("ROM Load");
    Serial.println("SSID_temp");
    Serial.println("Password");

    if (isValidate(SSID_temp) && isValidate(Password)) {
      // 숙주 WiFi 에 기생하기
      connectedSSID = SSID_temp;
      connectedPassword = Password;
      operatingMode = 2;
      Serial.println();
      Serial.print("Connection to ");
      Serial.println(connectedSSID);

      WiFi.mode(WIFI_STA);
      WiFi.begin(connectedSSID, connectedPassword);
      delay(1000);

      // 연결 성공할때까지 대기
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      // 숙주 WiFi 에게서 연결 허락
      Serial.println("WiFi Connection is OK!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("Switching AP >>>>> STA");
      delay(1000);

      // MQTT 연결
      setupMQTT();
      reconnect();
    } else {
      Serial.println("WiFi setting isn't yet");
      Serial.println("Check your WiFi information");
      Serial.println("SSID, PW, ROM etc...");
      delay(1000);
    }
  }

  switch (operatingMode) {
    case 1:
      networking();  // 숙주 WiFi 찾기
      break;

    case 2:
      sensing();  // 숙주 WiFi 에 기생
      break;
  }
}

void networking() {
  server.handleClient();
}

void sensing() {

  // 온습도 센서 측정 + 시분할 처리
  if (currentMillis - preMillis > DHTSensingDelay) {
    temp = readTemp();  // 온도
    hum = readHum();    // 습도

    String sensingLog = "Temp:" + String(temp) + "C | Hum:" + String(hum) + "% ";
    Serial.println(sensingLog);

    preMillis = currentMillis;

    // 숙주 WiFi 연결된 상태 + MQTT 연결될 떄만 data 전송
    if (WiFi.status() == WL_CONNECTED && pubClient.connected()) {
      // MQTT 로 data 전송하기 위한 JSON 으로 형변환
      String tempStr = String(temp);
      String humStr = String(hum);
      String readAnalogStr = "1023";
      String sensorID = "Ssamssara";

      // JSON 객체 MQTT 로 publish
      String data = String("{\"sensor_id\" : \"" + sensorID
                           + "\", \"analog\": \"" + readAnalogStr
                           + "\", \"temp\" : \"" + tempStr
                           + "\", \"hum\" : \"" + humStr
                           + "\"}");

      String rootTopic = "/IoT/Sensor/" + sensorID;

      publish(rootTopic, data);
      Serial.println(data);
    }
  } else {
    // 저전력을 고려한 로직
  }
}