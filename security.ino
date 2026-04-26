// reads distance and sound, sends data to flipper over uart
// triggers alert when something is close AND loud at the same time

#define SOUND_PIN  34
#define TRIG_PIN    5
#define ECHO_PIN   18


float distThreshold  = 40.0;  // cm
int   soundThreshold = 600;   // raw adc value (0-4095)

float smoothedDist   = 100.0;
float smoothedSound  = 0.0;
bool  armed          = true;
bool  alert          = false;

// fire a pulse and measure how long it takes to come back
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    return 999.0;  
  }
  return (duration * 0.0343) / 2.0;
}


int getSoundPeak() {
  int peak = 0;
  for (int i = 0; i < 50; i++) {
    int v = analogRead(SOUND_PIN);
    if (v > peak) {
      peak = v;
    }
    delayMicroseconds(200);
  }
  return peak;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17); 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  delay(500);
  Serial2.println("READY");
}

void loop() {
  float rawDist  = getDistance();
  int   rawSound = getSoundPeak();


  smoothedDist  = 0.7 * smoothedDist  + 0.3 * rawDist;
  smoothedSound = 0.7 * smoothedSound + 0.3 * rawSound;

  // rough db estimate based on adc range
  float dB = 20.0 * log10((smoothedSound / 4095.0) + 1e-6) + 90.0;


  if (armed) {
    alert = (smoothedDist < distThreshold) && (rawSound > soundThreshold);
  } else {
    alert = false;
  }

  // send everything to flipper as comma separated values

  Serial2.print(smoothedDist, 1);  Serial2.print(",");
  Serial2.print(rawSound);         Serial2.print(",");
  Serial2.print(dB, 1);            Serial2.print(",");
  Serial2.print(alert ? 1 : 0);   Serial2.print(",");
  Serial2.print(armed ? 1 : 0);   Serial2.print(",");
  Serial2.print(distThreshold, 0); Serial2.print(",");
  Serial2.println(soundThreshold);

  // listen for commands coming back from flippeer
  if (Serial2.available() > 0) {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim();
    if      (cmd == "D+")  distThreshold  = min(distThreshold  + 5.0f, 200.0f);
    else if (cmd == "D-")  distThreshold  = max(distThreshold  - 5.0f,   5.0f);
    else if (cmd == "S+")  soundThreshold = min(soundThreshold + 50,     4000);
    else if (cmd == "S-")  soundThreshold = max(soundThreshold - 50,      100);
    else if (cmd == "ARM") armed = true;
    else if (cmd == "DIS") armed = false;
  }

  delay(100);
}
