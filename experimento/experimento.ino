#define LDR_PIN A0      //

const int MAX_ITERATIONS = 100;

float total = 0;
float iterations = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  float ldrSensorValue; 
  ldrSensorValue = analogRead(LDR_PIN);

  while (iterations < MAX_ITERATIONS) {
    total += ldrSensorValue;
    Serial.print(ldrSensorValue);
    Serial.print(",");
    iterations+=1; 
    delay(50); // 50ms
  }

  Serial.println("");
  Serial.println("Total: ");
  Serial.println(total/MAX_ITERATIONS);
}
