#define USE_MCUFRIEND_RESISTIVE   0  //TouchScreen.h
#define USE_RESISTIVE_KBV         0  //TouchScreen_kbv.h
#define USE_MEGA2560_SHIELD       0  //URTouch.h
#define USE_HCTFT_SHIELD          0  //HCTFT_Touch.h
#define USE_ILI9341_XPT2046       0  //XPT2046_Touchscreen
#define USE_WAVESHARE_SHIELD      0  //XPT2046_Touchscreen.h
#define USE_EASTRISING_CAPACITIVE 1  //Adafruit_FT6206.h

#include <Arduino.h>

extern int pixel_x, pixel_y;  // global variables with pixel coordinates
static int tft_width, tft_height, tft_aspect;
static void mapxy(int x, int y);

#if USE_HCTFT_SHIELD
#include <HCTFT_Touch.h>

static HCTFT_Touch myTouch(MCUFRIEND_3_5_MEGA); // Standard Arduino Mega/Due shield
#define TS_LANDSCAPE 1
#define TS_LEFT MCUFRIEND_3_5_YMAX
#define TS_RT   MCUFRIEND_3_5_YMIN
#define TS_TOP  MCUFRIEND_3_5_XMIN
#define TS_BOT  MCUFRIEND_3_5_XMAX


void Touch_initialise(int aspect, int wid, int ht)
{
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    bool pressed = myTouch.Pressed();
    if (pressed) {         //XPT2046_Touchscreen can use hardware
        mapxy(myTouch.ReadAxis(XAXIS), myTouch.ReadAxis(YAXIS)); //raw values
    }
    return pressed;
}
#endif

#if USE_MEGA2560_SHIELD
#include <URTouch.h>
//URTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq);
static URTouch myTouch( 6, 5, 4, 3, 2); // Standard Arduino Mega/Due shield
#define TS_LANDSCAPE 1
#define TS_LEFT 3590 //3266    //The XPT2046_Touchscreen wired in Landscape
#define TS_RT   730  //745     //I would expect Touch in Portrait
#define TS_TOP  3581 //2828
#define TS_BOT  182  //214

void Touch_initialise(int aspect, int wid, int ht)
{
    myTouch.InitTouch(0);            //.kbv force PORTRAIT
//    myTouch.setPrecision(PREC_HI);
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    bool pressed = myTouch.dataAvailable();
    if (pressed) {         //XPT2046_Touchscreen can use hardware
        myTouch.read();           //0x6814 reads raw values in Landscape
        mapxy(myTouch.TP_X, myTouch.TP_Y); //raw values
    }
    return pressed;
}
#endif


#if USE_ILI9341_XPT2046 || USE_WAVESHARE_SHIELD
#include <XPT2046_Touchscreen.h> // define pins, calibration, getPoint() is different

#if USE_WAVESHARE_SHIELD
#define TS_LANDSCAPE 1
#define TS_LEFT 3800    //The XPT2046_Touchscreen wired in Landscape
#define TS_RT   360     //I would expect Touch in Portrait
#define TS_TOP  3900
#define TS_BOT  300
#define XPT_CS  4         // Waveshare shield
#define XPT_IRQ 255       //use 3 if you fix interrupts in library source code
#else
#define TS_LANDSCAPE 1
#define TS_LEFT 3800  //3900    //The XPT2046_Touchscreen
#define TS_RT   360 //300     //I would expect Touch in Portrait
#define TS_TOP  300 //360
#define TS_BOT  3900  //3800
#define XPT_CS  3         // Red ILI9341 display
#define XPT_IRQ 255       //use 3 if you fix interrupts in library source code
#endif

static XPT2046_Touchscreen ts(XPT_CS, XPT_IRQ);

void Touch_initialise(int aspect, int wid, int ht)
{
    ts.begin();            //.kbv XPT2046 Touchscreen needs to start
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    bool pressed = ts.touched();
    if (pressed) {
        TS_Point p = ts.getPoint(); //FT6236_touchscreen returns in Portrait on ILI
        mapxy(p.x, p.y);
    }
    return pressed;
}
#endif


#if USE_EASTRISING_CAPACITIVE
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>

#define TS_LEFT 0   //3900    //The XPT2046_Touchscreen works in Landscape
#define TS_RT   320 //300     //I would expect Touch in Portrait
#define TS_TOP  0   //360
#define TS_BOT  480 //3800

static Adafruit_FT6206 ctp = Adafruit_FT6206();

void Touch_initialise(int aspect, int wid, int ht)
{
    ctp.begin(40);            //.kbv FT6236 Touchscreen needs to start
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    bool pressed = ctp.touched();
    if (pressed) {
        TS_Point p = ctp.getPoint(); //FT6236_touchscreen returns in Portrait on ILI
        mapxy(p.x, p.y);
    }
    return pressed;
}
#endif


#if USE_MCUFRIEND_RESISTIVE
#include <TouchScreen.h>  //.kbv
// These are the pins for the shield!
#define XM A1  // must be an analog pin, use "An" notation!
#define YP A2  // must be an analog pin, use "An" notation!
#define XP 7   // can be a digital pin
#define YM 6   // can be a digital pin

/*
#define TS_LEFT 134
#define TS_RT   900
#define TS_TOP  117
#define TS_BOT  911
*/
#if 1
#define TS_LEFT 917
#define TS_RT   185
#define TS_TOP  950
#define TS_BOT  180
#else
#define TS_LANDSCAPE 1 //swap XM, YP and XP, YM
#define TS_LEFT 950
#define TS_RT   180
#define TS_TOP  185
#define TS_BOT  917
#endif

#define MINPRESSURE 200
#define MAXPRESSURE 1000

static TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //.kbv

void Touch_initialise(int aspect, int wid, int ht)
{
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        mapxy(p.x, p.y);
    }
    return pressed;
}
#endif

#if USE_RESISTIVE_KBV
#include "TouchScreen_kbv.h"  //.kbv
// These are the pins for the shield!
// Note that ARM calibration is different to AVR
// and probably uses different MINPRESSURE

#define XM A1  // must be an analog pin, use "An" notation!
#define YP A2  // must be an analog pin, use "An" notation!
#define XP 7   // can be a digital pin
#define YM 6   // can be a digital pin
#define TS_LEFT 904 //917 //9488_555 shield
#define TS_RT   104 //185
#define TS_TOP  930 //950
#define TS_BOT  77  //180
/*
#define TS_LEFT 134
#define TS_RT   900
#define TS_TOP  117
#define TS_BOT  911
*/

/*
#define XM A2  // must be an analog pin, use "An" notation!
#define YP A1  // must be an analog pin, use "An" notation!
#define XP 6   // can be a digital pin
#define YM 7   // can be a digital pin
#define TS_LEFT 911 //my touchscreen calibration 3.95"
#define TS_RT   179 //
#define TS_TOP  949 //
#define TS_BOT  164 //
*/

#define MINPRESSURE 20
#define MAXPRESSURE 1000

static TouchScreen_kbv ts(XP, YP, XM, YM, 300); //.kbv

void Touch_initialise(int aspect, int wid, int ht)
{
    tft_aspect = aspect;
    tft_width = wid;
    tft_height = ht;
}

bool Touch_getXY(void)
{
    TSPoint_kbv p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        mapxy(p.x, p.y);
    }
    return pressed;
}
#endif

void mapxy(int x, int y)
{
    int aspect = tft_aspect;
#if TS_LANDSCAPE
//    aspect += 3;
    int t = x; x = y; y = t;
#endif
    switch (aspect & 3) {
        case 0:      //PORTRAIT
            pixel_x = map(x, TS_LEFT, TS_RT, 0, tft_width);
            pixel_y = map(y, TS_TOP, TS_BOT, 0, tft_height);
            break;
        case 1:      //LANDSCAPE
            pixel_x = map(y, TS_TOP, TS_BOT, 0, tft_width);
            pixel_y = map(x, TS_RT, TS_LEFT, 0, tft_height);
            break;
        case 2:      //PORTRAIT REV
            pixel_x = map(x, TS_RT, TS_LEFT, 0, tft_width);
            pixel_y = map(y, TS_BOT, TS_TOP, 0, tft_height);
            break;
        case 3:      //LANDSCAPE REV
            pixel_x = map(y, TS_BOT, TS_TOP, 0, tft_width);
            pixel_y = map(x, TS_LEFT, TS_RT, 0, tft_height);
            break;
    }
}

