#include<Adafruit_GFX.h>
#include<Wire.h>
#include<Adafruit_SH110X.h>

Adafruit_SH1106G display(128, 64, &Wire, -1);

void setup() {
 display.begin(0x3C, true);
 display.clearDisplay();
 display.display();
 
draw();
}

void loop() {

}

static const unsigned char PROGMEM image_download_1_bits[] = {
  0x04,0x00,0x1c,0x0e,0x38,0x02,0x78,0x04,0x71,0xee,0xf0,0x40,0xf0,0x80,0xf1,0xe0,
  0xf8,0x00,0xf8,0x06,0x7e,0x1c,0x7f,0xfc,0x3f,0xf8,0x1f,0xf0,0x07,0xc0,0x00,0x00};

static const unsigned char PROGMEM image_download_2_bits[] = {
  0x01,0x00,0x21,0x08,0x10,0x10,0x03,0x80,0x8c,0x62,0x48,0x24,0x10,0x10,0x10,0x10,
  0x10,0x10,0x48,0x24,0x8c,0x62,0x03,0x80,0x10,0x10,0x21,0x08,0x01,0x00,0x00,0x00};

static const unsigned char PROGMEM image_download_bits[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0xb6,0xdb,0x68,0xb6,
  0xdb,0x6e,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,
  0x61,0xb6,0xdb,0x6e,0xb6,0xdb,0x68,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};

int Layer_11_radius = 2;

void draw(void) {
    display.clearDisplay();

    // Layer 1
    display.drawLine(0, 17, 127, 17, 1);

    // download
    display.drawBitmap(100, 0, image_download_bits, 24, 16, 1);

    // Layer 4
    display.setTextColor(1);
    display.setTextWrap(false);
    display.setCursor(3, 0);
    display.print("Mon");

    // Layer 5
    display.setCursor(3, 8);
    display.print("00-00-00");

    // Layer 7
    display.drawRect(99, 21, 11, 40, 1);

    // Layer 8
    display.drawRect(113, 21, 11, 40, 1);

    // Layer 8
    display.setTextSize(3);
    display.setCursor(4, 20);
    display.print("00");

    // Layer 9
    display.setCursor(4, 43);
    display.print("00");

    // Layer 11
    display.drawRoundRect(44, 21, 52, 40, Layer_11_radius, 1);

    // Layer 12
    display.drawRect(47, 24, 7, 7, 1);

    // Layer 13
    display.drawRect(47, 33, 7, 7, 1);

    // Layer 14
    display.drawRect(47, 42, 7, 7, 1);

    // Layer 15
    display.drawRect(47, 51, 7, 7, 1);

    // Layer 16
    display.setTextSize(1);
    display.setCursor(57, 24);
    display.print("Task 1");

    // Layer 17
    display.setCursor(57, 33);
    display.print("Task 2");

    // Layer 18
    display.setCursor(57, 42);
    display.print("Task 3");

    // download
    display.drawBitmap(79, 1, image_download_1_bits, 15, 16, 1);

    // Layer 19
    display.setCursor(57, 51);
    display.print("Task 4");

    // download
   // display.drawBitmap(58, 1, image_download_2_bits, 15, 16, 1);

    display.display();
}
