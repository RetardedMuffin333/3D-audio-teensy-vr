// 3D zvok

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <utility/imxrt_hw.h>
#include <U8g2lib.h>

#include "hrtf_filters.h"  // generated from MATLAB: hrtfL, hrtfR, NUM_POSITIONS

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=112.19999694824219,250.99998474121094
AudioInputUSB            usbIN;          //xy=125.19999694824219,156.99996948242188
AudioMixer4              mixerDAC_L;     //xy=371.20001220703125,169.99998474121094
AudioMixer4              mixerDAC_R;     //xy=373.20001220703125,239.99998474121094
AudioFilterFIR           firL;           //xy=568.2000122070312,170.19998168945312
AudioFilterFIR           firR; //xy=568.2000122070312,240.1999969482422
AudioOutputI2S           DAC;            //xy=786.2000122070312,232.99998474121094
AudioOutputUSB           usbOUT;         //xy=791.2000122070312,176.99996948242188
AudioConnection          patchCord1(playSdWav1, 0, mixerDAC_L, 2);
AudioConnection          patchCord2(playSdWav1, 0, mixerDAC_R, 2);
AudioConnection          patchCord3(playSdWav1, 1, mixerDAC_L, 3);
AudioConnection          patchCord4(playSdWav1, 1, mixerDAC_R, 3);
AudioConnection          patchCord5(usbIN, 0, mixerDAC_L, 0);
AudioConnection          patchCord6(usbIN, 0, mixerDAC_R, 0);
AudioConnection          patchCord7(usbIN, 1, mixerDAC_L, 1);
AudioConnection          patchCord8(usbIN, 1, mixerDAC_R, 1);
AudioConnection          patchCord9(mixerDAC_L, firL);
AudioConnection          patchCord10(mixerDAC_R, firR);
AudioConnection          patchCord11(firL, 0, usbOUT, 0);
AudioConnection          patchCord12(firL, 0, DAC, 0);
AudioConnection          patchCord13(firR, 0, usbOUT, 1);
AudioConnection          patchCord14(firR, 0, DAC, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=78.19999694824219,77
// GUItool: end automatically generated code

void applyHrtf(int index) {
  // st_fir comes in as 1..NUM_POSITIONS, convert to 0-based
  index = index - 1;
  if (index < 0) index = 0;
  if (index >= NUM_POSITIONS) index = NUM_POSITIONS - 1;

  // Teensy Audio lib expects pointer to coefficients + length
  firL.begin((short*)hrtfL[index], NUM_TAPS);
  firR.begin((short*)hrtfR[index], NUM_TAPS);
}


U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // vecji OLED

int Pot, Pot1, Pot2, Pot3, Pot4, Pot5, Pot6, Pot7, Pot8, Pot9, n;
int Nastavitev = 1;
int Nastavitev_spr = 1;
float AmplAC, IzhAmpl, IzhAmpl_dB;
int DispUpdVal = 1;
int DispUpd;
int mode = 1;
int inByte;//TO JE NOVO


const int LEDg = 0;
const int LEDb = 1;
const int LEDr = 2;

const float SPLcal = -44.5; // 1 Pa rms, mic_gain =  0
const float MICgain = 20;
const int Tmeas = 50;
int TPzg, TPsp, TPzg_cnt, TPsp_cnt;


short filtPASS[] = {0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000};
short filtATT[]  = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000};


// Use these with the Teensy Audio Shield Teensy 4.0
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11
#define SDCARD_SCK_PIN   13


void setI2SFreq(int freq) {
  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  int n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
  int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  double C = ((double)freq * 256 * n1 * n2) / 24000000;
  int c0 = C;
  int c2 = 10000;
  int c1 = C * c2 - (c0 * c2);
  set_audioClock(c0, c1, c2, true);
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
       | CCM_CS1CDR_SAI1_CLK_PRED(n1-1) // &0x07
       | CCM_CS1CDR_SAI1_CLK_PODF(n2-1); // &0x3f 
Serial.printf("SetI2SFreq(%d)\n",freq);
} // https://forum.pjrc.com/threads/57283-Change-sample-rate-for-Teensy-4-(vs-Teensy-3)


void setup() {  

     
  
  pinMode(LEDr, OUTPUT);
  pinMode(LEDg, OUTPUT);
  pinMode(LEDb, OUTPUT);  
  
  analogWrite(LEDr,0);    // LED OFF
  analogWrite(LEDg,0);
  analogWrite(LEDb,0);

  pinMode(16, INPUT);
  pinMode(16, INPUT_PULLUP);    // Tipka ZG
  pinMode(9, INPUT);
  pinMode(9,  INPUT_PULLDOWN);  // Tipka SP
  
  delay(100);
  
  Serial.begin(115200);
  Serial.setTimeout(10);
  AudioMemory(100);
  
  applyHrtf(1);   // start with first filter position

  
  sgtl5000_1.enable();
  sgtl5000_1.adcHighPassFilterFreeze();
  sgtl5000_1.adcHighPassFilterDisable();
  // sgtl5000_1.adcHighPassFilterEnable();
  sgtl5000_1.volume(0.0);
  sgtl5000_1.micGain(MICgain); // gain 0;  1 Pa_rms ..... -37 dB_fs peak
  sgtl5000_1.lineInLevel(15); // max 0 - min 15 (max gain = 15)
  sgtl5000_1.lineOutLevel(31); // min gain, 1.16 Vpp 
  
   delay(100);
  
  // setI2SFreq(44100);

  u8g2.begin();

  // Ali SD katica prisotna?
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
       u8g2.clearBuffer();          // clear the internal memory
       u8g2.setFont(u8g2_font_profont10_mf);      // v:08 12zn.
       u8g2.drawStr(0, 8, "Ni SD kartice ");
       u8g2.sendBuffer();          // transfer internal memory to the display 
       delay(2000);
       }
    }


  // Init filtrov                


  // Nastavitev mixerja
  mixerDAC_L.gain(0, 1);
  mixerDAC_L.gain(1, 0);
  mixerDAC_L.gain(2, 1);
  mixerDAC_L.gain(3, 0);
  mixerDAC_R.gain(0, 0);
  mixerDAC_R.gain(1, 1);
  mixerDAC_R.gain(2, 0);
  mixerDAC_R.gain(3, 1);             
       
  sgtl5000_1.volume(0.0); // 0.72 ... 512,0 mV rms izh; ampl = 1  
  
  } // setup

        // u8g2.setFont(u8g2_font_profont10_mf);      // v:08 12zn.
        // u8g2.setFont(u8g2_font_profont11_mf );     // v:10 11zn.
        // u8g2.setFont(u8g2_font_pxplusibmvga8_mf);  // v:13  8zn. 
        // u8g2.setFont(u8g2_font_7x14_mf);           // v:13  9,5zn.

elapsedMillis msecs;

void loop() {   // glavna zanka
  String data;
  

  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    data = Serial.readStringUntil('\n');
    Serial.println(data);
    String num1str = data.substring(0,1);
    Serial.print("string1: ");
    Serial.println(num1str);
    String num2str = data.substring(1,4);
    Serial.print("string2: ");
    Serial.println(num2str);
    String num3str = data.substring(4,6); 
    Serial.print("string3: ");
    Serial.println(num3str);
    int play_reset = num1str.toInt();
    int distance = num2str.toInt();
    int st_fir = num3str.toInt();
    Serial.print("števila: ");
    Serial.print(play_reset); Serial.print(" "); Serial.println(distance); Serial.print(" "); Serial.println(st_fir);
    if (distance < 1) distance = 1;  // avoid 0
    IzhAmpl = 0.1f / (distance * 1000.0f);
    sgtl5000_1.volume(IzhAmpl);
    IzhAmpl_dB = 60 * IzhAmpl - 48;
    IzhAmpl_dB = IzhAmpl_dB * 2;
    IzhAmpl_dB = (int) IzhAmpl_dB;
    IzhAmpl_dB =  IzhAmpl_dB / 2;
  
    applyHrtf(st_fir);

  u8g2.clearBuffer();          // clear the internal memory
            u8g2.setFont(u8g2_font_profont10_mf);      // v:08 12zn.
            u8g2.drawStr(0, 8, "   WAVpredv.");
            if (playSdWav1.isPlaying() == true)
                u8g2.drawStr(0, 17, "          > ");
            else 
                u8g2.drawStr(0, 17, "            ");

            u8g2.setFont(u8g2_font_profont10_mf);
            u8g2.drawStr(0, 28, "Filter ");
            u8g2.setCursor(30, 28);
            u8g2.print(st_fir);
            
           

            
  }
  



  
  if (msecs > Tmeas) 
    {
    msecs = 0;
    
    if (digitalRead(16)==0){
        TPzg_cnt++;
        }
    else {
        TPzg_cnt = 0;
        }
    if (digitalRead(9)==1){
        TPsp_cnt++;
        }
    else {
        TPsp_cnt = 0;
        }
    
     
    Pot1 = 1023 - analogRead(1);
    if ( abs(Pot1-Pot9) > 1 )
       Pot = Pot1;
    Pot9 = Pot8;
    Pot8 = Pot7;
    Pot7 = Pot6;
    Pot6 = Pot5;
    Pot5 = Pot4;
    Pot4 = Pot3;
    Pot3 = Pot2;
    Pot2 = Pot1;
       
    n++;

    
    

    
  if ( ( TPzg_cnt > 18 ) & ( TPzg_cnt < 22 ) & ( TPsp_cnt > 18 ) & ( TPsp_cnt < 22 ) )  {     // ca 2 s obe tipki
      Nastavitev++;
      Nastavitev_spr = 1;
      TPsp_cnt = 0;
      TPzg_cnt = 0;

      if (playSdWav1.isPlaying() == true) // ce se predavaja detoteke se pri premembi nastavitev prekine
        playSdWav1.stop();
      
      }
                
            IzhAmpl = (float)Pot/1023 * 0.7;
            sgtl5000_1.volume(IzhAmpl);
            IzhAmpl_dB = 60 * IzhAmpl - 48;
            IzhAmpl_dB = IzhAmpl_dB * 2;
            IzhAmpl_dB = (int) IzhAmpl_dB;
            IzhAmpl_dB =  IzhAmpl_dB / 2;

            if ( (TPsp_cnt > 10) & ( TPzg_cnt == 0 ) ) {   // PLAY / PAUSE
              TPsp_cnt = 0;
              if (playSdWav1.isPlaying() == true)
                playSdWav1.stop();
              else {
                playSdWav1.play("sirena.WAV");
                delay(10);
                }
              }

            

            
      
             
            
            // u8g2.setFont(u8g2_font_profont10_mf);      // v:08 12zn.
            // u8g2.setCursor(0, 28); 
            // u8g2.print(Pot);        
            // u8g2.setCursor(32, 28); 
            // u8g2.print(Pot1); 

            
            u8g2.setFont(u8g2_font_7x14_mf);           // v:13  9,5zn.
            u8g2.setCursor(0, 45); 
            if ( Pot < 5 ) {
              u8g2.drawStr(0, 45, "<-50  dB ");
              }
            else {
              u8g2.print(IzhAmpl_dB,1);           
              u8g2.setFont(u8g2_font_7x14_mf);           // v:13  9,5zn.
              u8g2.drawStr(42, 45, "dB "); 
            }
            u8g2.sendBuffer();          // transfer internal memory to the display
      
    }
} 
