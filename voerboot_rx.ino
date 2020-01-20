#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// bericht structuur extra info kan hier ingevoerd worden
struct sensor_bericht
{
  unsigned int selection;
  /*
     0 verifieer communicatie
     1 voorwaarts
     2 achteruit
     3 links
     4 rechts
  */
  int value;
};

#define voor 3
#define achter 4
#define ROT 5


RF24 radio(7, 8);      // nRF24L01(+) transceiver aangesloten met pins 9 en 10
RF24Network network(radio);   // netwerk definiëren

const uint16_t deze_node = 0;   // adres van onze (ontvanger) node
const uint16_t andere_node = 1; // adres van de andere (sensor) node

byte previous_bericht;

bool changeflag = true;

//*******************************************************************************
// SETUP
//*******************************************************************************
void setup(void)
{
  Serial.begin(115200);

  SPI.begin();
  radio.begin();
  network.begin( 90, deze_node);

  Serial.println("Sensor netwerk ontvanger start...");

pinMode(voor, OUTPUT);
pinMode(ROT, OUTPUT);
pinMode(achter, OUTPUT);

digitalWrite(voor,LOW);
digitalWrite(achter, LOW);

}



//*******************************************************************************
// HOOFD LOOP
//*******************************************************************************
void loop(void)
{
  // houdt het netwerk aan de gang
  network.update();
  // is er data beschikbaar op het netwerk voor ons?
  while ( network.available() )
  {

    // we geven aan dat 'header' de structuur heeft van 'RF24NetworkHeader' (voorgedefinieerd in de RF24network library)
    RF24NetworkHeader header;

    // we geven aan dat 'bericht' de structuur heeft van 'sensor_bericht'
    sensor_bericht bericht;

    // we lezen het bericht van het netwerk
    network.read(header, &bericht, sizeof(bericht));

    previous_bericht = bericht.selection;
    
    if(bericht.selection != previous_bericht)
    {
      changeflag = true;
    }
  
    // de ontvangen gegevens die in het sensor bericht zaten printen
    if (bericht.selection == 0) {
      Serial.println("stuur bevestiging");
      RF24NetworkHeader header(andere_node);
      sensor_bericht bericht = {0, 0};
      network.write(header, &bericht, sizeof(bericht));
    }

    if (bericht.selection == 1) {
      //motor voorwaarts
      Serial.println("voorwaarts");
      Serial.print("waarde: ");
      Serial.println(bericht.value);

      if(changeflag == true){
      digitalWrite(achter, LOW);
      digitalWrite(voor, HIGH);
      changeflag = false;
      }
      
      int snelheid = (bericht.value / 2);
      snelheid = map(snelheid, 0 , 10 , 153 , 255);
      Serial.println(snelheid);
      analogWrite(ROT, snelheid);
    }

    if (bericht.selection == 2)
    {
      //motor achteruit
      Serial.println("achteruit");
      Serial.print("waarde: ");
      Serial.println(bericht.value);

       if(changeflag == true){
      digitalWrite(achter, HIGH);
      digitalWrite(voor, LOW);
      changeflag = false;
      }
      
      int snelheid = ((bericht.value / 2)*(-1));
      Serial.println(snelheid);
      snelheid = map(snelheid, 0 , 10 , 153 , 255);
      analogWrite(ROT, snelheid);
      
    }
    if (bericht.selection == 3)
    {
      //motor links
    }
    if (bericht.selection == 4)
    {
      //motor rechts
    }
    if (bericht.selection == 5)
    {
      Serial.println("neutraal");

       if(changeflag == true){
      digitalWrite(achter, LOW);
      digitalWrite(voor, LOW);
      changeflag = false;
      }
      
      int snelheid;     
      analogWrite(ROT, 0);
    }
  }




}
