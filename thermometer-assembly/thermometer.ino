#include <OneWire.h>               // Bibliotecă pentru comunicație OneWire (pentru DS18B20)
#include <DallasTemperature.h>     // Bibliotecă care face citirea temperaturii de la DS18B20 mai ușoară

// digits e vectorul folosit in asm pentru a afisa fiecare cifra
// functiile de afisare sunt implementate in asm si apelate de aici
extern "C" {
  volatile uint8_t digits[3] = {0, 0, 0};
  void afiseaza_digit1();
  void afiseaza_digit2();
  void afiseaza_digit3();
}

// senzorul e legat pe pinul 14 (PB6)
const int oneWireBus = 14;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  sensors.begin();           // pornim comunicarea cu senzorul
  sensors.setResolution(11); // rezolutie 11 biti = citire rapida (~375ms)

  DDRA = 0xFF;               // toti pinii de pe PORTA ca output (segmentele)
  DDRB |= 0b00111000;        // PB3, PB4, PB5 ca output (cele 3 cifre ale afisajului)
}

// indexul cifrei curente afisate (0 = prima cifra din stanga)
int digitIndex = 0;

void loop() {
  static unsigned long lastRead = 0;   // timestamp-ul ultimei citiri
  static bool requested = false;       // flag ca am cerut o citire

  // daca nu am cerut deja o citire, trimitem comanda catre senzor
  if (!requested) {
    sensors.requestTemperatures();
    requested = true;
    lastRead = millis();
  }

  // asteptam 700ms (timpul minim pt citirea completa) si apoi preluam rezultatul
  if (requested && millis() - lastRead > 700) {
    float t = sensors.getTempCByIndex(0);  // citim temperatura in float
    int temp = int(t * 10);                // o transformam in int cu o zecimala (ex: 25.3 devine 253)
    digits[0] = (temp / 100) % 10;         // cifra sutelor
    digits[1] = (temp / 10) % 10;          // cifra zecilor
    digits[2] = temp % 10;                 // cifra unitatilor
    requested = false;
  }

  // afisam o cifra pe rand, la fiecare iteratie din loop
  if (digitIndex == 0) afiseaza_digit1();
  else if (digitIndex == 1) afiseaza_digit2();
  else afiseaza_digit3();

  digitIndex = (digitIndex + 1) % 3;       // trecem la cifra urmatoare
  delayMicroseconds(50);                  // intarziere mica pentru multiplexare
}