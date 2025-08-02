/*
will get data from laptop one byte at a time, will draw each byte on the LED Matrix, 
it will know its position by using two counters, one will count the row position, the other will count the 
column position


*/ 



#include <Adafruit_Protomatter.h> 



uint8_t rgbPins[] = {7,8,9,10,11,12}; 
uint8_t addrPins[] = {17,18,19,20,21}; 
uint8_t clockPin = 14; 
uint8_t latchPin = 15; 
uint8_t oePin = 16; 
int colNum = 0;  
int rowNum = 0; 
uint8_t pixelVal; 
uint8_t white = 255; 
uint8_t black = 0; 
uint8_t shiftNum = 128; //10000000 in binary, will be shifted by 1 each time to check each spot in the byte
byte frame[64]; 
bool frameFull = false; 
//5 address lines since its a 64x64 matrix
Adafruit_Protomatter matrix(64, 4, 1, rgbPins, 5, addrPins, clockPin, latchPin, oePin, false); 


void setup() {
  Serial.begin(115200); 
  while(!Serial); //waits for Serial to set up
  ProtomatterStatus status = matrix.begin(); 
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
    if(status != PROTOMATTER_OK) {
     for(;;); //infinite loop while protomatter isnt okay
     }

}

void loop() { 
  if (Serial.available() >= 64){
   //  pixelVal = Serial.read(); //recieves single byte representinng 8 bits
       Serial.readBytes((char*)frame, 64); //stores 64 bytes at a time at a 8 bit compression or 512 pixels total 
      for (int j = 0; j < 64; j++){
      pixelVal = frame[j]; 
     for (int i = 0; i < 8; i++){
       
        //if the ith bit in the recieved byte is a 1 then draw a white pixel
        if ((pixelVal & (shiftNum >> i)) == (shiftNum >> i)){
          matrix.drawPixel(colNum, rowNum, matrix.color565(white, white,  white)); 
          colNum += 1; 
        }
        else {
          matrix.drawPixel(colNum, rowNum, matrix.color565(black, black, black));
          colNum += 1;  
        }
        if ((colNum % 64) == 0){
          colNum = 0; 
          rowNum += 1; 
        }

        if ((rowNum % 64) == 0){
          rowNum = 0; 
          matrix.show(); 
        }
     }
  }
 
  }

  
  } 

