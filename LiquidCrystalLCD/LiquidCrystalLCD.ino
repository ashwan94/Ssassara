#include <LiquidCrystal_I2C.h>  // including header

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
// (주소값, 16x2 디스플레이)

void setup() {
  lcd.init(); // lcd 객체 초기화
  lcd.clear();  // 화면 지우고 커서를 왼쪽 상단 모서리로 옮김         
  lcd.backlight();      // Make sure backlight is on	// 백라이트 on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(2,0);   //Set cursor to character 2 on line 0	// 커서 옮김. 0행에 2열
  lcd.print("Hello world!");	// 문자 출력
  
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1	// 커서 옮김. 1행에 0열
  lcd.print("Success!");
}

void loop() {
}