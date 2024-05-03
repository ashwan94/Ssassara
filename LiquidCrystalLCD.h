#ifndef LiquidCrystalOn
#define LiquidCrystalOn

#include <LiquidCrystal_I2C.h>  // including header

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
// (주소값, 16x2 디스플레이)

void onLiquidCrystal();

void onLiquidCrystal(float temp, float hum) {
  
  String strHum = String(hum) + "%";
  String strTemp = String(temp) + "C" + " " + strHum;

  lcd.setCursor(1,0);   //Set cursor to character 2 on line 0	// 커서 옮김. 0행에 2열
  lcd.print(strTemp);	// 문자 출력
  
  lcd.setCursor(1,1);   //Move cursor to character 2 on line 1	// 커서 옮김. 1행에 0열
  // lcd.print(strHum);
  // lcd.print("시간");
}

#endif