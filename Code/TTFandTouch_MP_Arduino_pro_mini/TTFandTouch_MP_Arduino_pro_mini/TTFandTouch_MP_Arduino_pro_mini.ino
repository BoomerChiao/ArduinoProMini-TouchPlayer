#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000
#define ORIENTATION 1   //change screen rotation

//DFPlayerMini
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//#include "unnamed.c"
#include "Bmp.h"
#include "Adafruit_GFX.h"


// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// forward declarations 
extern bool update_button(Adafruit_GFX_Button *b, bool down);
extern bool update_button_list(Adafruit_GFX_Button **pb);

//Adafruit_GFX_Button on_btn, off_btn;
Adafruit_GFX_Button next_btn, previous_btn, Pausa_Continua, voium_High_btn, voium_Low_btn ,Led_open_btn ; //按鈕匿名

int pixel_x, pixel_y,BMP_cont_unmA ,BMP_cont_unmB ;     //Touch_getXY() updates global vars
int volume_rod=15; //音量控制器
int BMP_cont=0; //圖片切換
int RGB_xx=10,RED_xx=320,RGBandRED_xx=0; //人物X軸
bool pausa=false; //音樂  暫停/繼續
bool LED_ON_OFF,RGB_BOOL=false,RED_BOOL=true;
//bool RGBandRED_BOOL=false;

//DFPlayerMini
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;


//color
#define GRAY  0x7BEF
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DARKCYAN    0x03EF
#define Orange   0xFBE0



/*  
 * updating multiple buttons from a list
 * 
 * anything more than two buttons gets repetitive
 * 
 * you can place button addresses in separate lists
 * e.g. for separate menu screens
 */

// Array of button addresses to behave like a list
//Adafruit_GFX_Button *buttons[] = {&on_btn, &off_btn, NULL};



/* update the state of a button and redraw as reqd
 *
 * main program can use isPressed(), justPressed() etc
 */
bool update_button(Adafruit_GFX_Button *b, bool down)
{
    b->press(down && b->contains(pixel_x, pixel_y));
    if (b->justReleased())
        b->drawButton(false);
    if (b->justPressed())
        b->drawButton(true);
    return down;
}

/* most screens have different sets of buttons
 * life is easier if you process whole list in one go
 */
bool update_button_list(Adafruit_GFX_Button **pb)
{
    bool down = Touch_getXY();
    for (int i = 0 ; pb[i] != NULL; i++) {
        update_button(pb[i], down);
    }
    return down;
}


void setup()
{
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);
    
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
    tft.setRotation(ORIENTATION);   // try different rotations
    tft.fillScreen(BLACK);

    
 //----LED Open----
   Led_open_btn.initButton(&tft, 420, 155, 105, 135, DARKCYAN, GRAY, WHITE, "LED ON", 2);
   Led_open_btn.drawButton(false);
   LED_ON_OFF = false;
 
 //----TFT Button----
    previous_btn.initButton(&tft,  60, 275, 100, 40, WHITE, CYAN, BLACK, "<<", 3);
    Pausa_Continua.initButton(&tft, 230, 275, 150, 55, WHITE, CYAN, BLACK, "ON/OFF", 3);
    next_btn.initButton(&tft,  420, 275, 100, 40, WHITE, CYAN, BLACK, ">>", 3);
 
    voium_High_btn.initButton(&tft,  425, 33, 105, 55, WHITE, CYAN, BLACK, "Voice +", 2);
    voium_Low_btn.initButton(&tft, 55, 33, 105, 55, WHITE, CYAN, BLACK, "Voice -", 2);
    previous_btn.drawButton(false);
    next_btn.drawButton(false);
    Pausa_Continua.drawButton(false);
    voium_High_btn.drawButton(false);
    voium_Low_btn.drawButton(false);
// tft.fillRect(40, 80, 160, 80, RED);

    pinMode(LED_BUILTIN, OUTPUT);


//Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
   /* Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));*/
    tft.setCursor (0, 70);
    tft.setTextSize (4);
    tft.setTextColor(RED);
    tft.println(F("Unable to begin:\n" ));
    tft.setTextSize (2);
    tft.println(F(" Please insert the SD card and \n recheck the connection!!!" ));
    
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(15);  //Set volume value (0~30).
  //myDFPlayer.volumeUp(); //Volume Up
 // myDFPlayer.volumeDown(); //Volume Down

  
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);

  //----Set device we use SD as default----
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

 //----Read imformation----
  Serial.println(myDFPlayer.readState()); //read mp3 state
  Serial.println(myDFPlayer.readVolume()); //read current volume
  Serial.println(myDFPlayer.readEQ()); //read EQ setting
  Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
  Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
  Serial.println(myDFPlayer.readFileCountsInFolder(3)); //read file counts in folder SD:/03


}


/* compare the simplicity of update_button_list()
 */
void loop()
{




  
 bmp_time_cont();

  
//tft.drawBitmap(0, 0,  unnamed111, 55, 50,WHITE);
 // tft.drawRGBBitmap(10, 10, unnamed, 55, 50);
  //Serial.println("HIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII");
   // tft.fillRect(140, 0, 200, 40, CYAN); //清除
   // tft.drawRect(140, 0, 200, 40, CYAN);
   tft.drawRoundRect(140, 14, 200, 40,10, CYAN);  //彎曲角的矩形
   tft.fillRoundRect(143, 17, volume_rod*6.46, 34,6, CYAN);  //彎曲角的填充矩形
   //tft.fillRoundRect(143, 3, 195, 34,6, RED);  //彎曲角的填充矩形
   // tft.fillRoundRect(143, 3, 154, 34,6, CYAN);  //彎曲角的填充矩形

    
     bool down = Touch_getXY();
     previous_btn.press(down && previous_btn.contains(pixel_x, pixel_y));
     next_btn.press(down && next_btn.contains(pixel_x, pixel_y));
     Pausa_Continua.press(down && Pausa_Continua.contains(pixel_x, pixel_y));
     voium_High_btn.press(down && voium_High_btn.contains(pixel_x, pixel_y));
     voium_Low_btn.press(down && voium_Low_btn.contains(pixel_x, pixel_y));
     Led_open_btn.press(down && Led_open_btn.contains(pixel_x, pixel_y));
     
    if (previous_btn.justReleased())
        previous_btn.drawButton();
    if (next_btn.justReleased())
        next_btn.drawButton();
    if (Pausa_Continua.justReleased())
        Pausa_Continua.drawButton(); 
    if (voium_High_btn.justReleased())
        voium_High_btn.drawButton();
    if (voium_Low_btn.justReleased())
        voium_Low_btn.drawButton();
    if (Led_open_btn.justReleased())
        Led_open_btn.drawButton();

        
    //----Previous -----   
    if (previous_btn.justPressed()) {
      previous_btn.drawButton(true);

       // myDFPlayer.play(1);
       pausa=true;
       myDFPlayer.enableLoopAll();
       myDFPlayer.previous();
       // myDFPlayer.enableLoopAll();
       Serial.println(myDFPlayer.readVolume());
       Serial.println(myDFPlayer.readState());
      // tft.fillRect(40, 80, 160, 80, GREEN);

    }

    //----Next -----
    if (next_btn.justPressed()) {
        next_btn.drawButton(true);
       myDFPlayer.enableLoopAll();
       pausa=true;
       myDFPlayer.next();
       myDFPlayer.enableLoopAll();
       Serial.println(myDFPlayer.readVolume());
       Serial.println(myDFPlayer.readState());
      // tft.fillRect(40, 80, 160, 80, GREEN);

    }

    //----Pausa/Continua a musica ------
    if (Pausa_Continua.justPressed()) {
        myDFPlayer.enableLoopAll();
        Pausa_Continua.drawButton(true);
            if (pausa == false) {
              myDFPlayer.start();
               pausa=true;
            }
            else if (pausa == true) {
              myDFPlayer.pause(); 
              pausa=false;
            } 
        //    tft.fillRect(40, 80, 160, 80, RED);
            delay(500);
            

             
             

    }

    //----音量+ -------
   if (voium_High_btn.justPressed()) {
        voium_High_btn.drawButton(true);
        volume_rod_Hig();

      /*  Serial.println(volume_rod);
         Serial.println(myDFPlayer.readVolume());
         Serial.println(myDFPlayer.readState());*/
      //  tft.fillRect(40, 80, 160, 80, GRAY);
    }

    //----音量- -------
   if (voium_Low_btn.justPressed()) {
    voium_Low_btn.drawButton(true);
    volume_rod_Low();
    tft.fillRoundRect(143, 17, 194, 34,6, BLACK);//彎曲角的填充矩形
    
    /*Serial.println(myDFPlayer.readVolume());
      Serial.println(myDFPlayer.readState());*/
     //   tft.fillRect(40, 80, 160, 80, GRAY);
    }

    
     //----LED ON/Off -------
    if (Led_open_btn.justPressed()) {
      if(LED_ON_OFF == false){
      Led_open_btn.initButton(&tft, 420, 155, 105, 135, WHITE , Orange, RED, "LED OFF", 2);
      digitalWrite(LED_BUILTIN, HIGH);
      
      LED_ON_OFF = true;
      }
    else if(LED_ON_OFF == true){
      Led_open_btn.initButton(&tft, 420, 155, 105, 135, DARKCYAN, GRAY, WHITE, "LED ON", 2);
      digitalWrite(LED_BUILTIN, LOW);
      
      LED_ON_OFF = false;
      }
      
      Led_open_btn.drawButton(true);
      delay(500);
       
    }



/*if (myDFPlayer.available())  // 監視MP3有沒有回應
  {                                          // 有的話印出詳情
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }*/


   //  Serial.println(myDFPlayer.readState());
     


    
}



void bmp_time_cont(void){

  if(BMP_cont < 90 ){  //65=1000ms
  BMP_cont=BMP_cont+1;
  
  
  if (BMP_cont == 1 ||BMP_cont == 30||BMP_cont == 60){
    
  if(BMP_cont < 30 && BMP_cont >= 0 ){  //65=1000ms 


/*
///-RGB-KON
    tft.drawRGBBitmap(254, 120,RGB_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(272, 120,RGB_Stay_TOP_cat_RL, 6, 4);
    tft.drawRGBBitmap(254, 124,RGB_Stay_Top, 24, 8);
    tft.drawRGBBitmap(250, 132,RGB_Stay_Face, 32, 12);
    tft.drawRGBBitmap(258, 144,RGB_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(256, 150,RGB_Stay_ChestB, 20, 4);
    tft.drawRGBBitmap(260, 154,RGB_Stay_Foot, 20, 8);
    //tft.drawRGBBitmap(272, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(280, 154,R_Stay_Foot_C, 16, 8);
//-- -- --





///-RED-KON
    tft.drawRGBBitmap(304, 120,R_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(322, 120,R_Stay_TOP_cat_RL, 6, 4);
    tft.drawRGBBitmap(304, 124,R_Stay_TOP, 24, 8);
    tft.drawRGBBitmap(300, 132,R_Stay_Face, 32, 12);
    tft.drawRGBBitmap(308, 144,R_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(306, 150,R_Stay_ChestB, 20, 4);
    tft.drawRGBBitmap(310, 154,R_Stay_Foot_A, 20, 8);
    //tft.drawRGBBitmap(302, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(310, 154,R_Stay_Foot_C, 16, 8);
//-- -- --*/

///-RGB-KON
    tft.drawRGBBitmap(RGB_xx+4, 120,RGB_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(RGB_xx+22, 120,RGB_Stay_TOP_cat_RL, 6, 4);
    tft.drawRGBBitmap(RGB_xx+4, 124,RGB_Stay_Top, 24, 8);
    tft.drawRGBBitmap(RGB_xx, 132,RGB_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RGB_xx+8+RGBandRED_xx, 144,RGB_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RGB_xx+6+RGBandRED_xx, 150,RGB_Stay_ChestB, 20, 4);
    tft.drawRGBBitmap(RGB_xx+10, 154,RGB_Stay_Foot, 20, 8);
    //tft.drawRGBBitmap(RGB_xx+12, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(RGB_xx+30, 154,R_Stay_Foot_C, 16, 8);

    //tft.drawRGBBitmap(RGB_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    //tft.drawRGBBitmap(RGB_xx+22, 150,R_Stay_ChestB_A, 6, 2);
    
//-- -- --

///-RED-KON
    tft.drawRGBBitmap(RED_xx+4, 120,R_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(RED_xx+22, 120,R_Stay_TOP_cat_RL, 6, 4);
    tft.drawRGBBitmap(RED_xx+4, 124,R_Stay_TOP, 24, 8);
    tft.drawRGBBitmap(RED_xx, 132,R_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RED_xx+8+RGBandRED_xx, 144,R_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RED_xx+6+RGBandRED_xx, 150,R_Stay_ChestB, 20, 4);

    tft.drawRGBBitmap(RED_xx+10, 154,R_Stay_Foot_A, 20, 8);
    //tft.drawRGBBitmap(302, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(310, 154,R_Stay_Foot_C, 16, 8);
    
    //tft.drawRGBBitmap(RED_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    //tft.drawRGBBitmap(RED_xx+22, 150,R_Stay_ChestB_A, 6, 2);
    
//-- -- --

    if (pausa==false){
    tft.drawRGBBitmap(RGB_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RGB_xx+22, 150,R_Stay_ChestB_A, 6, 2);

    tft.drawRGBBitmap(RED_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RED_xx+22, 150,R_Stay_ChestB_A, 6, 2);
    }

    }

  
  if(BMP_cont < 60 && BMP_cont >= 30 ){  //65=1000ms 
    


///-RGB-KON
    tft.drawRGBBitmap(RGB_xx+4, 120,RGB_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(RGB_xx+22, 120,RGB_Stay_TOP_cat_LR, 6, 4);
    
  /*  tft.drawRGBBitmap(RGB_xx+4, 124,RGB_Stay_Top, 24, 8);
    tft.drawRGBBitmap(RGB_xx, 132,RGB_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RGB_xx+8, 144,RGB_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RGB_xx+6, 150,RGB_Stay_ChestB, 20, 4);
    tft.drawRGBBitmap(RGB_xx+10, 154,RGB_Stay_Foot, 20, 8);
    //tft.drawRGBBitmap(RGB_xx+12, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(RGB_xx+30, 154,R_Stay_Foot_C, 16, 8);

    tft.drawRGBBitmap(RGB_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RGB_xx+22, 150,R_Stay_ChestB_A, 6, 2);*/
    
//-- -- --

///-RED-KON
    tft.drawRGBBitmap(RED_xx+4, 120,R_Stay_TOP_cat_LR, 6, 4); tft.drawRGBBitmap(RED_xx+22, 120,R_Stay_TOP_cat_LR, 6, 4);
    
 /*   tft.drawRGBBitmap(RED_xx+4, 124,R_Stay_TOP, 24, 8);
    tft.drawRGBBitmap(RED_xx, 132,R_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RED_xx+8, 144,R_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RED_xx+6, 150,R_Stay_ChestB, 20, 4);

    tft.drawRGBBitmap(RED_xx+10, 154,R_Stay_Foot_A, 20, 8);
    //tft.drawRGBBitmap(RED_xx+2, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(RED_xx+10, 154,R_Stay_Foot_C, 16, 8);
    
    tft.drawRGBBitmap(RED_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RED_xx+22, 150,R_Stay_ChestB_A, 6, 2);*/
//-- -- --
    
    
  }

  if(BMP_cont < 90 && BMP_cont >= 60 ){  //65=1000ms 
///-RGB-KON
    tft.drawRGBBitmap(RGB_xx+4, 120,RGB_Stay_TOP_cat_RL, 6, 4); tft.drawRGBBitmap(RGB_xx+22, 120,RGB_Stay_TOP_cat_RL, 6, 4);
    
   /* tft.drawRGBBitmap(RGB_xx+4, 124,RGB_Stay_Top, 24, 8);
    tft.drawRGBBitmap(RGB_xx, 132,RGB_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RGB_xx+8, 144,RGB_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RGB_xx+6, 150,RGB_Stay_ChestB, 20, 4);
    tft.drawRGBBitmap(RGB_xx+10, 154,RGB_Stay_Foot, 20, 8);
    //tft.drawRGBBitmap(272, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(280, 154,R_Stay_Foot_C, 16, 8);

    tft.drawRGBBitmap(RGB_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RGB_xx+22, 150,R_Stay_ChestB_A, 6, 2);*/
//-- -- --


///-RED-KON
    tft.drawRGBBitmap(RED_xx+4, 120,R_Stay_TOP_cat_RL, 6, 4); tft.drawRGBBitmap(RED_xx+22, 120,R_Stay_TOP_cat_RL, 6, 4);
    
   /* tft.drawRGBBitmap(RED_xx+4, 124,R_Stay_TOP, 24, 8);
    tft.drawRGBBitmap(RED_xx, 132,R_Stay_Face, 32, 12);
    tft.drawRGBBitmap(RED_xx+8, 144,R_Stay_ChestA, 16, 6);
    tft.drawRGBBitmap(RED_xx+6, 150,R_Stay_ChestB, 20, 4);

    tft.drawRGBBitmap(RED_xx+10, 154,R_Stay_Foot_A, 20, 8);
    //tft.drawRGBBitmap(302, 154,R_Stay_Foot_B, 20, 8);
    //tft.drawRGBBitmap(310, 154,R_Stay_Foot_C, 16, 8);
    
    tft.drawRGBBitmap(RED_xx+4, 150,R_Stay_ChestB_A, 6, 2);
    tft.drawRGBBitmap(RED_xx+22, 150,R_Stay_ChestB_A, 6, 2);*/
//-- -- --


  } }}






  

else if (BMP_cont >= 90 ){  //65=1000ms
  BMP_cont=0;
  
  if (pausa==false){
    tft.fillRect(0, 100, 365, 80, BLACK);//填充矩形
    RGBandRED_xx=0;


//--RGB--
  if (RGB_BOOL==false){
  ///-RGB-KON
  
  RGB_xx=RGB_xx+10;
  if (RGB_xx>=320){
    
    RGB_BOOL=true;
  }}

  else{
    RGB_xx=RGB_xx-10;
  if (RGB_xx<=10){
    RGB_BOOL=false;
    }}
//-----


//--RED
if ( RED_BOOL==true ){
  ///-RGB-KON

RED_xx=RED_xx-10;
if (RED_xx<=10){
  RED_BOOL=false;
}}

  else{
RED_xx=RED_xx+10;
if (RED_xx>=320){
  
  RED_BOOL=true;
}}

}
//------


//if (pausa==true){
  else{
 // tft.fillRoundRect(10, 100, 345, 30,6, RED);//彎曲角的填充矩形
  tft.fillRect(0, 144, 365, 11, BLACK);//填充矩形

  BMP_cont_unmA=BMP_cont_unmA+1;

    switch (BMP_cont_unmA){
      
      case 1 :
      RGBandRED_xx=3; 
      tft.drawBitmap(RGB_xx+RGBandRED_xx+40, 144, NoteA, 7, 10, WHITE);
      tft.drawBitmap(RED_xx+RGBandRED_xx-18, 144, NoteA, 7, 10, WHITE);
      break;
      
      case 2 :
      RGBandRED_xx=0; 
      break;
      
      case 3 :
      RGBandRED_xx=-3; 
      tft.drawBitmap(RGB_xx+RGBandRED_xx-18, 144, NoteA, 7, 10, WHITE);
      tft.drawBitmap(RED_xx+RGBandRED_xx+38, 144, NoteA, 7, 10, WHITE);
      break;
      default:
      RGBandRED_xx=0; 
      BMP_cont_unmA=0;
    }



  
  }}

  }




void volume_rod_Hig (void) { 

        //------音量增用
        if (volume_rod >30){
          volume_rod=30;
        }
        if (volume_rod <30){
          volume_rod=volume_rod+1;
          
          myDFPlayer.volumeUp();
        }
        
        }


        
void volume_rod_Low (void) { 
        //------音量減用
        if (volume_rod >0){
          volume_rod=volume_rod-1;
          
          myDFPlayer.volumeDown();
        }
        if (volume_rod <0){
          volume_rod=0;
        }
        
        }



bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);      //because TFT control pins
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        switch (tft.getRotation() & 3) {
            // map raw ADC values to pixel coordinates
            // most apps only use a fixed rotation e.g omit unused rotations
            case 0:      //PORTRAIT
                pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
                pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
                break;
            case 1:      //LANDSCAPE
                pixel_x = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
                pixel_y = map(p.x, TS_RT, TS_LEFT, 0, tft.height());
                break;
            case 2:      //PORTRAIT REV
                pixel_x = map(p.x, TS_RT, TS_LEFT, 0, tft.width());
                pixel_y = map(p.y, TS_BOT, TS_TOP, 0, tft.height());
                break;
            case 3:      //LANDSCAPE REV
                pixel_x = map(p.y, TS_BOT, TS_TOP, 0, tft.width());
                pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());
                break;
        }
    }
    return pressed;
}
