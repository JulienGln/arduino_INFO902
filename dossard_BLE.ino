#include <ArduinoBLE.h>
#include <Wire.h>
#include "rgb_lcd.h"

// UUID du service et de la caractéristique du périphérique
#define MY_UUID(val) ("D65D0396-" val "-4381-9985-653653CE831F")
const char* serviceUUID = MY_UUID("0000");
const char* charUUID = MY_UUID("0001");

// Pin du bipeur
const int BUZZER_PIN = 7;
int compteur = 0;

rgb_lcd lcd;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  lcd.begin(16, 2);

  // Initialiser BLE
  if (!BLE.begin()) {
    displayMsg("starting BLE failed!");
    while (1);
  }

  displayMsg("Scanning for peripherals...");
  BLE.scanForUuid(serviceUUID);
}

void loop() {
  // Vérifier si un périphérique a été trouvé
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    displayMsg("Found ");
    displayMsg(peripheral.address());

    // Arrêter le scan
    BLE.stopScan();

    controlPeripheral(peripheral);

    // Redémarrer le scan après un délai
    displayMsg("Restarting scan...");
    delay(100);
    BLE.scanForUuid(serviceUUID);
  }
}

void controlPeripheral(BLEDevice peripheral) {
  displayMsg("Connecting to peripheral...");

  if (peripheral.connect()) {
    displayMsg("Connected");
  } else {
    displayMsg("Failed to connect");
    return;
  }

  displayMsg("Discovering attributes...");
  if (peripheral.discoverAttributes()) {
    displayMsg("Attributes discovered");
  } else {
    displayMsg("Attribute discovery failed");
    peripheral.disconnect();
    return;
  }

  // Trouver la caractéristique
  BLECharacteristic characteristic = peripheral.characteristic(charUUID);

  if (!characteristic) {
    displayMsg("Peripheral does not have characteristic with UUID: ");
    displayMsg(charUUID);
    peripheral.disconnect();
    return;
  } else if (!characteristic.canSubscribe()) {
    displayMsg("Peripheral does not allow subscriptions to characteristic");
    peripheral.disconnect();
    return;
  } else if (!characteristic.subscribe()) {
    displayMsg("Subscription failed");
    peripheral.disconnect();
    return;
  } else {
    displayMsg("Subscribed to characteristic");
  }

  while (peripheral.connected()) {
    if (characteristic.valueUpdated()) {
      byte value = 0;
      characteristic.readValue(value);
      displayMsg("Received value: ");
      //displayMsg(String(value));
      displayMsg(String(++compteur));

      // Faire sonner le bipeur
      bip();
    }
  }

  displayMsg("Peripheral disconnected");
}

void bip() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
}

void displayMessage(String message) {
    // Efface l'écran
    lcd.clear();
  
    // Si le message est trop long, divise-le en deux lignes
    if (message.length() > 16) {
      // Trouve le dernier espace avant la 16ème position pour diviser proprement
      int splitIndex = message.lastIndexOf(' ', 16);
      if (splitIndex == -1) {
        // Si aucun espace n'est trouvé, divise simplement à la 16ème position
        splitIndex = 16;
      }
  
      // Affiche la première partie du message sur la première ligne
      String firstLine = message.substring(0, splitIndex);
      lcd.setCursor(0, 0);
      lcd.print(firstLine);
  
      // Affiche la deuxième partie du message sur la deuxième ligne
      String secondLine = message.substring(splitIndex + 1);
      lcd.setCursor(0, 1);
      lcd.print(secondLine);
    } else {
      // Si le message tient sur une seule ligne, affiche-le sur la première ligne
      lcd.setCursor(0, 0);
      lcd.print(message);
    }
  }
  
  void displayMsg(String msg) {
    displayMessage(msg);
  }
