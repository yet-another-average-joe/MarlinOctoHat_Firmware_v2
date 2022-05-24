/*
 Name:       MarlinOctoHat_v2.ino
 Created:    2022/05/08
 Author:     Y@@J
 */

#include "MarlinOctoHat_v2.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI 2 : input (5V tolerant)

SPIClass    SPI_2(2);   // SPI in

// raw data buffer ; set buffer size to largest :
//
// ST7920_BUF_SIZE  : 1280 ; 1 page = half screen 
// SSD1306_BUF_SIZE : 131 : 3 control bytes + page data

volatile uint8_t SPI_2_Rx_Buffer[1280];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI 1 : output

// flag
volatile bool dataReady = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : "Arduino" setup()

void setup()
{
#ifdef __SERIAL_DEBUG
    Serial.begin(115200);
#endif // __SERIAL_DEBUG

#ifdef __POWER // TODO
	setupPower();
#endif // __POWER

    pinMode(LED_BUILTIN, OUTPUT); // will blink when display is updated
    pinMode(PIN_DTR, OUTPUT);
    pinMode(BOARD_SPI2_NSS_PIN, INPUT);
    pinMode(PIN_DISPLAY_TYPE, INPUT_PULLUP);

    digitalWrite(LED_BUILTIN, HIGH); // turn LED OFF
    digitalWrite(PIN_DTR, LOW);

    // "read" jumper and set emulator
    if (digitalRead(PIN_DISPLAY_TYPE) == HIGH)
        ST7920_setup_SPI_2();
    else
        SSD1306_setup_SPI_2();

    setup_SPI_1();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : Arduino loop()

void loop()
{
    if (dataReady)
    {
        dataReady = false; // reset flag

        digitalWrite(LED_BUILTIN, LOW); // LED ON
        digitalWrite(PIN_DTR, HIGH);
        delayMicroseconds(DTR_PULSE_DURATION); // let the RasPi know a Marlin UI capture is ready
        digitalWrite(PIN_DTR, LOW);
        digitalWrite(LED_BUILTIN, HIGH); // LED OFF

#ifdef __SERIAL_DEBUG
#ifdef __PRINT_HEX
        printBmpHex();
#endif//  __PRINT_HEX

#ifdef __PRINT_ASCII
        printBmpAscii();
#endif // __PRINT_ASCII
#endif // __SERIAL_DEBUG
    }
}

// END


