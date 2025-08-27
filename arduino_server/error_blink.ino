// Some utilities to give our Arduino a status blink

void status_blink_setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void waiting_blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(900);
}

void active_blink() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void fatal_error_blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}