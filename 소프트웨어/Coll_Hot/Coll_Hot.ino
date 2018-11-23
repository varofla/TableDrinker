#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Thermistor.h>
//#include <SteinhartHart.h>

#define _EN_1PIN_ 2
#define _EN_2PIN_ 3
#define _EN_CLPIN_ 8 //
#define _MO_PWMPIN_ 9
#define _MO_ENPIN_ 4 //
#define _FAN_PWMPIN_ 11
#define _FAN_ENPIN_ 10
/*
#define _EN_1PIN_ 2
#define _EN_2PIN_ 3
#define _EN_CLPIN_ 4
#define _MO_PWMPIN_ 9
#define _MO_ENPIN_ 8
#define _FAN_PWMPIN_ 11
#define _FAN_ENPIN_ 10
*/

#define address_EEPROM_hightemp 1
#define address_EEPROM_lowtemp 0
#define address_EEPROM_mode 2
Encoder Enc(_EN_1PIN_, _EN_2PIN_);
LiquidCrystal_I2C LCD(0x3F, 16, 2);
Thermistor Sensor_top(A0);
Thermistor Sensor_bot(A1);
/*해야할 것*/
/*
1. 초기화 기능을 on/off로 바꾸기 - > on/off를 누르면 메인 화면으로 돌아가야함
*/
/*LCD 커스텀 도트*/
byte newChar1[8] = {
B01000,
  B10100,
  B01000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte newChar2[8] = {
	B11111,
  B11011,
  B10101,
  B11011,
  B10101,
  B11011,
  B10101,
  B11111
};
byte newChar3[8] = {
	B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};
byte newChar4[8] = {
	B00000,
	B01010,
	B01010,
	B01110,
	B01010,
	B01010,
	B00000,
	B00000
};
byte newChar5[8] = {
	B00000,
	B00110,
	B01000,
	B01000,
	B01000,
	B00110,
	B00000,
	B00000
};
byte newChar6[8] = { //블럭
	B00000,
	B00000,
	B00000,
	B01110,
	B01110,
	B00000,
	B00000,
	B00000
};
byte newChar7[8] = { //논블럭
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000
};

/*변수 선언*/
int DEBUG = 1;
int Must_coll = 0; //50도 이상 올라가면 45도까지 쿨러 가동
int END_COLLING = 0; //정지 후 쿨링 설정, 1
int mode = 1; //1은 히팅모드, 2은 쿨링모드
int scroll = 0; //1: 히팅/쿨링, 2: MAX설정, 3: MIN설정, 0:뒤로가기
int mode_setting = 0; //인코더 스크롤의 역할 설정, 1: 선택모드, 2: 모드 히팅/쿨링 ...
long Time = 0;
long Time2 = 0;
long Time_plash = 0;
int ONOFF = 0; //1일 경우에 펠티어 소자 작동 가능, 0일 경우에 펠티어 소자 작동안함

struct TM { //온도 관련 변수
	int now = 0;
	int set = 0;
}; TM Temp_top, Temp_bot;

/*기타 함수*/
int plash() {
	digitalWrite(13, HIGH);
	Time_plash = millis() + 50;
}

/*LCD화면 프린팅*/
struct LC { //LCD관련 함수셋
	int menu = 0; //0: 홈, 1: 세팅모드
	int clear(int clear_location) {
		if (clear_location == 0) {
			LCD.setCursor(0, 0);
			LCD.print("                  ");
			LCD.setCursor(0, 1);
			LCD.print("                  ");
		}
		else {
			LCD.setCursor(0, clear_location -1);
			LCD.print("                  ");
		}
	}
	int home() {
		LCD.setCursor(0, 0);
		LCD.print("T:");
		if (Temp_top.now < 0) {
			if (Temp_top.now > -10) LCD.print(" ");
			LCD.print(Temp_top.now);
		}
		else if (Temp_top.now < 10) { //**9인 경우
			LCD.print("  ");
			LCD.print(Temp_top.now);
		}
		else if (Temp_top.now < 100) { //*99인 경우
			LCD.print(" ");
			LCD.print(Temp_top.now);
		}
		else LCD.print(Temp_top.now);
		LCD.print("/");
		if (Temp_top.set < 0) {
			if (Temp_top.set > -10) LCD.print(" ");
			LCD.print(Temp_top.set);
		}
		else if (Temp_top.set < 10) { //**9인 경우
			LCD.print("  ");
			LCD.print(Temp_top.set);
		}
		else if (Temp_top.set < 100) { //*99인 경우
			LCD.print(" ");
			LCD.print(Temp_top.set);
		}
		else LCD.print(Temp_top.set);
		LCD.write(byte(0)); //온도기호
		if(mode == 1) LCD.write(byte(1)); //히팅기호
		else LCD.write(byte(2)); //쿨링기호
		LCD.setCursor(0, 1);
		LCD.print("B:");
		if (Temp_bot.now < 0) {
			if (Temp_bot.now > -10) LCD.print(" ");
			LCD.print(Temp_bot.now);
		}
		else if (Temp_bot.now < 10) { //**9인 경우
			LCD.print("  ");
			LCD.print(Temp_bot.now);
		}
		else if (Temp_bot.now < 100) { //*99인 경우
			LCD.print(" ");
			LCD.print(Temp_bot.now);
		}
		else LCD.print(Temp_bot.now);
		LCD.print("/");
		if (Temp_bot.set < 0) {
			if (Temp_bot.set > -10) LCD.print(" ");
			LCD.print(Temp_bot.set);
		}
		else if (Temp_bot.set < 10) { //**9인 경우
			LCD.print("  ");
			LCD.print(Temp_bot.set);
		}
		else if (Temp_bot.set < 100) { //*99인 경우
			LCD.print(" ");
			LCD.print(Temp_bot.set);
		}
		else LCD.print(Temp_bot.set);
		LCD.write(byte(0)); //온도기호
		if (mode == 1) LCD.write(byte(2)); //히팅기호
		else LCD.write(byte(1)); //쿨링기호
	}
	int setting() {
		LCD.setCursor(0, 0);
		if (scroll == 1) LCD.write(byte(5)); else LCD.write(byte(6));
		LCD.setCursor(1, 0);
		LCD.print("Mode:");
		if (mode == 1) {
			LCD.print("Heating");
		}
		else if (mode == 2) {
			LCD.print("Cooling");
		}
		LCD.setCursor(0, 1);
		if (scroll == 2) LCD.write(byte(5)); else LCD.write(byte(6));
		LCD.setCursor(1, 1);
		LCD.print("MAX:");
		LCD.print((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set);
		LCD.setCursor(9, 1);
		if (scroll == 3) LCD.write(byte(5)); else LCD.write(byte(6));
		LCD.setCursor(10, 1);
		LCD.print("MIN:");
		LCD.print((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set);
		if (mode == 1) {
			if ((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 100);
			else if ((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 10) LCD.print(" ");
			else if ((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 0) LCD.print("  ");
			else if ((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set >= -9) LCD.print(" ");
		}
		else {
			if ((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 100);
			else if ((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 10) LCD.print(" ");
			else if ((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set >= 0) LCD.print("  ");
			else if ((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set >= -9) LCD.print(" ");
		}
	}
}; LC lcd;
void LCD_print() {
	if (lcd.menu == 0) { //메인메뉴에서 리프레싱
		lcd.home();
	}
	else if (lcd.menu == 1) {
		lcd.setting();
	}
}

/*인코더 체킹*/
struct EN {
	long New;
	long Old = -999;
	int New_cl = 0;
	int Old_cl = 0;
	int count_EN;
}; EN encoder;
void Check_encoder() { //인코더 체크
	int asdfawsdf = digitalRead(_EN_CLPIN_);
	if (!asdfawsdf) { //버튼누름
		if (lcd.menu == 0) { //홈에서만 작동하는 기능
			lcd.clear(0);
			lcd.menu = 1;
			mode_setting = 1;
		}
		else if (lcd.menu == 1) { //세팅에서만 작동하는 기능
			if (scroll == 0) {
				lcd.menu = 0; //스크롤도 나가는 위치에 있어야함
				lcd.clear(0);
			}
			else if (scroll == 1) { //히팅/쿨링 모드설정
				int  Encoder_v_n = 0, Encoder_v_o = -999, Encoder_v = 0;
				while(!digitalRead(_EN_CLPIN_)) {
					Encoder_v_n = Enc.read();
					if (Encoder_v_n != Encoder_v_o) {
						if (Encoder_v_n < Encoder_v_o) Encoder_v++;
						if (Encoder_v_n > Encoder_v_o) Encoder_v--;
						if (Encoder_v == 2) {
							Encoder_v = 0;
							mode = 2;
							int C = Temp_top.set;
							if (Temp_top.set > Temp_bot.set) {
								Temp_top.set = Temp_bot.set;
								Temp_bot.set = C;
							}
						}
						else if (Encoder_v == -2) {
							Encoder_v = 0;
							mode = 1;
							int C = Temp_bot.set;
							if (Temp_top.set < Temp_bot.set) {
								Temp_bot.set = Temp_top.set;
								Temp_top.set = C;
							}
						}
						Encoder_v_o = Encoder_v_n;
					}
					LCD_print();
				}
				EEPROM.write(address_EEPROM_mode, mode);
			}
			else if (scroll == 2) { //MAX온도 설정
				int  Encoder_v_n = 0, Encoder_v_o = -999, Encoder_v = 0;
				while (!digitalRead(_EN_CLPIN_)) {
					Encoder_v_n = Enc.read();
					if (Encoder_v_n != Encoder_v_o) {
						if (Encoder_v_n < Encoder_v_o) Encoder_v++;
						if (Encoder_v_n > Encoder_v_o) Encoder_v--;
						if (Encoder_v == 1) {
							Encoder_v = 0;
							if (mode == 1) Temp_top.set--;
							else Temp_bot.set--;
						}
						else if (Encoder_v == -2) {
							Encoder_v = 0;
							if (mode == 1) Temp_top.set++;
							else Temp_bot.set++;
						}
						Encoder_v_o = Encoder_v_n;
					}
					LCD_print();
				}
				//온도저장
				int min, max;
				min = (Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set;
				max = (Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set;
				EEPROM.write(address_EEPROM_hightemp, max);
			}
			else if (scroll == 3) { //MIN온도 설정
				int  Encoder_v_n = 0, Encoder_v_o = -999, Encoder_v = 0;
				while (!digitalRead(_EN_CLPIN_)) {
					Encoder_v_n = Enc.read();
					if (Encoder_v_n != Encoder_v_o) {
						if (Encoder_v_n < Encoder_v_o) Encoder_v++;
						if (Encoder_v_n > Encoder_v_o) Encoder_v--;
						if (Encoder_v == 1) {
							Encoder_v = 0;
							if(mode==1) Temp_bot.set--;
							else Temp_top.set--;
						}
						else if (Encoder_v == -2) {
							Encoder_v = 0;
							if (mode == 1) Temp_bot.set++;
							else Temp_top.set++;
						}
						Encoder_v_o = Encoder_v_n;
					}
					LCD_print();
				}
				//온도저장
				int min, max;
				min = (Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set;
				max = (Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set;
				EEPROM.write(address_EEPROM_lowtemp, -min);

			}
			else if (scroll == 4) { //초기화 모드 - > on/off모드로 바꾸기
				scroll = 0;
				if (ONOFF == 1) { //꺼야할때
					ONOFF = 0;
					END_COLLING = 1;
					//digitalWrite(_FAN_PWMPIN_, LOW);
				}
				else { //켜야할 때
					ONOFF = 1;
					//digitalWrite(_FAN_PWMPIN_, HIGH);
					Check_temp();
				}
				lcd.clear(0);
				lcd.menu = 0; //메인메뉴로 돌아감
			}
		}
		plash();
		LCD_print();
		while (!digitalRead(_EN_CLPIN_)) {} //두번누르는거 방지
	}
	encoder.New = Enc.read();
	if (encoder.New != encoder.Old) {
		if (encoder.New < encoder.Old) encoder.count_EN++;
		if (encoder.New > encoder.Old) encoder.count_EN--;
		if (encoder.count_EN == 4) { //반시계방향 (-)
			encoder.count_EN = 0;
			plash();
			if (lcd.menu == 1) {
				if (mode_setting == 1) { //선택모드
					if (scroll > 0) scroll--; //0 밖으로 나가지 않아야함
				}
				//mode = 2;
				LCD_print();
			}
		}
		else if (encoder.count_EN == -4) { //시계방향 (+)
			encoder.count_EN = 0;
			plash();
			if (lcd.menu == 1) {
				if (mode_setting == 1) { //선택모드
					if (scroll < 4) scroll++; //4 밖으로 나가지 않아야함
				}
				//mode = 1;
				LCD_print();
			}
		}
		encoder.Old = encoder.New;
	}
}
/*온도 체킹*/
//센서에러도 고려할것.
//define으로 강제 상한선도 만들 것.
void Check_temp() {
	Temp_top.now = (int)Sensor_bot.getTemp();
	Temp_bot.now = (int)Sensor_top.getTemp();
	if (END_COLLING) { //끌 때 45도까지 온도 내림
		digitalWrite(_FAN_PWMPIN_, HIGH);
		if (Temp_bot.now <= 45) {
			digitalWrite(_FAN_PWMPIN_, LOW);
			END_COLLING = 0;
		}
	}
	else if (mode == 1 && ONOFF) { //히팅 모드일 때
		if (Temp_top.now < Temp_top.set && Temp_bot.now > Temp_bot.set) {
			LCD.setCursor(15, 0);
			LCD.print("H");
			digitalWrite(_MO_PWMPIN_, HIGH);
			digitalWrite(_MO_ENPIN_, LOW);
		}
		else {
			LCD.setCursor(15, 0);
			LCD.print(" ");
			digitalWrite(_MO_PWMPIN_, LOW);
		}
		if (Temp_bot.now < 50 && !Must_coll) { //아래 온도가 40도 이하이면
			digitalWrite(_FAN_PWMPIN_, LOW);
		}
		if (Temp_bot.now >= 50 || Must_coll) {
			digitalWrite(_FAN_PWMPIN_, HIGH);

			//한 번 켜지면 45도까지 온도 내림(너무 떨어져도 안좋음)
			if (Temp_bot.now >= 50 && Must_coll == 0) {
				Must_coll = 1;
			}
			if (Temp_bot.now <= 45 && Must_coll == 1) {
				Must_coll = 0;
			}
		}
	}
	else if (mode == 2 && ONOFF) {
		digitalWrite(_FAN_PWMPIN_, HIGH);
		if (Temp_top.now > Temp_top.set && Temp_bot.now < Temp_bot.set) {
			LCD.setCursor(15, 0);
			LCD.print("H");
			digitalWrite(_MO_PWMPIN_, HIGH);
			digitalWrite(_MO_ENPIN_, HIGH); //
		}
		else {
			LCD.setCursor(15, 0);
			LCD.print(" ");
			digitalWrite(_MO_PWMPIN_, LOW);
		}
	}
}

void setup() {
	if(DEBUG) Serial.begin(9600);
	LCD.init();
	LCD.backlight();
	LCD.begin(16, 2);
	lcd.clear(0);
	LCD.setCursor(0, 0);
	LCD.print("VarOfLa");
	LCD.setCursor(0, 1);
	LCD.print("Portable_HeCoer");
	
	pinMode(_EN_CLPIN_, INPUT_PULLUP);
	pinMode(13, OUTPUT);
	pinMode(_MO_PWMPIN_, OUTPUT);
	pinMode(_MO_ENPIN_, OUTPUT);
	pinMode(_FAN_PWMPIN_, OUTPUT);
	pinMode(_FAN_ENPIN_, OUTPUT);
	digitalWrite(_MO_PWMPIN_, LOW);
	digitalWrite(_MO_ENPIN_, HIGH);
	digitalWrite(_FAN_PWMPIN_, LOW);
	digitalWrite(_FAN_ENPIN_, HIGH);
	digitalWrite(13, HIGH);
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	plash();
	
	/*기본 온도 세팅*/
	mode = EEPROM.read(address_EEPROM_mode);
	if (mode == 1) {
		Temp_top.set = EEPROM.read(address_EEPROM_hightemp);
		Temp_bot.set = -1 * EEPROM.read(address_EEPROM_lowtemp);
	}
	else {
		Temp_bot.set = EEPROM.read(address_EEPROM_hightemp);
		Temp_top.set = -1 * EEPROM.read(address_EEPROM_lowtemp);
	}
	/*LCD 커스텀 문자 세팅*/
	LCD.createChar(0, newChar1); //온도기호
	LCD.createChar(1, newChar2); //히팅기호
	LCD.createChar(2, newChar3); //쿨링기호
	LCD.createChar(3, newChar4); //N/C
	LCD.createChar(4, newChar5); //N/C
	LCD.createChar(5, newChar6); //블럭
	LCD.createChar(6, newChar7); //논블럭
	lcd.clear(0);
	lcd.home();
	plash();
}
int DEBUG_print() {
	/*
	============================
	Mode: Colling
	Top: 123.12
	Bot: 123.12
	Max: 123
	Min: -123
	============================
	*/
	Serial.println("============================");
	Serial.print("Mode: ");
	if (mode == 1) Serial.println("Heating");
	else if (mode == 2) Serial.println("Colling");
	Serial.print("Top: ");
	Serial.println(Sensor_bot.getTemp());
	Serial.print("Bot: ");
	Serial.println(Sensor_top.getTemp());
	Serial.print("Max: ");
	Serial.println((Temp_top.set > Temp_bot.set) ? Temp_top.set : Temp_bot.set);
	Serial.print("Min: ");
	Serial.println((Temp_top.set < Temp_bot.set) ? Temp_top.set : Temp_bot.set);
	Serial.println("============================");
}

void loop() {
	Check_encoder();
	//Serial.println(digitalRead(4));
	if (Time <= millis()) {
		LCD_print(); //1초마다 리프레쉬
		Check_temp(); //1초바다 온도체크
		Time = millis() + 200;
	}
	if (DEBUG&&Time2 <= millis()) {
		DEBUG_print();
		Time2 = millis() + 1000;
	}
	if (Time_plash <= millis()) {
		digitalWrite(13, LOW);
	}
}