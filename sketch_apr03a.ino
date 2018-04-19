
#include <Wire.h> 
#include <Keyboard.h>

//Set this to 1 to test using the serial monitor
#define debug 0

//This is the address range of the expander
#define PCA9555_Start 0x20 // address for PCA9555
#define PCA9555_End 0x23

//Stores the previous key state
int prevState[8][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};

//This is the key to send to computer. Might change this to string later so that we can send a string of keystrokes.
char charMap[8][16] = {{'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','L','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'}};

//So far we only have M for momentary
char keyType[8][16] = {{'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','M','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'}};

void setup()
{
  Serial.begin(9600);
  for (int i=PCA9555_Start; i<=PCA9555_End ;i++)
  {
    Wire.begin(i);
  }  
  Keyboard.begin();
}

void loop()
{ 
  unsigned char key;
  key=readdata();
}

unsigned char readdata(void)          //main read function
{
  for (int i=PCA9555_Start ;i<=PCA9555_End ;i++) //for loop
  {
    for (int p=0;p<2;p++) {   
         gpio_read(i,p);
    }
  }
}

unsigned int gpio_read(int address, int port)
{
  int data = 0;                   
  Wire.beginTransmission(address);
  Wire.write(port); 
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 1);
  if (Wire.available()) 
  {   
    data = Wire.read( ); // read lower 4 bit  0.3 ~ 0.0       
  }
  Wire.endTransmission();
  if (prevState[address - PCA9555_Start][port] != data) { 
    int changed = prevState[address - PCA9555_Start][port] ^ data;
    int bitNum = 0;
    bool down = false;
    for (int bit=1;bit<=128;bit=bit * 2) {
      if (byte(bit & changed) != 0x00) {
        int row = address - PCA9555_Start;
        int col = (bitNum) + (8 * port);
        if (byte(bit & data) == 0x00) {
          down = true;
        }
        else {
          down = false;
        }
        keySend(down, row, col);
      }
      bitNum++;
    }
    prevState[address - PCA9555_Start][port] = data;
  }
  return data;
}

void keySend(int down, int row, int col) {
  char key = charMap[row][col];
  char type = keyType[row][col];
  if (debug){
    if (down) {
      Serial.print("KeyDown:");
    } else {
      Serial.print("KeyUp:");
    }
    Serial.print("row:"); 
    Serial.print(row); 
    Serial.print(":col:");
    Serial.print(col); 
    Serial.print(":char:");
    Serial.println(key); 
  } else {
    switch (type) {
      case 'M':
        if (down) {
          Keyboard.press(key);
        } else {
          Keyboard.release(key);
        }
        break;
    }
  }
}



