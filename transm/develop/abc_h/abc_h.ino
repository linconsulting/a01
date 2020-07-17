void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);


}

void loop() // run over and over
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}