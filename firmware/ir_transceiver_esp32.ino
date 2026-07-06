/*
 * IR ARCADE TARGET — Receiver firmware
 * ESP32-S3
 *
 * Role: this board is a "target" that gets shot by a gun.
 * It listens for a specific NEC IR code (address + command),
 * and when it sees a match, registers a "HIT":
 *   - flashes the status LED
 *   - prints to Serial
 *   - (placeholder) increments a hit counter you can hook into game logic
 *
 * The pushbutton on GPIO 5 is wired as a manual "reset / re-arm" button
 * for testing — press it to reset the hit counter and simulate re-arming
 * the target between rounds.
 *
 * TESTING IN WOKWI:
 * While the simulation is running, click on the IR receiver component.
 * A popup lets you type in an "Address" and "Command" value (both in hex,
 * e.g. 0x00 and 0x01) and fire a simulated NEC IR signal at the receiver.
 * Set MY_TARGET_ADDRESS / MY_TARGET_COMMAND below to match what you send
 * in that popup, and you should see the LED flash + Serial print "HIT!".
 *
 * You can also just click the wokwi-ir-remote component's on-screen buttons
 * (each sends a fixed NEC command, see Wokwi docs for the code table) to
 * confirm the receiver picks up *something*, before testing exact-match logic.
 */

#include <IRremote.hpp>

// ---- Pin assignments (matches your diagram.json) ----
#define IR_RECEIVE_PIN   15
#define STATUS_LED_PIN   2
#define RESET_BUTTON_PIN 5

// ---- The IR code this target should respond to ----
// IMPORTANT: these are NOT fixed/special values you need to "achieve" —
// they should exactly match whatever your actual remote printed to
// Serial Monitor for whichever button you've decided to use as "fire".
// e.g. if your remote printed:
//     IR received -> Address: 0x0 Command: 0x7 Protocol: NEC
// then set:
//     MY_TARGET_ADDRESS 0x00
//     MY_TARGET_COMMAND 0x07
// Any button works fine as your "trigger" — pick one and use its real code.
#define MY_TARGET_ADDRESS 0x00
#define MY_TARGET_COMMAND 0x7

// ---- State ----
volatile unsigned int hitCount = 0;
unsigned long lastHitFlashTime = 0;
const unsigned long FLASH_DURATION_MS = 250;
bool ledIsFlashing = false;

// Simple debounce state for the reset button
int lastButtonState = HIGH;
unsigned long lastButtonChangeTime = 0;
const unsigned long DEBOUNCE_MS = 40;

void setup() {
  Serial.begin(115200);
  delay(300); // let USB CDC on the S3 settle in the simulator

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  Serial.println(F("=== IR ARCADE TARGET READY ==="));
  Serial.print(F("Listening on GPIO "));
  Serial.println(IR_RECEIVE_PIN);
  Serial.print(F("Target code -> Address: 0x"));
  Serial.print(MY_TARGET_ADDRESS, HEX);
  Serial.print(F(" Command: 0x"));
  Serial.println(MY_TARGET_COMMAND, HEX);
}

void loop() {
  handleIrReception();
  handleResetButton();
  handleLedFlashTimeout();
}

void handleIrReception() {
  if (IrReceiver.decode()) {

    // Always print what we received, even if it's not our code —
    // this is how you'll debug in the simulator, since you'll see
    // exactly what address/command values are arriving.
    Serial.print(F("IR received -> Address: 0x"));
    Serial.print(IrReceiver.decodedIRData.address, HEX);
    Serial.print(F(" Command: 0x"));
    Serial.print(IrReceiver.decodedIRData.command, HEX);
    Serial.print(F(" Protocol: "));
    Serial.println(getProtocolString(IrReceiver.decodedIRData.protocol));

    bool isOurCode =
      (IrReceiver.decodedIRData.address == MY_TARGET_ADDRESS) &&
      (IrReceiver.decodedIRData.command == MY_TARGET_COMMAND);

    // Ignore NEC "repeat" frames (held button) so one shot = one hit
    bool isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);

    if (isOurCode && !isRepeat) {
      registerHit();
    }

    IrReceiver.resume(); // ready for next signal
  }
}

void registerHit() {
  hitCount++;
  Serial.print(F(">>> HIT! Total hits: "));
  Serial.println(hitCount);

  digitalWrite(STATUS_LED_PIN, HIGH);
  ledIsFlashing = true;
  lastHitFlashTime = millis();

  // ---- Hook your game logic here ----
  // e.g. send score over ESP-NOW/WiFi, trigger a servo, play a sound, etc.
}

void handleLedFlashTimeout() {
  if (ledIsFlashing && (millis() - lastHitFlashTime >= FLASH_DURATION_MS)) {
    digitalWrite(STATUS_LED_PIN, LOW);
    ledIsFlashing = false;
  }
}

void handleResetButton() {
  int reading = digitalRead(RESET_BUTTON_PIN);

  if (reading != lastButtonState) {
    lastButtonChangeTime = millis();
  }

  if ((millis() - lastButtonChangeTime) > DEBOUNCE_MS) {
    // Button is pressed (active LOW because of INPUT_PULLUP)
    static bool actionTaken = false;
    if (reading == LOW && !actionTaken) {
      hitCount = 0;
      Serial.println(F("--- Target reset. Hit count cleared. ---"));
      actionTaken = true;
    } else if (reading == HIGH) {
      actionTaken = false;
    }
  }

  lastButtonState = reading;
}
