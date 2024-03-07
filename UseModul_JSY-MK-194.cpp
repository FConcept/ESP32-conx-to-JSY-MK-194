/* FranckM
Dev ESP32 connecté au module JSY-MK-194R par le port UART2 en utilisant le protocole MODBUS RTU
Au préalable j'ai utilisé l'émulateur Modbus (Modbus-Tool) pour tester et mettre la vitesse à 19200 bauds
Bordeaux - 07 Mars 2024 

Table Modbus 0x48H + offset ci-dessous  depuis la doc constructeur JSY_MK_194_Manual_english.pdf du 5/5/2023
Tension circuit 1      = 3, 4, 5, 6  >> la valeur occupe 4 octets
Intensité circuit 11   = 7, 8, 9, 10
Puissnce circuit 1     = 11, 12, 13, 14
Energie + circuit 1    = 15, 16, 17, 18
powerfactor C1         = 19, 20, 21, 22
Energie - circuit 1    = 23, 24, 25, 26
Sens courant circuit 1 = 27                 >> si = 0x01 > sens negatif;  0x00 > sens positif , la valeur occupe un octet
Sens courant circuit 2 = 28
not used = 29, 30

circuit 2 : Ajouter 32 a toutes les adresses du circuit 1

connexions   ESP32 --- JSY-MK-194
      Tension 3.3V --- VCC
       Rx2 GPIO16  --- Tx
       Tx2 GPIO17  --- Rx
         -   GND   --- GND
*/

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>

#define SERIAL2_BITRATE 19200 //  jsy-mk-194 - Initialisation préalable à 19200 bauds
#define RXD2 16     // par défaut pour UART2 de l'ESP32       
#define TXD2 17

const byte NumSlave=1; // numéro d'esclave modbus du module JSY
const boolean DebugTrame = true; //affiche ou pas les trames Modbus échangées
const uint Timeout_JSY = 1000; // timeout de réponse , on laisse le temps au module JSY de répondre
const uint TempoScrutation = 1000; // 1 seconde (on pourrait scruter le module dès la fin de la dernière réponse, le timeout_jsy temporise la scrutation au minimum)
unsigned long currentMillis = millis();
unsigned long startMillis;

class Module_JSY_MK_194 {
    private:
        byte TrameEnvoie[8] = {NumSlave, 0x03, 0x00, 0x48, 0x00, 0x0E, 0x44, 0x18};
        byte TrameRecue[64];
        int32_t Data32(uint8_t); // décode les valeurs transmises sur 4 octets
        byte Data8(uint8_t);     // décode les valeurs transmises sur 1 octet
    public:
        Module_JSY_MK_194();
        boolean Interroge();
        int32_t Tension(uint canal);
        int32_t Frequence();
        int32_t Puissance(uint canal);
        int32_t Intensite(uint canal);
        int32_t Energie_C(uint canal);
        int32_t Energie_I(uint canal);        
        byte SensCourant(uint canal);
};

Module_JSY_MK_194::Module_JSY_MK_194() {}

boolean Module_JSY_MK_194::Interroge() {
  int i;
  // On lit en une fois l'ensemble des données disponibles dans le module JSY
  // Requéte : 01 03 00 48 00 0E 44 18 soit 14 registres à partir de l'adresse hexa 48
  if (DebugTrame) Serial.print("Requete :");
  TrameEnvoie[0] = NumSlave;
  for ( i = 0; i < 8; i++)
  {
    Serial2.write(TrameEnvoie[i]);
   if (DebugTrame)  Serial.printf("<%02x>", TrameEnvoie[i]);
  }
  // on laisse le temps au module de répondre (communication à l'alternat) sans ralentir la scrutation :-) !
  startMillis = millis();
  while((Serial2.available()==0)&& (millis() - startMillis <Timeout_JSY)){}
    
  if (Serial2.available()==0){
    Serial.println("Timeout réponse du module JSY");
    return false;
  }
  // on doit recevoir une réponse de 56 octets (14 registres de 4 octets ) + 5 octets d'encapsulage de la trame = 61
  if (DebugTrame) Serial.print(" Réponse :");
  uint CRC = 0xFFFF;
  byte NumSlaveR = 0;
  i = 0;
  while (Serial2.available() && i < 64) {
    TrameRecue[i] = Serial2.read();  
    if (DebugTrame) Serial.printf("<%02x>", TrameRecue[i]);

    CRC ^= TrameRecue[i];     // on calcule le CRC en même temps que la reception, si à la fin le CRC=0 alors la trame est bonne !
    for (int j = 0; j < 8;++j){
      CRC= (CRC&0X0001) ?  CRC = (CRC >> 1) ^ 0xA001:CRC = CRC >> 1;
      }
    i++;
  }
  if (DebugTrame) Serial.printf(" CRC = %04x \n",CRC);
  NumSlaveR = TrameRecue[0];
  if ((i != 61)||(CRC!=0)||(NumSlaveR!=NumSlave)) {
    return false;
  }
  return true;
}

int32_t Module_JSY_MK_194::Data32(uint8_t i) {
 return TrameRecue[i + 3] + (TrameRecue[i + 2] << 8) + (TrameRecue[i + 1] << 16) + (TrameRecue[i] << 24);
}

byte Module_JSY_MK_194::Data8(uint8_t i) {
  return TrameRecue[i];
}

int32_t Module_JSY_MK_194::Tension(uint canal) { return Module_JSY_MK_194::Data32(3+(32*(canal-1))) * 0.0001; }
int32_t Module_JSY_MK_194::Energie_C(uint canal) { return Module_JSY_MK_194::Data32(15+(32*(canal-1))) * 0.1; }
int32_t Module_JSY_MK_194::Energie_I(uint canal) { return Module_JSY_MK_194::Data32(23+(32*(canal-1))) * 0.1; }
byte    Module_JSY_MK_194::SensCourant(uint canal) { return Module_JSY_MK_194::Data8(27+canal-1);}
int32_t Module_JSY_MK_194::Frequence() { return Module_JSY_MK_194::Data32(31) * 0.01; }
int32_t Module_JSY_MK_194::Intensite(uint canal) { return Module_JSY_MK_194::Data32(7+(32*(canal-1))) * 0.1; }
int32_t Module_JSY_MK_194::Puissance(uint canal) {
        int32_t p = Module_JSY_MK_194::Data32(11+(32*(canal-1))) * 0.0001;
        return (TrameRecue[27 + canal - 1] == 1 && p > 0) ? p = -p : p;
        }


Module_JSY_MK_194 Mod_jsy;


void setup() {
  Serial.begin(115200);
  Serial.println("Lancement programme ESP !");
  Serial2.begin(SERIAL2_BITRATE, SERIAL_8N1, RXD2, TXD2);  //  8-bit No parity 1 stop bit
  delay(200);
}

void loop() {

  if (Mod_jsy.Interroge()) {
    
    for (uint canal = 1; canal<3; canal++){
    Serial.printf("Circuit %d ", canal);
    Serial.printf(" en mode %s\n",Mod_jsy.SensCourant(canal)==true ? "Injection":"Consommation");
    Serial.printf("Tension   : %d V",Mod_jsy.Tension(canal));
    Serial.printf("  Intensité  : %d mA",Mod_jsy.Intensite(canal));
    Serial.printf("  Puissance : %d Watt\n",Mod_jsy.Puissance(canal));
    Serial.printf("Energie Consommée: %d Wh\n",Mod_jsy.Energie_C(canal));
    Serial.printf("        Injectée : %d Wh\n\n",Mod_jsy.Energie_I(canal));
    }

  } else {
    Serial.println("Data reçues Err !");
  }

  delay(TempoScrutation);
  /* l'utilisation du timeout (Timeout_JSY) permettra de scruter aussi rapidement qu'on veut ,
  ici le delay est juste là pour me laisser le temps de voir sur le monitoring les valeurs.

  Use the trametimeout (timeout_JSY) allows to scan the JSY module as fast as we want.
  Here the delay is just for me to have enought time to see the values on monitor ;-)
  */
}
