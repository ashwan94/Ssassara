#ifndef SoilMoisture
#define SoilMoisture

const int soilSensor = A0; // 토양습도센서
// const int waterPump = D5; // 워터펌프

int getSoilMoisture();

int getSoilMoisture() {
  int soilValue = analogRead(soilSensor);              //토양수분센서값 읽기
  int soilPercent = map(soilValue, 170, 1023, 100, 0); // 170 ~ 1023 에 대한 value 를 100 ~ 0% 로 매핑, 170에 가까울수록 수분 많음
  Serial.println(soilPercent);

  // 토양습도가 60% 이하일 경우 워터펌프 작동
  if(soilPercent < 60){
    Serial.println("물 주기");
    // digitalWrite(waterPump, HIGH);
  }else{
    Serial.println("수분 충분");
    // digitalWrite(waterPump,LOW);
  }
  return soilPercent;
}

#endif