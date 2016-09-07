#define SCLK1 28
#define SDIO1 26  
#define NRESET1 24
#define NCS1 22 
#define SCLK2 52
#define SDIO2 50 
#define NRESET2 48
#define NCS2 46 
#define REG_PRODUCT_ID 0x00
#define REG_REVISION_ID 0x01
#define REG_MOTION 0x02
#define REG_DELTA_X 0x03
#define REG_DELTA_Y 0x04
#define REG_SQUAL 0x05
#define REG_BURST_MODE 0x63
#define REG_PIXEL_GRAB 0x0b

float x = 0;
float y = 0; 
float x1 = 0; 
float y1 = 0; 
float x2 = 0;
float y2 = 0;
float xi1 = 0;
float yi1 = 0;
float xi2 = 0;
float yi2 = 0;
float d = 859; 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
/////////////////////

byte pullByte1() {
  pinMode (SDIO1, INPUT);

  delayMicroseconds(100); // tHOLD = 100us min.
  
  byte res1 = 0;
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK1, LOW);
    res1 |= i * digitalRead (SDIO1);
    delayMicroseconds(100);
    digitalWrite (SCLK1, HIGH);
  }

  return res1;
}

byte pullByte2() {
  pinMode (SDIO2, INPUT);

  delayMicroseconds(100); // tHOLD = 100us min.
  
  byte res2 = 0;
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK2, LOW);
    res2 |= i * digitalRead (SDIO2);
    delayMicroseconds(100);
    digitalWrite (SCLK2, HIGH);
  }

  return res2;
}

void pushByte1(byte data1){
  pinMode (SDIO1, OUTPUT);
  
  delayMicroseconds(100); // tHOLD = 100us min.
  
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK1, LOW);
    digitalWrite (SDIO1, (data1 & i) != 0 ? HIGH : LOW);
    delayMicroseconds(100);
    digitalWrite (SCLK1, HIGH);
    
  }
}

void pushByte2(byte data2){
  pinMode (SDIO2, OUTPUT);
  
  delayMicroseconds(100); // tHOLD = 100us min.
  
  for (byte i=128; i >0 ; i >>= 1) {
    digitalWrite (SCLK2, LOW);
    digitalWrite (SDIO2, (data2 & i) != 0 ? HIGH : LOW);
    delayMicroseconds(100);
    digitalWrite (SCLK2, HIGH);
    
  }
}

byte readRegister1(byte address1) {
  address1 &= 0x7F; // MSB indicates read mode: 0
  
  pushByte1(address1);
  
  byte data1 = pullByte1();
  
  return data1;  
}

byte readRegister2(byte address2) {
  address2 &= 0x7F; // MSB indicates read mode: 0
  
  pushByte2(address2);
  
  byte data2 = pullByte2();
  
  return data2;  
}


void writeRegister1(byte address1, byte data1) {
  address1 |= 0x80; // MSB indicates write mode: 1
  
  pushByte1(address1);
  
  delayMicroseconds(100);
  
  pushByte1(data1);

delayMicroseconds(100); // tSWW, tSWR = 1
}

void writeRegister2(byte address2, byte data2) {
  address2 |= 0x80; // MSB indicates write mode: 1
  
  pushByte2(address2);
  
  delayMicroseconds(100);
  
  pushByte2(data2);

delayMicroseconds(100); // tSWW, tSWR = 1
}

void reset() {
  pinMode(SCLK1, OUTPUT);
  pinMode(SDIO1, INPUT);
  pinMode(NCS1, OUTPUT);
  pinMode(NRESET1, OUTPUT);
  pinMode(SCLK2, OUTPUT);
  pinMode(SDIO2, INPUT);
  pinMode(NCS2, OUTPUT);
  pinMode(NRESET2, OUTPUT);
    
  digitalWrite(SCLK1, LOW);
  digitalWrite(NCS1, LOW);
  digitalWrite(NRESET1, HIGH);
  digitalWrite(SCLK2, LOW);
  digitalWrite(NCS2, LOW);
  digitalWrite(NRESET2, HIGH);
  delayMicroseconds(100);
  
  // Initiate chip reset
  digitalWrite(NRESET1, LOW);
  pushByte1(0xfa);
  pushByte1(0x5a);
  digitalWrite(NRESET1, HIGH);

  digitalWrite(NRESET2, LOW);
  pushByte2(0xfa);
  pushByte2(0x5a);
  digitalWrite(NRESET2, HIGH);
  
  // Set resolution to 1000cpi
  //writeRegister1(0x0d, 0x01); // Sensor1
  //writeRegister2(0x0d, 0x01); // Sensor2
  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SETUP
/////////////////////
void setup() {
   Serial.begin(9600);
  
  reset();
  byte productId1 = readRegister1(REG_PRODUCT_ID);
  byte revisionId1 = readRegister1(REG_REVISION_ID);
  int Resolution1 = readRegister1(0x0d);
  if (Resolution1 = 1){
    Resolution1 = 1000;
  }
    else{
    Resolution1 = 500;
    
  byte productId2 = readRegister2(REG_PRODUCT_ID);
  byte revisionId2 = readRegister2(REG_REVISION_ID);
  int Resolution2 = readRegister2(0x0d);
  if (Resolution2 = 1){
    Resolution2 = 1000;
  }
    else{
    Resolution2 = 500;
  }
  
  
  Serial.println("ProductId Sensor1: ");
  Serial.print(productId1, HEX);
  Serial.print(", rev1. ");
  Serial.println(revisionId1, HEX);
  Serial.print ("Resolution1, ");  
  Serial.print(Resolution1, DEC);
  Serial.println("cpi");
  Serial.println("");
  Serial.println("ProductId Sensor2: ");
  Serial.print(productId2, HEX);
  Serial.print(", rev2. ");
  Serial.println(revisionId2, HEX);
  Serial.print ("Resolution2, ");  
  Serial.print(Resolution2, DEC);
  Serial.println("cpi");
  
}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////
void dumpDelta1() {
  byte squal1 = readRegister1(REG_SQUAL);
  char motion1 = readRegister1(REG_MOTION); // Freezes DX and DY until they are read or MOTION is read again.
  int dx1 = readRegister1(REG_DELTA_X);
  int dy1 = readRegister1(REG_DELTA_Y);
  byte shutter_up1 = readRegister1(0x06);
  byte shutter_low1 = readRegister1(0x07);
  byte pixelsum1 = readRegister1(0x09);

  if (dx1 > 128) {   //Information über die Richtung
    dx1 = dx1 - 256;
  }

  if (dy1 > 128) {
    dy1 = dy1 - 256;
  }

  xi1 += dx1; //Ganze Zahlen addieren, statt Gleitkommazahlen, später erst multiplizieren -> weniger Genauigkeitsverluste durch Rundung
  yi1 += dy1;
  
  x1 = xi1 *0.046355; //für 500cpi - 0.046mm pro Count, für 1000cpi - 0.02247mm pro Count (auf kariertem Blockpapier)
  y1 = yi1 *0.046355;
  
  

  if (/*shutter_up > 1 | shutter_low < 10*/pixelsum1 < 40) {  //Grenzwert abhängig von der Oberfläche
    y1 = 0; 
    x1 = 0;
  }

  float a1 = atan(dy1/dx1);
  float s1 = y1*sin(a1); 
  float phi1 = (s1/d)*360;
  
  
}
void dumpDelta2() {
  byte squal2 = readRegister2(REG_SQUAL);
  char motion2 = readRegister2(REG_MOTION); // Freezes DX and DY until they are read or MOTION is read again.
  int dx2 = readRegister2(REG_DELTA_X);
  int dy2 = readRegister2(REG_DELTA_Y);
  byte shutter_up2 = readRegister2(0x06);
  byte shutter_low2 = readRegister2(0x07);
  byte pixelsum2 = readRegister2(0x09);

  if (dx2 > 128) {   //Information über die Richtung
    dx2 = dx2 - 256;
  }

  if (dy2 > 128) {
    dy2 = dy2 - 256;
  }
 
  xi2 += dx2;
  yi2 += dy2;
  
  x2 += xi2 *0.046355; //für 500cpi - 0.046mm pro Count, für 1000cpi - 0.02247mm pro Count (auf kariertem Blockpapier)
  y2 += yi2 *0.046355;
  
  

  if (/*shutter_up > 1 | shutter_low < 10*/pixelsum2 < 40) {  //Grenzwert abhängig von der Oberfläche
    y2 = 0; 
    x2 = 0;

  }

 float a2 = atan(dy2/dx2);
 float s2 = y2*sin(a2); 
 float phi2 = (s2/d)*360; 
  
}  

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LOOP
///////////////////////

void loop() {
  dumpDelta1();
  dumpDelta2();
      
  
  
  Serial.print("Motion1: ");
  Serial.print(motion1 && 1);
  Serial.print("  ");
  Serial.print("SQUAL1: ");
  Serial.print(squal1, DEC);
  Serial.print("      ");
  Serial.print("Motion2: ");
  Serial.print(motion2 && 1);
  Serial.print("  ");
  Serial.print("SQUAL2: ");
  Serial.println(squal2, DEC);
  /*
  Serial.print(" DELTA X1: ");
  Serial.print(dx1, DEC);
  Serial.print("      ");
  Serial.print("   DELTA Y1: ");
  Serial.println(dy1, DEC);
  */  
  Serial.print("Gesamt X1: ");
  Serial.print(x1); Serial.print("mm");
  Serial.print("      ");
  Serial.print("Gesamt X2: ");
  Serial.print(x2); Serial.println("mm");
  Serial.print("Gesamt Y1: ");
  Serial.print(y1); Serial.print("mm");
  Serial.print("      ");
  Serial.print("Gesamt Y2: ");
  Serial.print(y2); Serial.println("mm");
  Serial.println(" ");
  /*
  Serial.print(" DELTA X2: ");
  Serial.print(dx2, DEC);
  Serial.print("      ");
  Serial.print("   DELTA Y2: ");
  Serial.println(dy2, DEC);
  */  
    
    //delay(1000);
}
