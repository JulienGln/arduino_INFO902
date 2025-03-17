#include <ArduinoBLE.h>
#include <Ultrasonic.h>
#include <Wire.h>
#include "rgb_lcd.h"

#define MY_UUID(val) ("D65D0396-" val "-4381-9985-653653CE831F")

const int PIN_ULTRASONS = 7;

Ultrasonic ultrasonic(PIN_ULTRASONS);
rgb_lcd lcd;


char bleName[] = "BLE_ultrasonic";
// BLE service
BLEService myService(MY_UUID("0000"));

// create  characteristics: that differ from the service UUID by 1 digit:
BLEByteCharacteristic firstChar(MY_UUID("0001"), BLERead | BLEWrite | BLENotify);

void setup() {
  lcd.begin(16, 2);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin()) {
    displayMessage("starting BLE failed");
    while (true);
  }

  // set the local name peripheral advertises:
  BLE.setLocalName(bleName);
  // print it:
  displayMsg(bleName);
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(myService);
  // add the characteristic:
  myService.addCharacteristic(firstChar);

  // add the service:
  BLE.addService(myService);

  // set characteristic values:
  firstChar.setValue(0);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, connectHandler);
  BLE.setEventHandler(BLEDisconnected, disconnectHandler);

  // assign event handlers for characteristics:
  firstChar.setEventHandler(BLEUpdated, characteristicUpdated);
  firstChar.setEventHandler(BLESubscribed, characteristicSubscribed);
  firstChar.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  BLE.advertise();
  displayMsg("Bluetooth OK, waiting for connections...");
}

void loop() {
  long dist;
  
  dist = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
  if( dist < 20 ) {
    sendMsg(dist);
    // displayMessage(String(RangeInCentimeters) + " cm");
    delay(1000);
  }

  BLE.poll();

}

void sendMsg(long dist) {
  // Logique pour envoyer un message ou effectuer une action
  // Par exemple, envoyer une notification BLE
  firstChar.setValue(1);  // Valeur d'exemple pour signaler qu'un objet a été détecté
  firstChar.writeValue(1); // Envoyer la notification
  displayMsg("Objet detecte, envoi message");
  delay(1000);
  displayMsg("Distance : " + String(dist) + "cm");

  // Déconnexion puis reconnexion au bout de quelques secondes
  BLE.disconnect();
  delay(9000);
  BLE.advertise();
}


// listen for BLE connect events:
void connectHandler(BLEDevice central) {
    // central connected event handler
    displayMsg("Connected event, central: ");
    displayMsg(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
}

// listen for BLE disconnect events:
void disconnectHandler(BLEDevice central) {
    // central disconnected event handler
    displayMsg("Disconnected event, central: ");
    displayMsg(central.address());
    digitalWrite(LED_BUILTIN, LOW);
}

// listen for characteristic subscribed events:
void characteristicSubscribed(BLEDevice central, BLECharacteristic thisChar) {
    // central wrote new value to characteristic, update LED
    displayMsg("Characteristic subscribed. UUID: ");
    displayMsg(thisChar.uuid());
}

// listen for characteristic unsubscribed events:
void characteristicUnsubscribed(BLEDevice central, BLECharacteristic thisChar) {
    // central wrote new value to characteristic, update LED
    displayMsg("Characteristic unsubscribed. UUID: ");
    displayMsg(thisChar.uuid());
}

// listen for characteristic updated events:
void characteristicUpdated(BLEDevice central, BLECharacteristic thisChar) {
    // central wrote new value to characteristic, update LED
    displayMsg("Characteristic updated. UUID: ");
    displayMsg(thisChar.uuid());
    displayMsg("   value: ");
    byte incoming = 0;
    thisChar.readValue(incoming);
    displayMsg(String(incoming));
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
