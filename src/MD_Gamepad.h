/**
\mainpage Gamepad/Joystick Library
The library implements functions that allows gamepad/joystick shields similar to the one in the 
photo to be easily managed by the calling application.

![Gamepad Hardware] (Gamepad_Hardware.jpeg "Gamepad Hardware")

These shields are generically known as _JoyStick Shield V1_ and are meant to plug into 
Arduino controller boards.

The hardware configuration for these shields is fixed as follows:
- __PS2 game joystick__ - X axis is connected to A0, Y axis to A1, and Z axis switch (K) to D8.
- __Four gamepad switches__ - A connected to D2, B to D3, C to D4, and D to D5.
- __Two selection switches__ - E connected to D6, F to D7.

The shield provides a additional groupings for _I2C_, _Nokia 5110_ display, _nRF24L01_ 
and _Bluetooth_ modules which are not directly supported by this library, but could be 
managed using other function specific libraries.

All the hardware resources are declared at the top of the library file and can be modified 
for alternative arrangements.

Revision History 
----------------
Jun 2018 - version 1.0.0
- First release

Copyright
---------
Copyright (C) 2018 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#pragma once

#include <Arduino.h>

/**
 * \file
 * \brief Main header file for the MD_Gamepad library
 */

// Miscellaneous defines
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))    ///< Universal array size macro
#define DEFAULT_DELAY 100   ///< Default delay between reads in milliseconds
#define DEFAULT_DB    5     ///< Default deadband for ananlog zero conditioning

// Define pin numbers for the joystick shield 
#define PIN_A 2   ///< A switch on the gamepad
#define PIN_B 3   ///< B switch on the gamepad
#define PIN_C 4   ///< C switch on the gamepad
#define PIN_D 5   ///< D switch on the gamepad
#define PIN_E 6   ///< E switch on the gamepad
#define PIN_F 7   ///< F switch on the gamepad
#define PIN_K 8   ///< K (joystick) switch on the gamepad
#define PIN_X A0  ///< X axis analog pot
#define PIN_Y A1  ///< Y axis analog pot

/**
 * Core object for the MD_Gamepad library
 */
class MD_Gamepad
{
  public:
  /**
  * Switch name enumerated type.
  *
  * This enumerated type is used to specify a specific switch.
  * SW_NONE is used if no switch is named.
  */
  enum switch_t { SW_NONE, SW_A, SW_B, SW_C, SW_D, SW_E, SW_F, SW_K, SW_X, SW_Y };

 /** 
   * Initialize the object.
   *
   * Initialize the object data. This needs to be called during setup() to initialize new 
   * data for the class that cannot be done during the object creation.
   */
  void begin(void)
  {
    // initialize the hardware
    for (uint8_t i=0; i<ARRAY_SIZE(_pinDigital); i++)
      pinMode(_pinDigital[i].pin, INPUT_PULLUP);
    pinMode(PIN_X, INPUT);
    pinMode(PIN_Y, INPUT);

    // other variables
    _timeBetweenReads = DEFAULT_DELAY;
    _deadband = DEFAULT_DB;

    _offsetX = analogRead(PIN_X);
    _offsetY = analogRead(PIN_Y);
  }

 /** 
   * Set the minimum time between reads.
   * 
   * Set the minimum time in milliseconds between reads of a digital or analog value
   *
   * This can be used by the application to throttle the speed of repeat keypress detection
   * when a switch is kept active by a user. The value is initialized to DEFAULT_DELAY.
   *
   * \param d  the new delay value.
   * \return No return value.
   */
  inline void setReadDelay(uint16_t d) { _timeBetweenReads = d; }

 /** 
   * Test for any digital switch pressed.
   * 
   * The method returns true if any of the digital keys are pressed.
   *
   * \return true if any digital key has been pressed.
   */
  inline bool anyKey(void) { return(getSwitch() != SW_NONE); }

  /**
  * Get the value of the currently pressed switch.
  *
  * Returns the switch_t value of the first digital switch that is detected active.
  * If no key is found, return SW_NONE.
  *
  * \return The switch_t value corresponding to the switch pressed, SW_NONE if no key.
  */
  switch_t getSwitch(void)
  {
    // check if it is time to read something
    if (millis() - _timeLastDigital < _timeBetweenReads)
      return(SW_NONE);
    _timeLastDigital = millis();

    // now read and return the first pin found
    for (uint8_t i = 0; i < ARRAY_SIZE(_pinDigital); i++)
      if (digitalRead(_pinDigital[i].pin) == LOW)
        return(_pinDigital[i].sw);

    return(SW_NONE);
  }

  /**
  * Get direction for the joystick position.
  *
  * The joystick axis position is often more important than the actual value of its position. 
  * This method will tell the application whether a joystick is pushed into the positive or 
  * negative range of the analog axis. 
  * 
  * A small deadband is applied around the zero value to avoid jittery feedback when the joystick 
  * is centered. If the time delay between reads has not expired, 0 is returned.
  *
  * \see getJoystickValue() for the joystick position value.
  *
  * \param sw the switch_t value for the analog axis to be checked (SW_X or SW_Y).
  * \return 0 for centered, 1 for positive X or Y axis, -1 for negative X or Y axis.
  */
  int8_t getJoystickDirection(switch_t sw)
  {
    int16_t v;

    if (millis() - _timeLastAnalog < _timeBetweenReads)
      return(0);
    v = getJoystickValue(sw);
    
    return(v == 0 ? 0 : (v<0 ? -1 : 1));
  }

  /**
  * Get analog value of joystick position.
  *
  * This method will return the zero adjusted analog value of the requested joystick axis.
  * 
  * The zero values are read during the call to begin and are subtracted from the actual 
  * analog reading to created +/- range of values centred around 0.
  * 
  * A small deadband is applied around the zero value to avoid jittery feedback when the joystick 
  * is centered. If the time delay between reads has not expired, the last value read is returned.
  *
  * \see getJoystickDirection() for the relative axis position only.
  *
  * \param sw the switch_t value for the analog axis to be checked (SW_X or SW_Y).
  * \return The zero adjusted analog value.
  */
  int16_t getJoystickValue(switch_t sw)
  {
    if (millis() - _timeLastAnalog < _timeBetweenReads)
      return(sw == SW_X ? _valueX : _valueY); // return last read value
    _timeLastAnalog = millis();

    switch (sw)
    {
    case SW_X: 
      _valueX = analogRead(PIN_X) - _offsetX; 
      if (abs(_valueX) < _deadband) _valueX = 0;
      return(_valueX);

    case SW_Y: 
      _valueY = analogRead(PIN_Y) - _offsetY; 
      if (abs(_valueY) < _deadband) _valueY = 0;
      return(_valueY);
    }

    return(0);
  }
  
  private:
    // One element of the pin to switch ID table
    struct pin2sw_t
    {
      uint8_t pin;  // hardware pin
      switch_t sw;  // the switch ID
    };

    // Define lookup table for digital pin to switch ID
    pin2sw_t _pinDigital[7] = 
    { 
      { PIN_A, SW_A }, { PIN_B, SW_B }, { PIN_C, SW_C }, 
      { PIN_D, SW_D }, { PIN_E, SW_E }, { PIN_F, SW_F }, 
      { PIN_K, SW_K } 
    };

  // Time value and preset
  uint32_t _timeLastDigital;   ///< millis() saved value for last digitals processing
  uint32_t _timeLastAnalog;    ///< millis() saved value for analog processing
  uint16_t _timeBetweenReads;  ///< in milliseconds

  // Analog joystick handling
  uint8_t  _deadband;  ///< deadband for analog zero conditioning
  uint16_t _offsetX;    ///< the joystick offset for the X axis
  uint16_t _offsetY;    ///< the joystick offset for the Y axis
  int16_t _valueX;      ///< the adjusted value for the X axis
  int16_t _valueY;      ///< the adjusted value for the Y axis
};

MD_Gamepad gamepad;   ///< A single instance of this hardware declared in the library
