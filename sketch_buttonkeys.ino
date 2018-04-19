
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
//Each expander has two ports of eight bits. I do maths to figure out which pin was activited and then use this
//to determine what to send.
char charMap[8][16] = {{'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','L','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'},
                      {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'}};

//This is a table to determine the behavior of the button. M presses the key as long as it is held down just
//like a keyboard. So far we only have M for momentary. Other behaviors will be added later.
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
  //We can use the serial monitor to figure out what expander and pin the button is pluggest into in 
  //debug mode
  if (debug) {
    Serial.begin(9600);
  }
  //Each expander as a unique address that it is set to
  for (int i=PCA9555_Start; i<=PCA9555_End ;i++)
  {
    Wire.begin(i);
  }  
  //Arduino Leondardos can act like HID devices and we can send keystrokes
  Keyboard.begin();
}

void loop()
{ 
  unsigned char key;
  //Read forever. Eventually there will be an hardware interrupt driven version of this where reads only occur when 
  //there is a pin state change. That will leave us time to do other things instead of just frantically checking for data
  key=readdata();
}

unsigned char readdata(void)          //main read function
{
  for (int i=PCA9555_Start ;i<=PCA9555_End ;i++) //for loop
  {
    //Each Expander has two ports
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
  //Read one byte from port on expander with address
  Wire.requestFrom(address, 1);
  if (Wire.available()) 
  {   
    data = Wire.read( ); 
  }
  Wire.endTransmission();
  //The data comes across as bits in a byte. We check for a change from the previous state. If there is a change
  //we do stuff. We use bitwise logic to figure what bits have been set or unset. Unset is a keydown event. Set is a keyup event.
  if (prevState[address - PCA9555_Start][port] != data) { 
    int changed = prevState[address - PCA9555_Start][port] ^ data;
    int bitNum = 0;
    bool down = false;
    for (int bit=1;bit<=128;bit=bit * 2) {
      if (byte(bit & changed) != 0x00) {
        //We figure out where in the table this bit maps to to figure out the key and behavior.
        //It should be pretty efficient.
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
  //If debug is set we just print out data about this bit to the serial monitor so we can create a mapping in the tables
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
      //This should behave like a keyboard key. Press a button is pressed until you release. I may have to look into deboucing
      //but I haven't had issues yet.
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



