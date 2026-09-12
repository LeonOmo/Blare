
#include <Adafruit_GFX.h> // graphics library
#include <Adafruit_ST7789.h> // driver for the ST7789 screen
#include <SPI.h> 

// Defining pins for the display, change according to your setup!!! Uses the white numbers on the ESP
#define TFT_SCLK 0 // labeled SCL on the screen
#define TFT_MOSI 1 // labeled SDA on the screen
#define TFT_RST 2
#define TFT_DC 3
#define TFT_CS 4
#define TFT_BL 5

// Fix setColRowStart() by exposing it via a subclass
class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int hours = 12, minutes = 0, seconds = 0;
int alarmHours = 7, alarmMinutes = 0;
bool alarmEnabled = false, alarmRinging = false;
int mode = 0;

unsigned long previousMillis = 0;

void updateDisplay();
void handleButtons();
void printFormattedTime(int h, int m, int s);

// setup() runs ONCE when the board powers on
void setup() {
  Serial.begin(115200); // lets the board talk to your computer

  pinMode(TFT_BL, OUTPUT); // Set the backlight pin mode, or just wire it to 3.3V
  digitalWrite(TFT_BL, LOW); // Turns the backlight ON, for some reason this screen is active Low, so setting it to LOW is really HIGH

  tft.init(76, 284); // Our panel size (portrait)
  tft.setOffsets(82, 18); // Offsets for the weird resolution
  tft.invertDisplay(false); // Invert the colors (This display is flipped from normal)
  tft.setRotation(1); // Landscape, if it's upside down use 3!
  Serial.println("TFT Initialized!");

  tft.fillScreen(ST77XX_BLACK); // clear the screen

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(6);
  tft.setCursor(0,0); // Where the text is drawn, 0,0 is top left
  tft.print(Omo); // Show whatever you want! Draws from the top left of the text/number/shape 

  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ALARM, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
}

// loop() runs OVER and OVER, forever
void loop() {

unsigned long currentMillis = millis();

if (currentMillis - previousMillis >= 1000) {
  previousMillis += 1000;

  seconds++;
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours = (hours + 1) % 24;
      }
    }
  
    if (alarmEnabled && hours == alarmHours && minutes == alarmMinutes && seconds == 0) {
      alarmRinging = true;
    }

    updateDisplay();
  }

  if (alarmRinging) {
    tone(BUZZER, 1000, 200);
  }

  handleButtons();
}

void handleButtons() {

  if (digitalRead(BTN_MODE) == LOW) {
    if (alarmRinging) {
      alarmRinging = false;
      noTone(BUZZER);
    } else {
      mode = (mode + 1) % 3;
    }
    delay(200);
    updateDisplay();
  }

  if (digitalRead(BTN_UP) == LOW) {
    if (mode == 1) minutes = (minutes + 1) % 60;
    else if (mode == 2) alarmMinutes = (alarmMinutes + 1) % 60;
    delay(150);
    updateDisplay();
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    if (mode == 1) hours = (hours + 1) % 24;
    else if (mode == 2) alarmHours = (alarmHours + 1) % 24;
    delay(150);
    updateDisplay();
  }

  if (digitalRead(BTN_ALARM) == LOW) {
    if (alarmRinging) {
      alarmRinging = false;
      noTone(BUZZER);
    } else {
      alarmEnabled = !alarmEnabled;
    }
    delay(200);
    updateDisplay();
  }
}

void updateDisplay() {
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(4);
  tft.setCursor(20, 10);

  if (mode == 2) {
    printFormattedTime(alarmHours, alarmMinutes, -1);
  } else {
    printFormattedTime(hours, minutes, seconds);
  }

  tft.setTextSize(2);
  tft.setCursor(20, 50);
  
  if (mode == 1) {
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.print("SET TIME     ");
  } else if (mode == 2) {
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.print("SET ALARM    ");
  } else {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    if (alarmEnabled) {
      tft.print("ALARM: ON  (");
      if (alarmHours < 10) tft.print("0");
      tft.print(alarmHours);
      tft.print(":");
      if (alarmMinutes < 10) tft.print("0");
      tft.print(alarmMinutes);
      tft.print(")");
    } else {
      tft.print("ALARM: OFF         ");
    }
  }
}

void printFormattedTime(int h, int m, int s) {
  if (h < 10) tft.print("0");
  tft.print(h);
  tft.print(":");
  if (m < 10) tft.print("0");
  tft.print(m);
  
  if (s >= 0) {
    tft.print(":");
    if (s < 10) tft.print("0");
    tft.print(s);
  }
}
