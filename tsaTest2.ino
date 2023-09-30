

//code by M.Angelini (really is a collage of copy paste of arduino sketch find on the web and adapted to my hardware)

#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

const char *ssid     = "SSID";
const char *password = "password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

uint16_t Connect[]={35,47,46,46,37,35,52,0};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


// Definitions 

//#define BUTTON_PIN  14      // pushbutton pin (this is also analog pin 0, but INTERNAL_PULLUP only works when you refer to this pin as '14')

#define SER_DATA_PIN  5     // serial data pin for VFD-grids shift register
#define SER_LAT_PIN  4      // latch pin for VFD-grids shift register
#define SER_CLK_PIN  0      // clock pin for VFD-grids shift register
#define DECODER_A  12      
#define DECODER_B  13       
#define DECODER_C  15
#define BUTTON  8      
//#define LONG_PUSH_TIME  60  // Time to register a long button press
#define DATE_DELAY 10000     // Delay before date is displayed (loop exectutions)
//#define SERIAL            // uncomment for debug

#define INTERVALLO 3000
unsigned long t0, dt;
uint16_t loopCounter = 0;

/******************************************************************************* 
* Digits are lit by shiting out one byte where each bit corresponds to a grid. 
* 1 = on; 0 = off;
* msb = leftmost digit grid;
* lsb = rightmost digit grid.
*******************************************************************************/

/******************************************************************************* 
* TSA1605A 14 segments segments are lit by shiting out one byte where each bit corresponds to a segment:
*                                                                                    
*  -----A----
*  |\   |   /|
*  F H  I  J B
*  |  \ | /  |
*  --G-----K--
*  |  / | \  |
*  E N  M  L C
*  /    |   \|
*  -----D---- 
* 
* 1 = on; 0 = off;
* 
* 
* // HC595s wiring:
// HC595-1, 2 & 3 pin 8 to GND
// HC595-1, 2 & 3 pin 10 to +5V
// HC595-1, 2 & 3 pin 16 to +5V
// HC595-1 QA to QH ( 15, 1, 2, 3, 4, 5, 6, 7) outputs --> TSA1605A cathodes  to 8 (pins 12, 11, 15, 8, 19, 3, 22 ,1)
// HC595-2 QC to QH ( 15, 1, 2, 3, 4, 5) outputs --> TSA1605A I to N segments  (pins 20,18,17,2,7,10)
// HC595-3 QA to QH ( 15, 1, 2, 3, 4, 5, 6, 7) outputs -->  TSA1605A A to H segments (pins 14, 16, 5, 4, 9, 21, 6,13)
// place one 68 ohms resistor between each display segment anodes and it's HC959 output.
*******************************************************************************/

// Mappings of bits to segments for the character map
#define   H   0x0001
#define   G   0x0002
#define   F   0x0004
#define   E   0x0008
#define   D   0x0010
#define   C   0x0020
#define   B   0x0040
#define   A   0x0080
#define   N   0x0400
#define   M   0x0800
#define   L   0x1000
#define   K   0x2000
#define   J   0x4000
#define   I   0x8000


// Build the character map
const uint16_t charactermap[64] PROGMEM = {
  0,                  // space
  D+I,                // exclamation point
  F+I,                // double quote       
  I+B+G+K+M+C,        // hash
  A+C+D+F+G+K+I+L,    // dollar
  F+H+G+J+N+K+L+C,    // percent
  A+L+H+J+D+M,        // ampersand
  B,                  // single quote
  J+L,                // left paranthesis
  H+N,                // right paranthesis
  G+K+H+J+I+L+M+N,    // asterisk
  G+K+I+M,            // plus
  N,                  // comma
  G+K,                // minus
  E,                  // dot
  J+N,                // slash
  A+B+C+D+E+F+J+N,    // zero
  I+M,                // one
  A+B+D+E+G+K,        // two
  A+B+C+D+G+K,        // three
  B+C+F+G+K,          // four
  A+C+D+F+G+K,        // five
  A+C+D+E+F+G+K,      // six
  A+B+C,              // seven
  A+B+C+D+E+F+G+K,    // eight
  A+B+C+D+F+G+K,      // nine
  A+D,                // colon
  A+N,                // semi colon
  D+N,                // less than
  D+G+K,              // equal sign
  D+L,                // greater than
  A+B+K+M,            // question mark
  A+B+D+E+F+I+K,      // @
  A+B+C+E+F+G+K,      // a
  D+E+F+G+C+K,        // b
  A+D+E+F,            // c
  A+B+C+D+I+M,        // d
  A+D+E+F+G,          // e
  A+E+F+G,            // f
  A+C+D+E+F+K,        // g
  B+C+E+F+G+K,        // h
  I+M,                // i
  B+C+D+E,            // j
  E+F+G+J+L,          // k
  D+E+F,              // l
  B+C+E+F+H+J,        // m
  B+C+E+F+H+L,        // n
  A+B+C+D+E+F,        // o
  A+B+E+F+G+K,        // p
  A+B+C+D+E+F+L,      // q
  A+E+F+G+J+L,        // r
  A+C+D+F+G+K,        // s
  A+I+M,              // t
  B+C+D+E+F,          // u
  E+F+J+N,            // v
  B+C+E+F+L+N,        // w
  H+J+L+N,            // x
  H+J+M,              // y
  A+D+J+N,            // z
  A+F+E+D,            // left bracket
  H+L,                // backslash
  A+B+C+D,            // right bracket
  B+J,                // caret
  D                   // underscore
};

                            
/******************************************************************************* 
Funtion prototypes
*******************************************************************************/
void updateVFD(uint8_t pos, uint16_t num);

void clearVFD(void);
void displayTime();
void displayString ();


void setup(){

  pinMode(SER_DATA_PIN,OUTPUT);
  pinMode(SER_CLK_PIN,OUTPUT);
  pinMode(SER_LAT_PIN,OUTPUT);
  pinMode(DECODER_A,OUTPUT);
  pinMode(DECODER_B,OUTPUT);
  pinMode(DECODER_C,OUTPUT);
  //pinMode(BUTTON, INPUT_PULLUP);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.setTimeOffset(7200);

 displayString ();
 
  t0 = millis();
}

void loop(){

    dt = millis() - t0; 
    
    if ( dt >= DATE_DELAY ) {
       t0 = millis();
       
       do{
       dt = millis() - t0;
       displayDate(); 
       }while (dt < INTERVALLO);
       
       t0 = millis();
       }
       
    
    displayTime(); 
    
}   //End of main program loop





void updateVFD(uint8_t pos, uint16_t num){ //This shifts 16 bits out LSB first on the rising edge of the clock, clock idles low. Leftmost byte is position 7-0, rightmost byte is 14-seg digit
      
     if(pos >= 0 && pos < 9){               //Only accept valid positons on the display
      digitalWrite(SER_CLK_PIN, LOW);
      digitalWrite(SER_DATA_PIN, LOW);
      digitalWrite(SER_LAT_PIN, LOW);
      
      uint16_t wordOut = num; // concatenate bytes into a 32-bit word to shift out
      boolean pinState;

        for (byte i=0; i<16; i++){        // once for each bit of data
          digitalWrite(SER_CLK_PIN, LOW);
          if (wordOut & (1<<i)){          // mask the data to see if the bit we want is 1 or 0
            pinState = 1;                 // set to 1 if true
          }
          else{
            pinState = 0; 
          }
                 
          digitalWrite(SER_DATA_PIN, pinState); //Send bit to register
          digitalWrite(SER_CLK_PIN, HIGH);      //register shifts bits on rising edge of clock
          digitalWrite(SER_DATA_PIN, LOW);      //reset the data pin to prevent bleed through
        }
      
        digitalWrite(SER_CLK_PIN, LOW);
        digitalWrite(SER_LAT_PIN, HIGH); //Latch the word to light the VFD


        switch (pos)
{
     
      case 0:
        digitalWrite(DECODER_A, HIGH);
        digitalWrite(DECODER_B, HIGH);
        digitalWrite(DECODER_C, HIGH);
      break;
      case 1:
        digitalWrite(DECODER_A, LOW);
        digitalWrite(DECODER_B, HIGH);
        digitalWrite(DECODER_C, HIGH);
      break;
      case 2:
        digitalWrite(DECODER_A, HIGH);
        digitalWrite(DECODER_B, LOW);
        digitalWrite(DECODER_C, HIGH);
      break;
      case 3:
        digitalWrite(DECODER_A, LOW);
        digitalWrite(DECODER_B, LOW);
        digitalWrite(DECODER_C, HIGH);
      break;
      case 4:
        digitalWrite(DECODER_A, HIGH);
        digitalWrite(DECODER_B, HIGH);
        digitalWrite(DECODER_C, LOW);
      break;
      case 5:
        digitalWrite(DECODER_A, LOW);
        digitalWrite(DECODER_B, HIGH);
        digitalWrite(DECODER_C, LOW);
      break;
      case 6:
        digitalWrite(DECODER_A, HIGH);
        digitalWrite(DECODER_B, LOW);
        digitalWrite(DECODER_C, LOW);
      break;   
       case 7:   
        digitalWrite(DECODER_A, LOW);
        digitalWrite(DECODER_B, LOW);
        digitalWrite(DECODER_C, LOW);
         break;
}
        
        delayMicroseconds(500); //This delay slows down the multiplexing to get get the brightest display (but too long causes flickering)
      
     clearVFD();
    }
} 

void clearVFD(void){
    digitalWrite(SER_DATA_PIN, LOW);          //clear data and latch pins
    digitalWrite(SER_LAT_PIN, LOW);
        for (byte i=0; i<16; i++){            //once for each bit of data
            digitalWrite(SER_CLK_PIN, LOW);
            digitalWrite(SER_CLK_PIN, HIGH);  //register shifts bits on rising edge of clock
            }
    digitalWrite(SER_CLK_PIN, LOW);
    digitalWrite(SER_LAT_PIN, HIGH);          //Latch the word to update the VFD
}



void displayTime(){
  
    timeClient.update();

  
  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);

 
    byte tensHour = timeClient.getHours() / 10; //Extract the individual digits
    byte unitsHour = timeClient.getHours() % 10;
    byte tensMin = timeClient.getMinutes() / 10;
    byte unitsMin = timeClient.getMinutes() % 10;
    byte tensSec = timeClient.getSeconds() / 10;
    byte unitsSec = timeClient.getSeconds() % 10;

    displayDigit(tensHour, unitsHour, tensMin, unitsMin, tensSec, unitsSec, true);
       
}

void displayDate(){
  
    timeClient.update();

    time_t epochTime = timeClient.getEpochTime();
  //Serial.println(timeClient.getFormattedTime());

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);
    
 
  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);  

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);
  
    
    byte tensDay = monthDay / 10; //Extract the individual digits
    byte unitsDays = monthDay % 10;
    byte tensMonth = currentMonth / 10;
    byte unitsMonth = currentMonth % 10;
    byte thousandsYear = currentYear / 1000;
    byte hundredsYear = (currentYear /100) % 10;
    byte tensYear = (currentYear / 10) % 10;
    byte unitsYear = currentYear % 10;

    displayDigit(tensDay, unitsDays, tensMonth, unitsMonth, tensYear, unitsYear, false);
}

void displayDigit(byte digit0,byte digit1,byte digit2,byte digit3,byte digit4,byte digit5, bool colon )
{
    uint16_t displayDigit0 = pgm_read_word_near(charactermap +16 + digit0);
    uint16_t displayDigit1 = pgm_read_word_near(charactermap +16 + digit1);
    uint16_t displayDigit2 = pgm_read_word_near(charactermap +16 + digit2);
    uint16_t displayDigit3 = pgm_read_word_near(charactermap +16 + digit3);
    uint16_t displayDigit4 = pgm_read_word_near(charactermap +16 + digit4);
    uint16_t displayDigit5 = pgm_read_word_near(charactermap +16 + digit5);
    uint16_t separator = (colon) ? pgm_read_word_near(charactermap + 13): pgm_read_word_near(charactermap + 15);
  
  
    updateVFD(0, displayDigit0);  
    updateVFD(1, displayDigit1);
    updateVFD(2, separator);                //dash
    updateVFD(3, displayDigit2);  
    updateVFD(4, displayDigit3);
    updateVFD(5, separator);                //dash
    updateVFD(6, displayDigit4);  
    updateVFD(7, displayDigit5);
  
  }

void displayString (){
  
  
    uint16_t digit0 = pgm_read_word_near(charactermap+35);
    uint16_t digit1 = pgm_read_word_near(charactermap+47);
    uint16_t digit2 = pgm_read_word_near(charactermap+46);
    uint16_t digit3 = pgm_read_word_near(charactermap+46);
    uint16_t digit4 = pgm_read_word_near(charactermap+37);
    uint16_t digit5 = pgm_read_word_near(charactermap+35);
    uint16_t digit6 = pgm_read_word_near(charactermap+52);
    uint16_t digit7 = pgm_read_word_near(charactermap+0);
    
    for ( int i = 0; i < 100; i++)
{
    updateVFD(0, digit0);  
    updateVFD(1, digit1);
    updateVFD(2, digit2);                
    updateVFD(3, digit3);  
    updateVFD(4, digit4);
    updateVFD(5, digit5);                
    updateVFD(6, digit6);  
    updateVFD(7, digit7);
}
    
}
