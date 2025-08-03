#include <OneWire.h>               // Bibliotecă pentru comunicație OneWire (pentru DS18B20)
#include <DallasTemperature.h>     // Bibliotecă care face citirea temperaturii de la DS18B20 mai ușoară

// DS18B20 este conectat la pinul PB6 (pin digital 14 în ATTinyCore)
const int oneWireBus = 14;         // Definim pinul de date pentru senzor
OneWire oneWire(oneWireBus);       // Inițializăm magistrala OneWire
DallasTemperature sensors(&oneWire); // Obiect pentru comunicarea cu senzorul DS18B20

// Pinii pentru segmentele A–G + DP, conectați la portul A, în ordine PA7 → PA0
const int segmentPins[8] = {7, 6, 5, 4, 3, 2, 1, 0};  // Pini digitali 7–0 = PA7–PA0

// Pinii care controlează activarea fiecărei cifre (DIG1–DIG3), conectați la PB3–PB5
const int digitPins[3] = {11, 12, 13};  // PB3, PB4, PB5

// Coduri binare pentru afișarea cifrelor 0–9 pe display cu ANOD COMUN
// Formatul: A B C D E F G DP — 0 = segment aprins, 1 = segment stins
const byte digitTable[10] = {
  B11000000,  // 0
  B11111001,  // 1
  B10100100,  // 2
  B10110000,  // 3
  B10011001,  // 4
  B10010010,  // 5
  B10000010,  // 6
  B11111000,  // 7
  B10000000,  // 8
  B10010000   // 9
};

// Variabile pentru stocarea cifrelor individuale (ex: 25.3°C → [2, 5, 3])
int digits[3] = {0, 0, 0};

// Indexul cifrei curente activată în multiplexare
int digitIndex = 0;

// Ultima temperatură citită
float lastTemperature = 0.0;

// Timpul ultimei actualizări de temperatură (pentru control temporizare)
unsigned long lastTempUpdate = 0;

// Intervalul dintre două citiri de temperatură (ms)
const unsigned long tempInterval = 700;

void setup() {
  sensors.begin();              // Inițializare senzor DS18B20
  sensors.setResolution(11);   // Setăm rezoluția senzorului la 11 biți (precizie 0.125°C, ~375ms)

  // Configurăm pinii segmentelor ca ieșiri și îi setăm stinși (HIGH = stins la anod comun)
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], HIGH);
  }

  // Configurăm pinii cifrelor ca ieșiri și îi lăsăm opriți (LOW = neactivat)
  for (int i = 0; i < 3; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }
}

void loop() {
  // Dacă au trecut cel puțin `tempInterval` milisecunde, citim o nouă temperatură
  if (millis() - lastTempUpdate >= tempInterval) {
    lastTempUpdate = millis();                      // Actualizăm momentul ultimei citiri
    sensors.requestTemperatures();                  // Cerem citirea temperaturii
    lastTemperature = sensors.getTempCByIndex(0);   // Citim temperatura în grade Celsius

    // Convertim temperatura într-un număr întreg cu o zecimală (ex: 25.3°C → 253)
    int temp = int(lastTemperature * 10);
    digits[0] = (temp / 100) % 10;   // Sute
    digits[1] = (temp / 10) % 10;    // Zeci
    digits[2] = temp % 10;           // Unități
  }

  // Afișăm cifra curentă (digitIndex): una câte una pentru multiplexare rapidă
  // Doar cifra din mijloc (digitIndex == 1) are punctul zecimal aprins
  showDigit(digitIndex, digits[digitIndex], digitIndex == 1);

  digitIndex = (digitIndex + 1) % 3;  // Trecem la următoarea cifră
  delayMicroseconds(100);            // Timp foarte scurt între comutări
}

// Funcție care afișează o cifră pe display
// digitIndex — ce cifră activăm (0, 1, 2)
// number — cifra 0–9 de afișat
// showDot — true dacă activăm punctul zecimal
void showDigit(int digitIndex, int number, bool showDot) {
  byte segments = digitTable[number];  // Codul binar pentru cifra respectivă

  // Setăm fiecare segment A–G
  for (int i = 0; i < 7; i++)
    digitalWrite(segmentPins[i], (segments >> i) & 0x01 ? HIGH : LOW);  // 0 = aprins

  // Setăm punctul zecimal (segmentul DP)
  digitalWrite(segmentPins[7], showDot ? LOW : HIGH);  // LOW = aprins (anod comun)

  // Activăm cifra curentă
  digitalWrite(digitPins[digitIndex], HIGH);

  // Ținem cifra aprinsă pentru o durată scurtă (multiplexare)
  delayMicroseconds(200);

  // Dezactivăm cifra (altfel s-ar suprapune cu următoarea)
  digitalWrite(digitPins[digitIndex], LOW);
}
