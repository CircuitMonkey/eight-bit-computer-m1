/*

   Programs the ROM for the output register.

   Solstice StarFire module modified version:
   From original Ben Eater code (using Arduino and a different circuit).
   https://github.com/beneater/eeprom-programmer/blob/master/multiplexed-display/multiplexed-display.ino


   @author Maehem


   Pins:

    - !MCE  A0        OUTPUT
    - !MOE  A1        OUTPUT
    - ADR9  A2        OUTPUT
    - ADR8  A3        OUTPUT
    - !WE   A4        OUTPUT
    - !PASS/FAIL  A5  OUTPUT  (Set as input to turn off)
    - !PROG 13        OUTPUT
    - !SOE  12        OUTPUT
    - !RST  11        OUTPUT
    - LAT   10        OUTPUT
    - SHFT   9        OUTPUT
    - SADDR  8        OUTPUT

    - D[0:7]  0:7
*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN    SCK
#define LED_COUNT 16
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#define RED strip.Color(255,   0,   0)
#define GRN strip.Color(  0, 255,   0)
#define YEL strip.Color(255, 255,   0)

#define MCE_ A0
#define MOE_ A1
#define ADR10 12
#define ADR9 A2
#define ADR8 A3
#define WRITE_EN  A4
#define FAIL A5

#define SADDR  8
#define SHFT_CLK  9
#define SHFT_LAT  10
#define RST_  11
//#define SOE_  12
#define PROG_ 13

// Bit patterns for the digits 0..9
byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };


void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("ROM Programmer");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(MCE_, OUTPUT);
  pinMode(MOE_, OUTPUT);
  pinMode(WRITE_EN, OUTPUT);
  pinMode(SADDR, OUTPUT);
  pinMode(SHFT_LAT, OUTPUT);
  pinMode(SHFT_CLK, OUTPUT);
  pinMode(RST_, OUTPUT);
  //pinMode(SOE_, OUTPUT);
  pinMode(PROG_, OUTPUT);
  pinMode(ADR10, OUTPUT);
  pinMode(ADR9, OUTPUT);
  pinMode(ADR8, OUTPUT);

  pinMode(FAIL, OUTPUT);
  digitalWrite(FAIL, 1);

  //strip.setPixelColor(0, GRN);         //  Set pixel's color (in RAM)
  //strip.show();

  // PROM state
  digitalWrite(MCE_, HIGH);
  digitalWrite(MOE_, HIGH);
  digitalWrite(MCE_, HIGH);
  digitalWrite(WRITE_EN, HIGH);

  // Shift Register State
  //digitalWrite(SOE_, HIGH);
  digitalWrite(RST_, HIGH);
  digitalWrite(SHFT_CLK, HIGH);
  digitalWrite(SHFT_LAT, HIGH);


  Serial.println("Programming ones place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value, digits[value % 10]);
  }
  strip.setPixelColor(0, RED);         //  Set pixel's color (in RAM)
  strip.show();


  strip.setPixelColor(1, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming tens place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value + 0x0100, digits[(value / 10) % 10]);
  }
  strip.setPixelColor(1, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(2, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming hundreds place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value + 0x0200, digits[(value / 100) % 10]);
  }
  strip.setPixelColor(2, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(3, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming sign");
  for (int value = 0; value <= 255; value ++) {
    writeEEPROM(value + 0x0300, 0);
  }
  strip.setPixelColor(3, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(4, YEL);         //  Set pixel's color (in RAM)
  strip.show();



  Serial.println("Programming ones place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 0x400, digits[abs(value) % 10]);
  }
  strip.setPixelColor(4, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(5, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming tens place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 0x500, digits[abs(value / 10) % 10]);
  }
  strip.setPixelColor(5, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(6, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming hundreds place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 0x600, digits[abs(value / 100) % 10]);
  }
  strip.setPixelColor(6, RED);         //  Set pixel's color (in RAM)
  strip.show();

  strip.setPixelColor(7, YEL);         //  Set pixel's color (in RAM)
  strip.show();
  Serial.println("Programming sign (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    if (value < 0) {
      writeEEPROM((byte)value + 0x700, 0x01);
    } else {
      writeEEPROM((byte)value + 0x700, 0);
    }
  }
  strip.setPixelColor(7, RED);         //  Set pixel's color (in RAM)
  strip.show();

  delay(2000);

  boolean pass;

  pass = true;
  // Verify
  Serial.println("Verifying ones place");
  strip.setPixelColor(15,  YEL);
  for (int value = 0; value <= 255; value++) {
    if ( readEEPROM(value) == digits[value % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(15, pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying tens place");
  pass = true;
  strip.setPixelColor(14,  YEL);
  for (int value = 0; value <= 255; value++) {
    if ( readEEPROM(value | 0x100) == digits[(value / 10) % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(14,  pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying hundreds place");
  pass = true;
  strip.setPixelColor(13,  YEL);
  for (int value = 0; value <= 255; value++) {
    if ( readEEPROM(value | 0x200) == digits[(value / 100) % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(13,  pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying sign");
  pass = true;
  strip.setPixelColor(12,  YEL);
  for (int value = 0; value <= 255; value++) {
    if ( readEEPROM(value | 0x300 ) == 0 ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(12,  pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();





  Serial.println("Verifying ones place (twos complement)");
  strip.setPixelColor(11,  YEL);
  pass=true;
  for (int value = -128; value <= 127; value += 1) {
    if ( readEEPROM((byte)value + 0x400) == digits[abs(value) % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(11, pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying tens place (twos complement)");
  strip.setPixelColor(10,  YEL);
  pass=true;
  for (int value = -128; value <= 127; value += 1) {
    if ( readEEPROM((byte)value + 0x500) == digits[abs(value / 10) % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(10, pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying hundreds place (twos complement)");
  strip.setPixelColor(9,  YEL);
  pass=true;
  for (int value = -128; value <= 127; value += 1) {
    if ( readEEPROM((byte)value + 0x600) == digits[abs(value / 100) % 10] ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(9, pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();

  Serial.println("Verifying sign place (twos complement)");
  strip.setPixelColor(8,  YEL);
  pass=true;
  for (int value = -128; value <= 127; value += 1) {
    int expected;
    if (value < 0) {
      expected = 0x01;
    } else {
      expected = 0;
    }
   
    if ( readEEPROM((byte)value + 0x700) == expected ) {
      Serial.println("  #");
    } else {
      Serial.println("  .");
      pass = false;
    }
  }
  strip.setPixelColor(8, pass ? GRN : RED);     //  Set pixel's color (in RAM)
  strip.show();



  Serial.println("...done!");

}

void loop() {
  // put your main code here, to run repeatedly:

}

/**
   Output the address bits and outputEnable signal using shift registers.
*/
void setAddress(int address, bool outputEnable) {
  // Set address bits 8 and 9
  digitalWrite(ADR10, address >> 10 & 0x01 );
  digitalWrite(ADR9, address >> 9 & 0x01 );
  digitalWrite(ADR8, address >> 8 & 0x01 );

  shiftOut(SADDR, SHFT_CLK, MSBFIRST, address & 0xFF);

  digitalWrite(SHFT_LAT, LOW);
  digitalWrite(SHFT_LAT, HIGH);
  digitalWrite(SHFT_LAT, LOW);

}

/*
   Write a byte to the EEPROM at the specified address.
*/
void writeEEPROM(int address, byte data) {
  Serial.print("Write addr:");
  Serial.print(address, HEX);
  Serial.print(" ==> ");
  if ( data < 0b10000000 ) {
    Serial.print("0");
  }
  if ( data < 0b01000000 ) {
    Serial.print("0");
  }
  if ( data < 0b00100000 ) {
    Serial.print("0");
  }
  if ( data < 0b00010000 ) {
    Serial.print("0");
  }
  if ( data < 0b00001000 ) {
    Serial.print("0");
  }
  if ( data < 0b00000100 ) {
    Serial.print("0");
  }
  if ( data < 0b00000010 ) {
    Serial.print("0");
  }

  Serial.println( data, BIN );

  setAddress(address, /*outputEnable*/ false);
  //digitalWrite(SOE_, LOW);

  for (int pin = 0; pin <= 7; pin++) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = 0; pin <= 7; pin ++) {
    digitalWrite(pin, data & 0x01);
    data = data >> 1;
  }

  digitalWrite(MCE_, LOW);
  digitalWrite(WRITE_EN, LOW);

  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delayMicroseconds(2);
  digitalWrite(MCE_, HIGH);
  delayMicroseconds(1);

//  digitalWrite(SOE_, HIGH);

  for (int pin = 0; pin <= 7; pin++) {
    pinMode(pin, INPUT);
  }

  delay(10);
}

byte readEEPROM(int address) {
  Serial.print("Read addr:");
  Serial.print(address, HEX);
  //  Serial.print(" :: ");
  //  Serial.println(data, BIN );

  setAddress(address, /*outputEnable*/ false);
  //digitalWrite(SOE_, LOW);

  for (int pin = 0; pin <= 7; pin++) {
    pinMode(pin, INPUT);
  }


  digitalWrite(MCE_, LOW);
  digitalWrite(MOE_, LOW);

  delayMicroseconds(1);

  byte data = 0;
  for (int pin = 0; pin <= 7; pin ++) {
    data |= digitalRead(pin) << pin;
  }

  digitalWrite(MOE_, HIGH);
  delayMicroseconds(2);
  digitalWrite(MCE_, HIGH);
  delayMicroseconds(1);

//  digitalWrite(SOE_, HIGH);


  Serial.print(" ==> ");
  if ( data < 0b10000000 ) {
    Serial.print("0");
  }
  if ( data < 0b01000000 ) {
    Serial.print("0");
  }
  if ( data < 0b00100000 ) {
    Serial.print("0");
  }
  if ( data < 0b00010000 ) {
    Serial.print("0");
  }
  if ( data < 0b00001000 ) {
    Serial.print("0");
  }
  if ( data < 0b00000100 ) {
    Serial.print("0");
  }
  if ( data < 0b00000010 ) {
    Serial.print("0");
  }

  Serial.print( data, BIN );
  delay(10);

  return data;
}
//// prints 8-bit data in hex
//void PrintHex83(uint8_t *data, uint8_t length)  {
//  char tmp[length * 2 + 1];
//  byte first ;
//  int j = 0;
//  for (uint8_t i = 0; i < length; i++)
//  {
//    first = (data[i] >> 4) | 48;
//    if (first > 57) tmp[j] = first + (byte)39;
//    else tmp[j] = first ;
//    j++;
//
//    first = (data[i] & 0x0F) | 48;
//    if (first > 57) tmp[j] = first + (byte)39;
//    else tmp[j] = first;
//    j++;
//  }
//  tmp[length * 2] = 0;
//  Serial.print(tmp);
//}
