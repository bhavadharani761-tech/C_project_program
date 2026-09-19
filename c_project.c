#define TRIG_PIN 9
#define ECHO_PIN 10

#define WHITE_LED 2
#define GREEN_LED 3
#define BLUE_LED 4

#define PUMP_LED 5
#define BUZZER 6

#define BUTTON_PIN 7

#define TANK_HEIGHT 100.0

unsigned long lowStartTime = 0;
unsigned long blinkTime = 0;

bool lowWater = false;
bool manualPump = false;

bool ledState = false;
bool lastButtonState = HIGH;


float getDistance()
{
  long duration;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return -1;
  }

  return duration * 0.0343 / 2;
}


void setup()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(WHITE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  pinMode(PUMP_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}


void loop()
{
  float distance;
  float waterLevel;
  float percentage;

  // =================================================
  // PUSHBUTTON TOGGLE
  // =================================================

  bool buttonState = digitalRead(BUTTON_PIN);

  // Detect one button press
  if (lastButtonState == HIGH && buttonState == LOW)
  {
    manualPump = !manualPump;

    if (manualPump == true)
    {
      Serial.println("MANUAL PUMP ON");
    }
    else
    {
      Serial.println("MANUAL PUMP OFF");
    }

    delay(50);
  }

  lastButtonState = buttonState;


  // =================================================
  // GET DISTANCE
  // =================================================

  distance = getDistance();


  // =================================================
  // SENSOR ERROR
  // =================================================

  if (distance < 0)
  {
    Serial.println("ULTRASONIC SENSOR ERROR");

    digitalWrite(WHITE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(PUMP_LED, LOW);
    digitalWrite(BUZZER, LOW);

    delay(500);
    return;
  }


  // =================================================
  // CALCULATE WATER LEVEL
  // =================================================

  waterLevel = TANK_HEIGHT - distance;

  if (waterLevel < 0)
    waterLevel = 0;

  if (waterLevel > TANK_HEIGHT)
    waterLevel = TANK_HEIGHT;

  percentage = (waterLevel / TANK_HEIGHT) * 100;


  // =================================================
  // SERIAL MONITOR
  // =================================================

  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Water Level = ");
  Serial.print(waterLevel);
  Serial.print(" cm (");
  Serial.print(percentage);
  Serial.println("%)");


  // =================================================
  // MANUAL PUMP
  // =================================================

  if (manualPump == true)
  {
    digitalWrite(PUMP_LED, HIGH);
  }


  // =================================================
  // VERY LOW WATER
  // BELOW 30%
  // =================================================

  if (percentage < 30)
  {
    if (lowWater == false && manualPump == false)
    {
      lowWater = true;
      lowStartTime = millis();

      Serial.println("VERY LOW WATER");
      Serial.println("WARNING STARTED");
    }


    // -----------------------------------------------
    // FIRST 5 SECONDS
    // -----------------------------------------------

    if (lowWater == true &&
        millis() - lowStartTime < 5000 &&
        manualPump == false)
    {
      digitalWrite(WHITE_LED, LOW);
      digitalWrite(GREEN_LED, LOW);

      // Blue LED blinking
      if (millis() - blinkTime >= 250)
      {
        blinkTime = millis();

        ledState = !ledState;

        digitalWrite(BLUE_LED, ledState);
      }

      // Buzzer ON
      digitalWrite(BUZZER, HIGH);

      // Pump OFF
      digitalWrite(PUMP_LED, LOW);
    }


    // -----------------------------------------------
    // AFTER 5 SECONDS
    // -----------------------------------------------

    else if (lowWater == true &&
             millis() - lowStartTime >= 5000)
    {
      digitalWrite(BLUE_LED, LOW);
      digitalWrite(BUZZER, LOW);

      // Automatic pump ON
      digitalWrite(PUMP_LED, HIGH);

      Serial.println("5 SECONDS COMPLETED");
      Serial.println("AUTOMATIC WATER PUMP ON");
    }
  }


  // =================================================
  // AVERAGE WATER
  // 30% - 79%
  // =================================================

  else if (percentage < 80)
  {
    lowWater = false;

    digitalWrite(WHITE_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(BUZZER, LOW);

    // Green LED continuously blinking
    if (millis() - blinkTime >= 500)
    {
      blinkTime = millis();

      ledState = !ledState;

      digitalWrite(GREEN_LED, ledState);
    }

    // If manual pump is OFF, pump is OFF
    if (manualPump == false)
    {
      digitalWrite(PUMP_LED, LOW);
    }

    Serial.println("AVERAGE WATER");
  }


  // =================================================
  // MAXIMUM WATER
  // 80% OR ABOVE
  // =================================================

  else
  {
    lowWater = false;

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(BUZZER, LOW);

    // White LED continuously blinking
    if (millis() - blinkTime >= 500)
    {
      blinkTime = millis();

      ledState = !ledState;

      digitalWrite(WHITE_LED, ledState);
    }

    // If manual pump is OFF, pump is OFF
    if (manualPump == false)
    {
      digitalWrite(PUMP_LED, LOW);
    }

    Serial.println("MAXIMUM WATER");
  }

  delay(50);
}