//Basiert auf http://strofoland.com/stf-resources/A5020_OptMouse_1.ino

#define SCLK 28 // orange
#define SDIO 26 // gelb
#define NCS 22  // blau 
#define NRESET 24 // grün

#define REG_PRODUCT_ID 0x00
#define REG_REVISION_ID 0x01
#define REG_MOTION 0x02
#define REG_DELTA_X 0x03
#define REG_DELTA_Y 0x04
#define REG_SQUAL 0x05
#define REG_BURST_MODE 0x63
#define REG_PIXEL_GRAB 0x0b
#define REG_PIXEL_SUM 0x09

void setup() {
  Serial.begin(250000);
  
  reset(); 
  Serial.println("Read Registers");
  writeRegister(0x0d, 0x01); //set Resolution 1000cpi
}

void loop() {
  
  /*for (unsigned int i = 0; i < 14; i++) {
    int data = readRegister (i);
    Serial.print(i);
    Serial.print(": ");
    Serial.println(data);
    Serial.println(" ");
  }
  */

    byte shutter_up = readRegister(0x06);
    byte shutter_low = readRegister(0x07);
    byte squal = readRegister(0x05);
    byte pixel_sum = readRegister(0x09);


    Serial.print("SQUAL: ");
    Serial.println(squal);
    /*Serial.print("Shutter_Upper: ");
    Serial.print(shutter_up);
    Serial.print("      ");
    Serial.print("Shutter_Lower: ");
    Serial.println(shutter_low);
    */
    Serial.print("Pixel_Sum: ");
    Serial.println(pixel_sum);
    delay(50);
}

void reset() {
  pinMode(SCLK, OUTPUT);
  pinMode(SDIO, INPUT);
  pinMode(NCS, OUTPUT);
  pinMode(NRESET, OUTPUT);
    
  digitalWrite(SCLK, LOW);
  digitalWrite(NCS, LOW);
  digitalWrite(NRESET, HIGH);
  delayMicroseconds(100);
  
  // Initiate chip reset
  digitalWrite(NRESET, LOW);
  pushByte(0xfa);
  pushByte(0x5a);
  digitalWrite(NRESET, HIGH);
  
  // Set 1000cpi resolution
  digitalWrite(NRESET, LOW);
  //writeRegister(0x0d, 0x01);
  pushByte(0x0d);
  pushByte(0x01); // 1000cpi
  //pushByte(0x00); // 500cpi
  digitalWrite(NRESET, HIGH);
}

byte pullByte() {
  pinMode (SDIO, INPUT);

  delayMicroseconds(100); // tHOLD = 100us min.
  
  byte res = 0;
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK, LOW);
    res |= i * digitalRead (SDIO);
    delayMicroseconds(100);
    digitalWrite (SCLK, HIGH);
  }

  return res;
}

void pushByte(byte data){
  pinMode (SDIO, OUTPUT);
  
  delayMicroseconds(100); // tHOLD = 100us min.
  
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK, LOW);
    digitalWrite (SDIO, (data & i) != 0 ? HIGH : LOW);
    delayMicroseconds(100);
    digitalWrite (SCLK, HIGH);
    
    //Serial.print((data & i) != 0 ? HIGH : LOW, BIN);
  }
  //Serial.println("");
}

byte readRegister(byte address) {
  address &= 0x7F; // MSB indicates read mode: 0
  
  pushByte(address);
  
  byte data = pullByte();
  
  return data;  
}

void writeRegister(byte address, byte data) {
  address |= 0x80; // MSB indicates write mode: 1
  
  pushByte(address);
  
  delayMicroseconds(100);
  
  pushByte(data);

  delayMicroseconds(100); // tSWW, tSWR = 100us min.
}
