// https://forum.pjrc.com/threads/31518-Can-t-communicate-with-Teensy-3-2-through-Teensyduino?p=88073&viewfull=1#post88073
#define qBlink() (digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) ))  // Pin13 on T3.x & LC
void setup() {
  Serial.begin(38400);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  while (!Serial && (millis() <= 4000))
    { qBlink(); delay(50); }
  Serial.print("\nSetup() OK :: millis()==");
  Serial.println(millis());
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
}

elapsedMillis emilBlink;  // Teensy way to delay action without delay()
int8_t loopCount = 0; // Print a NewLine periodically

void loop() {
  if ( emilBlink > 1000 ) {
    qBlink();
    emilBlink = 0;
    Serial.print(".");
    loopCount += 1;
    if ( 40 < loopCount ) {
      loopCount = 0;
      Serial.println("!");
    }
  }
}
