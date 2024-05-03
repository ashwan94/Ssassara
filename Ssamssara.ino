#include "WIFIManager.h"  // WIFI
#include "MQTTManager.h"  // MQTT 통신
#include "TH11.h"         // 온습도
#include "LiquidCrystalLCD.h" // LCD 창
#include "soilMoistureSensor.h" // 토양습도

const int operatingPin = D0; // WiFi 연결을 위한 button
int operatingMode = 1;
int btnState;
unsigned long currentMillis = 0;
unsigned long preMillis = 0;

// WIFI 없어도 온습도 data 유지
float temp = 0;

void setup() {
  pinMode(operatingPin, INPUT); // ROM 정보로 WiFi 연결
  pinMode(LED_BUILTIN, OUTPUT); // ESP8266 보드의 LED
  pinMode(soilSensor, INPUT);   // 토양습도센서
  // pinMode(waterPump, OUTPUT);   // 워터펌프
  
  lcd.init(); // lcd 객체 초기화
  lcd.clear();  // 화면 지우고 커서를 왼쪽 상단 모서리로 옮김         
  lcd.backlight();      // 백라이트 on
  lcd.setCursor(1,0);
  lcd.print("initializing...");

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

  // 온습도 센서 측정 + 시분할 처리(2.1초 간격이지만 너무 과하므로 5초로 세팅함)
  // TODO
  // LCD 에 온습도를 소수첫째자리까지만 표기하도록 변경
  if (currentMillis - preMillis > DHTSensingDelay) {
    temp = round(readTemp() * 10.0) / 10.0;  // 대기 중 온도
    int soil = getSoilMoisture();            // 토양 습도

    lcd.clear();
    onLiquidCrystal(temp, soil);

    String sensingLog = "Temp: " + String(temp) + "C | Soil: " + String(soil) + "%";
    // Serial.println(sensingLog);

    preMillis = currentMillis;

    // 숙주 WiFi 연결된 상태 + MQTT 연결될 떄만 data 전송
    if (WiFi.status() == WL_CONNECTED && pubClient.connected()) {
      // MQTT 로 data 전송하기 위한 JSON 으로 형변환
      String tempStr = String(temp);
      String soilStr = String(soil);
      String readAnalogStr = "1023"; // TODO 이거 뭐였지?
      String sensorID = "Ssamssara";

      // JSON 객체 MQTT 로 publish
      String data = String("{\"sensor_id\" : \"" + sensorID
                           + "\", \"analog\": \"" + readAnalogStr
                           + "\", \"temp\" : \"" + tempStr
                           + "\", \"soil\" : \"" + soilStr
                           + "\"}");

      String rootTopic = "/IoT/Sensor/" + sensorID;

      publish(rootTopic, data);
      Serial.println(data);
    }
  } else {
    // 저전력을 고려한 로직
  }
}