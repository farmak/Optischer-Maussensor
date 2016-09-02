//Basiert auf http://strofoland.com/stf-resources/A5020_OptMouse_1.ino

#define SCLK 52
#define SDIO 50 
#define NCS 46  
#define NRESET 48 
#define REG_PRODUCT_ID 0x00
#define REG_REVISION_ID 0x01
#define REG_MOTION 0x02
#define REG_DELTA_X 0x03
#define REG_DELTA_Y 0x04
#define REG_SQUAL 0x05
#define REG_BURST_MODE 0x63
#define REG_PIXEL_GRAB 0x0b

//#define FRAME_LENGTH 225


//byte frame[256];
float x = 0; 
float y = 0; 

void setup() {
   Serial.begin(250000);
  
  reset();
  byte productId = readRegister(REG_PRODUCT_ID);
  byte revisionId = readRegister(REG_REVISION_ID);
  int Resolution = readRegister(0x0d);
  if (Resolution = 1){
    Resolution = 1000;
  }
    else{
    Resolution = 500;
  }
  
  
  Serial.println("Found productId ");
  Serial.print(productId, HEX);
  Serial.print(", rev. ");
  Serial.println(revisionId, HEX);
  Serial.print ("Resolution, ");  
  Serial.print(Resolution, DEC);
  Serial.println("cpi");
  
}

void loop() {
    dumpDelta();
    //dumpFrame();
    
    delay(10);
}
/*
void dumpFrame(){
    byte frame[FRAME_LENGTH];
        
    for(int idx = 0; idx < FRAME_LENGTH; idx++){
      frame[idx] = readRegister(REG_PIXEL_GRAB);
      
      if((frame[idx] & 0x80) != 0x80)
        idx--;
      else 
        frame[idx] &= 0x7F;

      // todo - check if is valid
      Serial.println(frame[idx], DEC);
    }
    
    Serial.println("--------"); 
    Serial.println("--------"); 
}
*/
void dumpDelta() {
  byte squal = readRegister(REG_SQUAL);
  char motion = readRegister(REG_MOTION); // Freezes DX and DY until they are read or MOTION is read again.
  int dx = readRegister(REG_DELTA_X);
  int dy = readRegister(REG_DELTA_Y);
  byte shutter_up = readRegister(0x06);
  byte shutter_low = readRegister(0x07);
  byte pixelsum = readRegister(0x09);

  if (dx > 128) {   //Information über die Richtung
    dx = dx - 256;
  }

  if (dy > 128) {
    dy = dy - 256;
  }
  
  x += dx*0.046355; //für 500cpi - 0.046mm pro Count, für 1000cpi - 0.02247mm pro Count (auf kariertem Blockpapier)
  y += dy*0.046355;
  
  double arc = 0; 
//  arc += sqrt((dx*0.046)^2+(dy*0.046)^2);
  

  if (/*shutter_up > 1 | shutter_low < 10*/pixelsum < 40) {  //Grenzwert abhängig von der Oberfläche
    y = 0; 
    x = 0;
   

  }

  Serial.print("SQUAL: ");
  Serial.print(squal, DEC);
  Serial.print("      ");
  Serial.print("Motion: ");
  Serial.println(motion && 1);
  Serial.print(" DELTA X: ");
  Serial.print(dx, DEC);
  Serial.print("      ");
  Serial.print("   DELTA Y: ");
  Serial.println(dy, DEC);  
  Serial.print("Gesamt X: ");
  Serial.print(x, DEC); Serial.print("mm");
  Serial.print("      ");
  Serial.print("Gesamt Y: ");
  Serial.print(y); Serial.println("mm");
  //Serial.print("arc: "); Serial.print(arc, DEC); Serial.println("mm");
  Serial.println(" ");
  
  

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
  
  /*/ Set 1000cpi resolution
  digitalWrite(NRESET, LOW);
  //writeRegister(0x0d, 0x01);
  pushByte(0x0d);
  pushByte(0x01); // 1000cpi
  //pushByte(0x00); // 500cpi
  digitalWrite(NRESET, HIGH);
  */
  //writeRegister(0x0d, 0x01); //set Resolution 1000cpi
  
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
