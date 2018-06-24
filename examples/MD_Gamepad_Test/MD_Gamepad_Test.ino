// Test/example sketch for the MD_Gamepad libray
//
#include <MD_Gamepad.h>

void setup(void)
{
  Serial.begin(57600);
  Serial.print("\n[Gamepad Test]");
  gamepad.begin();
}

void loop(void)
{
  MD_Gamepad::switch_t sw = gamepad.getSwitch();
  int16_t z;

  // Process the digital switches
  if (sw != MD_Gamepad::SW_NONE)
  {
    Serial.print("\nSW_");
    switch (sw)
    {
    case MD_Gamepad::SW_A: Serial.print("A"); break;
    case MD_Gamepad::SW_B: Serial.print("B"); break;
    case MD_Gamepad::SW_C: Serial.print("C"); break;
    case MD_Gamepad::SW_D: Serial.print("D"); break;
    case MD_Gamepad::SW_E: Serial.print("E"); break;
    case MD_Gamepad::SW_F: Serial.print("F"); break;
    case MD_Gamepad::SW_K: Serial.print("K"); break;
    default: Serial.print("?");
    }
  }

  // Process the analog joystick
  z = gamepad.getJoystickDirection(MD_Gamepad::SW_X);
  if (z != 0)
  {
    Serial.print("\nX axis dir=");
    Serial.print(z);
    Serial.print(" value=");
    Serial.print(gamepad.getJoystickValue(MD_Gamepad::SW_X));
  }
  z = gamepad.getJoystickDirection(MD_Gamepad::SW_Y);
  if (z != 0)
  {
    Serial.print("\nY axis dir=");
    Serial.print(z);
    Serial.print(" value=");
    Serial.print(gamepad.getJoystickValue(MD_Gamepad::SW_Y));
  }
}
