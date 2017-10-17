#include <Adafruit_GFX.h>    // Core graphics library
#include <ILI9488.h>     // Hardware-specific library
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>

//edit begin() method in XPT2046_Touchscreen.cpp source code :
//        attachInterrupt( digitalPinToInterrupt(tirqPin), isrPin, FALLING );  //.kbv

//#define XPT_CS  3
//#define XPT_IRQ 255       //use 3 if you fix interrupts in library source code

#define TS_LEFT 0   //3900    //The XPT2046_Touchscreen works in Landscape
#define TS_RT   320 //300     //I would expect Touch in Portrait
#define TS_TOP  0   //360
#define TS_BOT  480 //3800

ILI9488 tft(10, 9, 8);
Adafruit_FT6206 ctp = Adafruit_FT6206();
Adafruit_GFX_Button buttons[15];

// Color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
#define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define TFT_PINK        0xF81F

/******************* UI details */
#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

// text box where numbers go
#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 220
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR TFT_MAGENTA
// the data (phone #) we store in the textfield
#define TEXT_LEN 12
char textfield[TEXT_LEN + 1] = "";
uint8_t textfield_i = 0;

// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65

/* create 15 buttons, in classic candybar phone style */
char buttonlabels[15][5] = {
    "Send", "Clr", "End", "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"
};
uint16_t buttoncolors[15] = {
    TFT_DARKGREEN, TFT_DARKGREY, TFT_RED,
    TFT_BLUE, TFT_BLUE, TFT_BLUE,
    TFT_BLUE, TFT_BLUE, TFT_BLUE,
    TFT_BLUE, TFT_BLUE, TFT_BLUE,
    TFT_ORANGE, TFT_BLUE, TFT_ORANGE
};

void setup(void)
{
    Serial.begin(9600);
    Serial.println(F("TFT LCD test"));
//    uint16_t ID = tft.readID();
//    tft.begin(ID);
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    ctp.begin(40);            //.kbv FT6236 Touchscreen needs to start
    // create buttons
    for (uint8_t row = 0; row < 5; row++) {
        for (uint8_t col = 0; col < 3; col++) {
            uint8_t n = col + row * 3;
            //                    &gfx, x, y, w, h, outline, fill, text, textsize
            buttons[n].initButton(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                  BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y),
                                  BUTTON_W, BUTTON_H, TFT_WHITE, buttoncolors[n], TFT_WHITE,
                                  buttonlabels[n], BUTTON_TEXTSIZE);
            buttons[n].drawButton();
        }
    }

    // create 'text field'
    tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, TFT_WHITE);
}

// Print something in the mini status bar with either flashstring
void status(const __FlashStringHelper *msg)
{
    tft.fillRect(STATUS_X, STATUS_Y, 240, 8, TFT_BLACK);
    tft.setCursor(STATUS_X, STATUS_Y);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.print(msg);
}
// or charstring
void status(char *msg)
{
    tft.fillRect(STATUS_X, STATUS_Y, 240, 8, TFT_BLACK);
    tft.setCursor(STATUS_X, STATUS_Y);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.print(msg);
}

void loop(void)
{
    int x = -1, y = -1;   //regular pixel coordinates
    if (ctp.touched()) {         //XPT2046_Touchscreen can use hardware
        TS_Point p = ctp.getPoint(); //FT6236_touchscreen returns in Portrait on ILI
        x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
        y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    // go thru all the buttons, checking if they were pressed
    for (uint8_t b = 0; b < 15; b++) {
        bool down = buttons[b].contains(x, y);
        buttons[b].press(down);
    }

    // now we can ask the buttons if their state has changed
    for (uint8_t b = 0; b < 15; b++) {
        if (buttons[b].justReleased()) {
            // Serial.print("Released: "); Serial.println(b);
            buttons[b].drawButton();  // draw normal
        }

        if (buttons[b].justPressed()) {
            buttons[b].drawButton(true);  // draw invert!

            // if a numberpad button, append the relevant # to the textfield
            if (b >= 3) {
                if (textfield_i < TEXT_LEN) {
                    textfield[textfield_i] = buttonlabels[b][0];
                    textfield_i++;
                    textfield[textfield_i] = 0; // zero terminate
                    // fona.playDTMF(buttonlabels[b][0]);
                }
            }

            // clr button! delete char
            if (b == 1) {

                textfield[textfield_i] = 0;
                if (textfield > 0) {
                    textfield_i--;
                    textfield[textfield_i] = ' ';
                }
            }

            // update the current text field
            Serial.println(textfield);
            tft.setCursor(TEXT_X + 2, TEXT_Y + 10);
            tft.setTextColor(TEXT_TCOLOR, TFT_BLACK);
            tft.setTextSize(TEXT_TSIZE);
            tft.print(textfield);

            // its always OK to just hang up
            if (b == 2) {
                status(F("Hanging up"));
                Serial.println("Hanging up");
                delay(1000);
                tft.fillRect(TEXT_X + 1, TEXT_Y + 1, TEXT_W - 2, TEXT_H - 2, TFT_BLACK);
                textfield_i = 0;
            }
            // we dont really check that the text field makes sense
            // just try to call
            if (b == 0) {
                status(F("Calling"));
                Serial.print("Calling "); Serial.println(textfield);
                //fona.callPhone(textfield);
            }

            delay(100); // UI debouncing
        }
    }
}

