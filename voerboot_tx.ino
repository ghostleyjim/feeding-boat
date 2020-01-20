// ZENDER (SENSOR) NODE, RF24 SENSOR NETWERK
// PROGRAMMEERVOORBEELD BITS & PARTS, WWW.BITSANDPARTS.EU

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// we definieeren de structuur van het te sturen bericht (het envelopje met daarin de gegevens)
struct sensor_bericht
{
  unsigned int selection;
  /*
     0 verifieer communicatie
     1 voorwaarts
     2 achteruit
     3 links
     4 rechts
     5 neutraal
  */
  unsigned int value;
};

byte red = 6;
byte green = 7;
byte blue = 8;

bool startflag = false;

RF24 radio(7,8);      // nRF24L01(+) transceiver aangesloten met pins 9 en 10
RF24Network network(radio);   // netwerk definiëren

const uint8_t deze_node = 1;   // adres van onze (sensor) node
const uint8_t andere_node = 0; // adres van de andere (ontvanger) node

int throttle;
int prev_throttle = 0;

//*******************************************************************************
// SETUP
//*******************************************************************************
void setup(void)
{
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  Serial.begin(115200);

  SPI.begin();
  radio.begin();
  network.begin(/*kanaal*/ 90, /*node adres*/ deze_node);

  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
  digitalWrite(blue, LOW);

  Serial.println("Sensor netwerk zender (sensor node) start...");
}

//*******************************************************************************
// HOOFD LOOP
//*******************************************************************************
void loop(void)
{
  // houdt het netwerk aan de gang
  network.update();
 
  if ( startflag == false ){
    Serial.print("Verzend start...");

    // we geven aan dat 'header' de structuur heeft van 'RF24NetworkHeader' (voorgedefinieerd in de RF24network library)
    RF24NetworkHeader header(andere_node);

    // we geven aan dat 'bericht' de structuur heeft van 'sensor_bericht'
    // en zetten er gelijk de gegevens in
    sensor_bericht bericht = { 0, 0 };

    digitalWrite(red, LOW);
    digitalWrite(blue, HIGH);

    // we sturen het bericht het netwerk op
    network.write(header, &bericht, sizeof(bericht));



    unsigned long time_from_send;
    time_from_send = millis();
    
    while (!network.available() ) {
      if (millis() - time_from_send > 20000) {
        Serial.println("not ackknowledged");
        break;
      }
    }
    while (network.available() ) {
      RF24NetworkHeader header;

      sensor_bericht ontvangen;

      network.read(header, &ontvangen, sizeof(ontvangen));

      if (ontvangen.selection == 0) {
        Serial.println("waarde klopt doorgaan");
        startflag = true;
        digitalWrite(blue, LOW);
        digitalWrite(green, HIGH);
      }
      else {
        Serial.println("verkeerde waarde ontvangen");
      }
    }
  }
gaslever();
}


void gaslever(){
throttle = analogRead(A2);
throttle = map(throttle, 513, 1023, -20, 20);
//Serial.println(throttle);

if (throttle != prev_throttle && throttle == 0 && (throttle % 2 == 0)){
  Serial.println("send neutraal");
  RF24NetworkHeader header(andere_node);
  sensor_bericht bericht { 5, 0 };
  network.write(header, &bericht, sizeof(bericht));
  prev_throttle = throttle;
}

if (prev_throttle != throttle && throttle > 0  && (throttle % 2 == 0)){
  Serial.println("send voorwaarts");
  RF24NetworkHeader header(andere_node);
  sensor_bericht bericht { 1, throttle };
  network.write(header, &bericht, sizeof(bericht));
  prev_throttle = throttle;
}

if (prev_throttle != throttle && throttle < 0  && (throttle % 2 == 0)){
  Serial.println("send achteruit");
  RF24NetworkHeader header(andere_node);
  sensor_bericht bericht { 2, throttle };
  network.write(header, &bericht, sizeof(bericht));
  prev_throttle = throttle;
}
}
