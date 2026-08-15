#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "iQOO Neo9 Pro"
#define WIFI_PASSWORD "password"

#define API_KEY "AIzaSyBH-CvLwg13Ke-lQq8AheUkQqBkCi7m9F4"
#define DATABASE_URL "https://smart-home-energy-saver-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ======================================================
// PINS
// ======================================================

const int PIR_PIN = 23;
const int RELAY_PIN = 18;
const int LED_PIN = 2;

const int RED_LED = 27;
const int GREEN_LED = 25;

// ======================================================
// SETTINGS
// ======================================================

const bool RELAY_ACTIVE_LOW = false;

const unsigned long OFF_DELAY_MS = 10000;

const float LIGHT_POWER = 10.0;

// ======================================================
// VARIABLES
// ======================================================

bool systemOn = false;

unsigned long lastMotionTime = 0;
unsigned long relayOnStart = 0;

float energyUsed = 0.0;
float energySaved = 0.0;

// ======================================================
// RELAY / ONBOARD LED
// ======================================================

void setRelay(bool on)
{
  digitalWrite(LED_PIN, on ? HIGH : LOW);

  if (RELAY_ACTIVE_LOW)
  {
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  }
}

// ======================================================
// TRAFFIC LIGHT - ACTIVE LOW
// ======================================================

void showNoMotion()
{
  digitalWrite(GREEN_LED, HIGH);   // Green OFF
  digitalWrite(RED_LED, LOW);      // Red ON
}

void showMotion()
{
  digitalWrite(RED_LED, HIGH);     // Red OFF
  digitalWrite(GREEN_LED, LOW);    // Green ON
}

// ======================================================
// WIFI RECONNECT
// ======================================================

bool checkWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  Serial.println("WiFi lost - reconnecting...");

  WiFi.disconnect();
  delay(500);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startTime < 10000
  )
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi reconnected");
    return true;
  }

  Serial.println("WiFi reconnect FAILED");

  return false;
}

// ======================================================
// FIREBASE STATUS UPDATE
// ======================================================

void updateFirebaseStatus(
  String motion,
  String light,
  String occupancy,
  String alert
)
{
  if (!checkWiFi())
  {
    Serial.println("Firebase skipped - WiFi unavailable");
    return;
  }

  if (!Firebase.ready())
  {
    Serial.println("Firebase temporarily not ready");
    return;
  }

  FirebaseJson json;

  json.set("motion", motion);
  json.set("light", light);
  json.set("occupancy", occupancy);
  json.set("alert", alert);

  if (
    Firebase.RTDB.updateNode(
      &fbdo,
      "/home",
      &json
    )
  )
  {
    Serial.println("Firebase status updated");
  }
  else
  {
    Serial.print("Firebase update FAILED: ");
    Serial.println(fbdo.errorReason());
  }
}

// ======================================================
// ENERGY UPDATE
// ======================================================

void updateEnergyFirebase()
{
  if (!checkWiFi())
  {
    return;
  }

  if (!Firebase.ready())
  {
    return;
  }

  if (
    Firebase.RTDB.setFloat(
      &fbdo,
      "/home/energyUsed",
      energyUsed
    )
  )
  {
    Serial.print("Energy Used = ");
    Serial.println(energyUsed, 4);
  }
  else
  {
    Serial.print("Energy Used FAILED: ");
    Serial.println(fbdo.errorReason());
  }

  if (
    Firebase.RTDB.setFloat(
      &fbdo,
      "/home/energySaved",
      energySaved
    )
  )
  {
    Serial.print("Energy Saved = ");
    Serial.println(energySaved, 4);
  }
  else
  {
    Serial.print("Energy Saved FAILED: ");
    Serial.println(fbdo.errorReason());
  }
}

// ======================================================
// SETUP
// ======================================================

void setup()
{
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Initial state
  systemOn = false;

  setRelay(false);
  showNoMotion();

  // ======================================================
  // WIFI
  // ======================================================

  WiFi.mode(WIFI_STA);

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  Serial.print("Connecting to WiFi");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ======================================================
  // FIREBASE CONFIG
  // ======================================================

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Connection timeout settings
  config.timeout.networkReconnect = 10 * 1000;
  config.timeout.socketConnection = 30 * 1000;
  config.timeout.sslHandshake = 60 * 1000;
  config.timeout.serverResponse = 15 * 1000;

  if (
    Firebase.signUp(
      &config,
      &auth,
      "",
      ""
    )
  )
  {
    Serial.println("Firebase SignUp OK");
  }
  else
  {
    Serial.print("Firebase SignUp Failed: ");

    Serial.println(
      config.signer.signupError.message.c_str()
    );
  }

  Firebase.begin(
    &config,
    &auth
  );

  Firebase.reconnectWiFi(true);

  fbdo.setBSSLBufferSize(
    4096,
    1024
  );

  // TCP keep-alive
  fbdo.keepAlive(5, 5, 1);

  while (!Firebase.ready())
  {
    Serial.println("Waiting for Firebase...");
    delay(500);
  }

  Serial.println("Firebase Ready");

  // ======================================================
  // INITIAL DATABASE VALUES
  // ======================================================

  updateFirebaseStatus(
    "No Motion Detected",
    "OFF",
    "Empty",
    "No Alerts"
  );

  updateEnergyFirebase();

  lastMotionTime = millis();

  Serial.println("==============================");
  Serial.println("SMART HOME SYSTEM READY");
  Serial.println("RED   = No Motion");
  Serial.println("GREEN = Motion Detected");
  Serial.println("==============================");
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
  unsigned long now = millis();

  // Important: keep Firebase authentication/network tasks alive
  Firebase.ready();

  int motion = digitalRead(PIR_PIN);

  // ======================================================
  // PIR DEBUG
  // ======================================================

  static unsigned long debugTime = 0;

  if (now - debugTime >= 1000)
  {
    debugTime = now;

    Serial.print("PIR = ");
    Serial.println(motion);
  }

  // ======================================================
  // MOTION DETECTED
  // ======================================================

  if (motion == HIGH)
  {
    lastMotionTime = now;

    if (!systemOn)
    {
      systemOn = true;

      relayOnStart = now;

      // Local hardware works first
      setRelay(true);
      showMotion();

      Serial.println("Motion Detected");

      // Cloud update second
      updateFirebaseStatus(
        "Motion Detected",
        "ON",
        "Occupied",
        "Motion Alert"
      );
    }
  }

  // ======================================================
  // NO MOTION FOR 10 SECONDS
  // ======================================================

  if (
    systemOn &&
    (now - lastMotionTime >= OFF_DELAY_MS)
  )
  {
    systemOn = false;

    // Local hardware first
    setRelay(false);
    showNoMotion();

    Serial.println("No Motion");

    // ======================================================
    // ENERGY CALCULATION
    // ======================================================

    unsigned long relayTime =
      now - relayOnStart;

    energyUsed +=
      (LIGHT_POWER * relayTime) /
      3600000.0;

    energySaved +=
      (LIGHT_POWER * OFF_DELAY_MS) /
      3600000.0;

    // ======================================================
    // FIREBASE
    // ======================================================

    updateFirebaseStatus(
      "No Motion Detected",
      "OFF",
      "Empty",
      "No Alerts"
    );

    updateEnergyFirebase();
  }

  delay(50);
}