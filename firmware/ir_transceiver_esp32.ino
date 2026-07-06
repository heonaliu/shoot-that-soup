/*
 * IR ARCADE GUN — Transmitter firmware
 * ESP32
 *
 * Role: this board is the "gun". When you pull the trigger, it fires
 * an infrared NEC-encoded signal containing a fixed Address + Command.
 * Point it at a target running the matching receiver firmware, and
 * that target's Serial Monitor should print "HIT!".
 *
 * IMPORTANT: these values MUST match MY_TARGET_ADDRESS / MY_TARGET_COMMAND
 * in the receiver/target sketch, or the target will ignore the shot.
 *
 * HARDWARE (bare IR LED, no breakout module):
 *   - Trigger button on GPIO 5 (other leg to GND, uses INPUT_PULLUP)
 *   - IR LED wired directly to GPIO 4 through a resistor:
 *       GPIO 4 -> [220 ohm resistor] -> LED anode (long leg / +)
 *       LED cathode (short leg or flat-edge side / -) -> GND
 *     This drives the LED at GPIO-level current only, so expect
 *     roughly 1-3m of range. For more range, add an NPN transistor
 *     (2N2222/BC547/etc) as a driver stage:
 *       LED anode -> [220 ohm resistor] -> 5V
 *       LED cathode -> transistor collector
 *       transistor emitter -> GND
 *       transistor base -> [1k ohm resistor] -> GPIO 4
 *
 * IRremote automatically handles the 38kHz carrier modulation for you —
 * you never manually toggle the LED pin. The library takes care of that
 * internally the moment you call IrSender.sendNEC().
 */

#include <IRremote.hpp>

// ---- Pin assignments ----
#define IR_SEND_PIN     6
#define TRIGGER_PIN     7

// ---- The IR code this gun fires ----
// Must match the target's MY_TARGET_ADDRESS / MY_TARGET_COMMAND exactly.
#define GUN_ADDRESS   0x00
#define GUN_COMMAND   0x01

// ---- Trigger debounce + fire-rate limiting ----
int lastButtonState = HIGH;
unsigned long lastButtonChangeTime = 0;
const unsigned long DEBOUNCE_MS = 40;

unsigned long lastShotTime = 0;
const unsigned long FIRE_COOLDOWN_MS = 300;

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  IrSender.begin(IR_SEND_PIN);

  Serial.println(F("=== IR ARCADE GUN READY ==="));
  Serial.print(F("Firing on GPIO "));
  Serial.println(IR_SEND_PIN);
  Serial.print(F("Shot code -> Address: 0x"));
  Serial.print(GUN_ADDRESS, HEX);
  Serial.print(F(" Command: 0x"));
  Serial.println(GUN_COMMAND, HEX);
  Serial.println(F("Pull the trigger to fire!"));
}

void loop() {
  handleTrigger();
}

void handleTrigger() {
  int reading = digitalRead(TRIGGER_PIN);

  if (reading != lastButtonState) {
    lastButtonChangeTime = millis();
  }

  if ((millis() - lastButtonChangeTime) > DEBOUNCE_MS) {
    static bool triggerHandled = false;

    if (reading == LOW && !triggerHandled) {
      fireShot();
      triggerHandled = true;
    } else if (reading == HIGH) {
      triggerHandled = false;
    }
  }

  lastButtonState = reading;
}

void fireShot() {
  if (millis() - lastShotTime < FIRE_COOLDOWN_MS) {
    return;
  }
  lastShotTime = millis();

  Serial.println(F(">>> FIRE!"));
  IrSender.sendNEC(GUN_ADDRESS, GUN_COMMAND, 0);
}
