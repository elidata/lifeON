#define DEBUG 1
#include <Metro.h> //Include Metro library

// Randoms
long randNumber;
long rand_sound_or_song;
const int randomSeedPin = A3;

//Motor
long motor_duration = 1000;//60000; //1 min
const long min_motor_delay = 10000;//1 * 60000; //10 min
const long max_motor_delay = 3 * 60000; //30 min
long motor_delay = min_motor_delay; 
int motor_level = 1; //motor vibration speed/strength level

Metro motor_interval = Metro(motor_delay); 
boolean motor_state = LOW;
boolean motor_en = HIGH;

int motor_pwmpin = 11;
int motor_enpin = 10;
int motor_speed = 100;

//modes and buttons
int mode = 0;

int mode1pin = 7; //MP3 player
int mode2pin = 8; //sound + vibration
int mode3pin = 9; //sound - vibration

int motorintervalpin = A1;
int randsoundintervalpin = A2;
int motorspeedbutton = 4;

int volumeuppin = 5;
int volumedownpin = 6;
//MP3
#include <SoftwareSerial.h>
#define ARDUINO_RX 12//should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 13//connect to RX of the module
SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);

// Uncomment next line if you are using an Arduino Mega.
//#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

String mp3Answer;           // Answer from the MP3.

unsigned char playmode = 1; 
  #define PLAY  1
  #define PAUSE 0
/*
//      Command byte
#define CMD_NEXT_SONG 0X01
#define CMD_PREV_SONG 0X02
#define CMD_PLAY_W_INDEX 0X03
#define CMD_VOLUME_UP 0X04
#define CMD_VOLUME_DOWN 0X05
#define CMD_SET_VOLUME 0X06
#define CMD_SINGLE_CYCLE_PLAY 0X08
#define CMD_SEL_DEV 0X09
  #define DEV_TF 0X02
#define CMD_SLEEP_MODE 0X0A
#define CMD_WAKE_UP 0X0B
#define CMD_RESET 0X0C
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F
#define CMD_STOP_PLAY 0X16
#define CMD_FOLDER_CYCLE 0X17
#define CMD_SHUFFLE_PLAY 0X18
#define CMD_SET_SINGLE_CYCLE 0X19
  #define SINGLE_CYCLE_ON 0X00
  #define SINGLE_CYCLE_OFF 0X01
#define CMD_SET_DAC 0X1A
  #define DAC_ON  0X00
  #define DAC_OFF 0X01
#define CMD_PLAY_W_VOL 0X22

/*********************************************************************/


/************ Command byte **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16  // Stop playing continuously. 
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Opitons **************************/
#define DEV_TF            0X02


/*macro definitions of Rotary angle sensor and LED pin*/
#define ROTARY_ANGLE_SENSOR A0

#define ADC_REF 5//reference voltage of ADC is 5v
#define VCC     5    //the default value of VCC of the control interface is 5v
#define FULL_ANGLE 280//full value of the rotary angle is 280 degrees


static int8_t pre_vol = 0x0f; 
int8_t volume = 0x04;//0x0f;

const long randsound_min = 30000;//2 * 60000; //3min
const long randsound_max = 60000;//4 * 60000; //6min
long randsound_duration = randsound_min;

Metro randsound_interval = Metro(randsound_duration); 

void setup()
{
  Serial.begin(9600);
  mp3.begin(9600);
  pinMode(mode1pin,INPUT_PULLUP);
  pinMode(mode2pin,INPUT_PULLUP);
  pinMode(mode3pin,INPUT_PULLUP);

  pinMode(motorspeedbutton,INPUT_PULLUP);
  pinMode(motorintervalpin,INPUT_PULLUP);
  pinMode(randsoundintervalpin,INPUT_PULLUP);
  
  pinMode(volumeuppin,INPUT_PULLUP);
  pinMode(volumedownpin,INPUT_PULLUP);

  pinMode(motor_pwmpin,OUTPUT);
  pinMode(motor_enpin,OUTPUT);
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);
  delay(500);
  //attachInterrupt(0, playOrPause, FALLING);//pin2 -> INT0 
  attachInterrupt(1, skip, FALLING);//pin3 -> INT1 

  //sendCommand(CMD_SEL_DEV, DEV_TF);  
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  //sendCommand(CMD_SET_VOLUME, volume, 0);
  //sendCommand(CMD_PLAY_W_VOL, 0X0F01); //vol 15

  randomSeed(analogRead(randomSeedPin));
}

void loop()
{

setmotorspeed();
checkmode();
setvolume_digital();
//setvolume_analog();
set_motor_interval();
set_randsound_interval();

  char c = ' ';

  // If there a char on Serial call sendMP3Command to sendCommand
  if ( Serial.available() )
  {
    c = Serial.read();
    sendMP3Command(c);
  }

  // Check for the answer.
 if (mp3.available())
  {
   Serial.println(decodeMP3Answer());
  }
  delay(100);

if (motor_interval.check() == 1) { // check if the metro has passed its interval .
    if (motor_state == LOW){
    //turn motor on
    if (motor_en = HIGH){
      analogWrite(motor_pwmpin, motor_speed);
      digitalWrite(motor_enpin, HIGH);
      if (DEBUG){
        Serial.print("MOTOR ON "); Serial.println(motor_speed);
      }  
    }
    motor_state = HIGH;
    motor_interval.interval(motor_duration);

    }
    else if (motor_state == HIGH){
    //turn motor off
    digitalWrite(motor_pwmpin, 0);
    digitalWrite(motor_enpin, LOW);
    motor_state = LOW;
    motor_interval.interval(motor_delay);
    if (DEBUG){
      Serial.println("MOTOR OFF");
    }  
    }
  }

if (mode == 1){
  motor_state = HIGH;
  //Music
  //sendCommand(CMD_QUERY_STATUS);
  
  if (playmode == PLAY){
  //sendCommand(CMD_PLAY_FOLDER_FILE, 0x0100 | randNumber); //folder 01
  //sendCommand(CMD_FOLDER_CYCLE, 1, 0);
  }

}

/*
else if (mode == 2){
  //play a random sound OR song with vibration
//play random sound
  if (randsound_interval.check() == 1){
    randNumber = random(1, 10);
    randPlayInterval = random(1, 10);
    //play file randNumber
    playmode = PAUSE;
    sendCommand(CMD_PLAY_FOLDER_FILE, 0x0100 | randNumber); //folder 01
  }
}
*/

else if (mode == 2){
  //play a random sound OR song with vibration
  motor_en = HIGH;
  if (randsound_interval.check() == 1){
 
  randNumber = random(1, 11);
  rand_sound_or_song = random(1, 3);
  if (DEBUG){
    Serial.println(rand_sound_or_song);
  }  

  if (rand_sound_or_song == 1){
    // play sound
    //go to folder 1
    //play file randNumber

    //sendCommand(CMD_PLAY_FOLDER_FILE, 0x0200 | randNumber); //folder 02
      if (DEBUG){
    Serial.print("play sound ");  Serial.println(randNumber);
  }  
    sendCommand(CMD_PLAY_FOLDER_FILE, 1, randNumber);
   //delay(500);
   //sendCommand(CMD_STOP_PLAY);
   //delay(500);
   //sendCommand(CMD_SHUFFLE_PLAY);
    
  }
  else if (rand_sound_or_song == 2){
    // play song
    //go to folder 2
   //sendCommand(CMD_PLAY_FOLDER_FILE, 0x0100 | randNumber); //folder 01
     if (DEBUG){
    Serial.print("play song ");  Serial.println(randNumber);
  }  
   sendCommand(CMD_PLAY_FOLDER_FILE, 2, randNumber);
   //sendCommand(CMD_FOLDER_CYCLE, 2, 1);
   //delay(500);
   //sendCommand(CMD_STOP_PLAY);
   //delay(500);
   //sendCommand(CMD_SHUFFLE_PLAY);
   
   
  }
  }
}

else if (mode == 3){
  //play a random sound OR song without vibration
    motor_en = LOW;  //turn motor off
    analogWrite(motor_pwmpin, 0);
    digitalWrite(motor_enpin, LOW);

  //sendCommand(CMD_QUERY_STATUS);
  //delay(200);
  //Serial.print("ans:"); Serial.println(sanswer());
 //   if (mp3.available())
 // {
  //  if (decodeMP3Answer() == "Status playing: 1"){
  //    Serial.println("still playing");
  //  }
  //  else Serial.println("not playing");
  //}
  
  if (randsound_interval.check() == 1){
  randNumber = random(1, 11);
  rand_sound_or_song = random(1, 3); //either 1 or 2 (not 3 although it's written)
  if (DEBUG){
    Serial.println(rand_sound_or_song);
  }  
  
  if (rand_sound_or_song == 1){
    // play sound
    //go to folder 1
    //play file randNumber
  if (DEBUG){
    Serial.print("play song ");  Serial.println(randNumber);
  }  
  sendCommand(CMD_PLAY_FOLDER_FILE, 1, randNumber);
    //sendCommand(CMD_PLAY_FOLDER_FILE, 0x0200 | randNumber); //folder 02
   // sendCommand(CMD_FOLDER_CYCLE, 1, 0);
   //delay(500);
   //sendCommand(CMD_STOP_PLAY);
   //delay(500);
   //sendCommand(CMD_SHUFFLE_PLAY);
  }
  
  else if (rand_sound_or_song == 2){
    // play song
    //go to folder 2
   //sendCommand(CMD_PLAY_FOLDER_FILE, 0x0100 | randNumber); //folder 01
     if (DEBUG){
    Serial.print("play sound ");  Serial.println(randNumber);
  }  
  sendCommand(CMD_PLAY_FOLDER_FILE, 2, randNumber);
   //sendCommand(CMD_FOLDER_CYCLE, 2, 0);
   //delay(500);
   //sendCommand(CMD_STOP_PLAY);
   //delay(500);
   //sendCommand(CMD_SHUFFLE_PLAY);
  }
  }
}
}

int setmotorspeed(){
  if (!digitalRead(motorspeedbutton)){
    switch (motor_level){
      case 1:
        motor_level = 2;
        motor_speed = 255/4;
      break;
      case 2:
        motor_level = 3;
        motor_speed = 255/2;
      break;
      case 3:
        motor_level = 4;
        motor_speed = 255*3/4;
      break;
      case 4:
        motor_level = 1;
        motor_speed = 255;
      break;
    }
  if (DEBUG){
    Serial.print("speed level ="); Serial.println(motor_level); Serial.print("\t"); 
    Serial.print("motor speed ="); Serial.println(motor_speed);
  }
  }
}

long set_motor_interval(){
  if (!digitalRead(motorintervalpin)){
    motor_delay = motor_delay +60000; //up by 1 min
    if (motor_delay > max_motor_delay){
      motor_delay =  min_motor_delay; //go back to minimum
    }
   if (DEBUG){
    Serial.print("motor_delay ="); Serial.print(motor_delay/60000); Serial.println(" minutes");
  } 
  }
}

long set_randsound_interval(){
    if (!digitalRead(randsoundintervalpin)){
      randsound_duration = randsound_duration + 6000000;//60000; //up by 1 min
      if (randsound_duration > randsound_max){
        randsound_duration = randsound_min; //go back to minimum
      }
      if (DEBUG){
    Serial.print("randsound_duration ="); Serial.print(randsound_duration/60000); Serial.println(" minutes");
  }  
    }
}

int checkmode(){

  if((digitalRead(mode1pin) == 0) && mode != 1){
    mode = 1;
    
  if (DEBUG){
    Serial.print("mode ="); Serial.println(mode);
  }  
  }
  else if((digitalRead(mode2pin) == 0) && mode != 2){
    
      mode = 2;
    sendCommand(CMD_STOP_PLAY);
    delay(500);
  if (DEBUG){
    Serial.print("mode ="); Serial.println(mode);
  }  
    }
  
   else if((digitalRead(mode3pin) == 0) && mode != 3){
   
    mode = 3;
    sendCommand(CMD_STOP_PLAY);
    delay(500);
  if (DEBUG){
    Serial.print("mode ="); Serial.println(mode);
  }    
   }
 
  

}

void setvolume_digital(){
  if (!digitalRead(volumeuppin)){
    sendCommand(CMD_VOLUME_UP);
    //volume = volume+0x01;
    //if (volume > 0x1E) volume = 0x1E; 
    //sendCommand(CMD_SET_VOLUME, volume);
  //if (DEBUG){
  //  Serial.print("volume ="); Serial.println(volume);
  //}  
  }
  
  if (!digitalRead(volumedownpin)){
    //volume = volume-0x01;
    sendCommand(CMD_VOLUME_DOWN);
    //if (volume < 0x0F) volume = 0x0F; 
    //sendCommand(CMD_SET_VOLUME, volume);
  //if (DEBUG){
    //Serial.print("volume ="); Serial.println(volume);
  //}  
  delay(500);
  }
  

  
}
/*
void setvolume_analog(){
    int degrees;
  degrees = getDegree();
    
  int8_t volume;
  //The degrees is 0~280, should be converted to be 0~255 to control the
  //brightness of LED 
  volume = map(degrees, 0, 280, 0, 30); 
  if(volume != pre_vol)
  {
    sendCommand(CMD_SET_VOLUME, volume);
    pre_vol = volume;
  }
    if (DEBUG){
    Serial.print("volume ="); Serial.println(volume);
  }  
}
*/
/*
void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e; //
  Send_buf[1] = 0xff; //
  Send_buf[2] = 0x06; //
  Send_buf[3] = command; //
  Send_buf[4] = 0x00;//
  Send_buf[5] = (int8_t)(dat >> 8);//datah
  Send_buf[6] = (int8_t)(dat); //datal
  Send_buf[7] = 0xef; //
  for(uint8_t i=0; i<8; i++)//
  {
    mp3.write(Send_buf[i]) ;
  }
}
*/


/********************************************************************************/
/*Function sendMP3Command: seek for a 'c' command and send it to MP3  */
/*Parameter: c. Code for the MP3 Command, 'h' for help.                                                                                                         */
/*Return:  void                                                                */

void sendMP3Command(char c) {
  
  switch (c) {
    case '?':
    case 'h':
      Serial.println("HELP  ");
      Serial.println(" p = Play");
      Serial.println(" P = Pause");
      Serial.println(" > = Next");
      Serial.println(" < = Previous");
      Serial.println(" s = Stop Play"); 
      Serial.println(" + = Volume UP");
      Serial.println(" - = Volume DOWN");
      Serial.println(" c = Query current file");
      Serial.println(" q = Query status");
      Serial.println(" v = Query volume");
      Serial.println(" x = Query folder count");
      Serial.println(" t = Query total file count");
      Serial.println(" f = Play folder 1.");
      Serial.println(" g = Play folder 2.");
      Serial.println(" S = Sleep");
      Serial.println(" W = Wake up");
      Serial.println(" r = Reset");
      break;


    case 'p':
      Serial.println("Play ");
      sendCommand(CMD_PLAY);
      break;

    case 'P':
      Serial.println("Pause");
      sendCommand(CMD_PAUSE);
      break;

    case '>':
      Serial.println("Next");
      sendCommand(CMD_NEXT_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case '<':
      Serial.println("Previous");
      sendCommand(CMD_PREV_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case 's':
      Serial.println("Stop Play");
      sendCommand(CMD_STOP_PLAY);
      break;


    case '+':
      Serial.println("Volume Up");
      sendCommand(CMD_VOLUME_UP);
      break;

    case '-':
      Serial.println("Volume Down");
      sendCommand(CMD_VOLUME_DOWN);
      break;

    case 'c':
      Serial.println("Query current file");
      sendCommand(CMD_PLAYING_N);
      break;

    case 'q':
      Serial.println("Query status");
      sendCommand(CMD_QUERY_STATUS);
      break;

    case 'v':
      Serial.println("Query volume");
      sendCommand(CMD_QUERY_VOLUME);
      break;

    case 'x':
      Serial.println("Query folder count");
      sendCommand(CMD_QUERY_FLDR_COUNT);
      break;

    case 't':
      Serial.println("Query total file count");
      sendCommand(CMD_QUERY_TOT_TRACKS);
      break;

    case 'f':
      Serial.println("Playing folder 1");
      sendCommand(CMD_FOLDER_CYCLE, 1, 0);
      break;

    case 'g':
      Serial.println("Playing folder 2");
      sendCommand(CMD_FOLDER_CYCLE, 2, 0);
      break;

    case 'S':
      Serial.println("Sleep");
      sendCommand(CMD_SLEEP_MODE);
      break;

    case 'W':
      Serial.println("Wake up");
      sendCommand(CMD_WAKE_UP);
      break;

    case 'r':
      Serial.println("Reset");
      sendCommand(CMD_RESET);
      break;
  }
}



/********************************************************************************/
/*Function decodeMP3Answer: Decode MP3 answer.                                  */
/*Parameter:-void                                                               */
/*Return: The                                                  */

String decodeMP3Answer() {
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data recived correctly. ";
      break;

    case 0x42:
      decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
      break;

    case 0x48:
      decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
      break;

    case 0x4C:
      decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
      break;

    case 0x4E:
      decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
      break;

    case 0x4F:
      decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
      break;
  }

  return decodedMP3Answer;
}

void sendCommand(byte command){
  sendCommand(command, 0, 0);
}

void sendCommand(byte command, byte dat1, byte dat2){
  delay(20);
  Send_buf[0] = 0x7E;    //
  Send_buf[1] = 0xFF;    //
  Send_buf[2] = 0x06;    // Len
  Send_buf[3] = command; //
  Send_buf[4] = 0x01;    // 0x00 NO, 0x01 feedback
  Send_buf[5] = dat1;    // datah
  Send_buf[6] = dat2;    // datal
  Send_buf[7] = 0xEF;    //
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]) ;
    Serial.print(sbyte2hex(Send_buf[i]));
  }
  Serial.println();
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                       */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */


String sbyte2hex(uint8_t b){
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}


/********************************************************************************/
/*Function: shex2int. Returns a int from an HEX string.                         */
/*Parameter: s. char *s to convert to HEX.                                      */
/*Parameter: n. char *s' length.                                                */
/*Return: int                                                                   */

int shex2int(char *s, int n){
  int r = 0;
  for (int i=0; i<n; i++){
     if(s[i]>='0' && s[i]<='9'){
      r *= 16; 
      r +=s[i]-'0';
     }else if(s[i]>='A' && s[i]<='F'){
      r *= 16;
      r += (s[i] - 'A') + 10;
     }
  }
  return r;
}


/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

String sanswer(void){
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10))
  {
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  {
    return mp3answer;
  }

  return "???: " + mp3answer;
}

/*
int getDegree()
{
  
//Function: Get the angle between the mark on the potentiometer cap and the starting position  
//Parameter:-void                             
//Return:     -int,the range of degrees is 0~280   

  int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
  float voltage;
  voltage = (float)sensor_value*ADC_REF/1023;
  float degrees = (voltage*FULL_ANGLE)/VCC;
  return degrees;
}
*/
/*Interrupt service routine*/
/*
void playOrPause()
{
  cli();
  if(playmode == PLAY)
  {
    playmode = PAUSE;
  sendCommand(CMD_PAUSE);
  }
  else
  {
    playmode = PLAY;
  sendCommand(CMD_PLAY);
  }
  sei();
}
*/

/*Interrupt service routine*/
void skip(){
   cli();
  //if(playmode == PLAY)
  //{
    //playmode = PAUSE;
  //sendCommand(CMD_NEXT_SONG,0);
  sendCommand(CMD_NEXT_SONG);
  //}
  sei();
}
