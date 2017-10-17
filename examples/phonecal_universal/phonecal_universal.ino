#include <Adafruit_GFX.h>    // Core graphics library

#define LCD_ROTATION  0        //should work in all rotations
#define USE_READID    0        //Adafruit and ILI9488 can't read ID
//#include <MCUFRIEND_kbv.h>     // Hardware-specific library
//MCUFRIEND_kbv tft;
//#include <Adafruit_ILI9341.h>   // Hardware-specific library
//Adafruit_ILI9341 tft(10, 9, 8);
//#include <HX8347D_kbv.h>        // Hardware-specific library
//HX8347D_kbv tft;
#include <ILI9488.h>            // Hardware-specific library
ILI9488 tft(10, 9, 8);
//#include <ILI9488_kbv.h>            // Hardware-specific library
//ILI9488_kbv tft;

Adafruit_GFX_Button buttons[15];

extern void Touch_initialise(int aspect, int wid, int ht);
extern bool Touch_getXY(void);
bool touch_pressed;
int pixel_x, pixel_y;

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
#if USE_READID
    uint16_t ID;
    ID = tft.readID();  //ILI9488 does not have readID()
    if (ID == 0xD3D3) ID = 0x9090;  //your Mcufriend Mega2560 shield
//    if (ID == 0xD3D3) ID = 0x6814;  //my Mcufriend Mega2560 shield
    tft.begin(ID);
#else
    tft.begin();    //ILI9488 and Adafruit_ILI9341 do not have ID
#endif
    int aspect = LCD_ROTATION;   //PORTRAIT
    tft.setRotation(aspect);
    Touch_initialise(aspect, tft.width(), tft.height());  //.kbv external function
    tft.fillScreen(TFT_BLACK);
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
    touch_pressed = Touch_getXY();  //external function
#if 0
    Serial.print("X="); Serial.print(pixel_x); 
    Serial.print(" Y="); Serial.print(pixel_y); 
    Serial.print(" Z="); Serial.print(touch_pressed); 
    Serial.println("");
#endif
    if (touch_pressed) {
        x = pixel_x;      //copy global variable
        y = pixel_y;
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


