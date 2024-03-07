# ESP32-conx-to-JSY-MK-194
ESP32C++ code to connect and test connexion to JSY-MK-194 power data mesurement module

France - Bordeaux 7 Mars 2024 

V0 but tested !


see below for shakespeare's langage

Dev ESP32 connecté au module JSY-MK-194R par le port UART2 en utilisant le protocole MODBUS RTU

Au préalable avec une carte de conversion TTL-USB, j'ai utilisé l'émulateur Modbus (Modbus-Tool) pour tester et mettre la vitesse à 19200 bauds

Bordeaux - 06 Mars 2024 

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
      ....
      circuit 2 : Ajouter 32 a toutes les adresses du circuit 1

connexions  - cablâge

      ESP32 <-> JSY-MK-194
      Tension 3.3V --- VCC      
       Rx2 GPIO16  --- Tx       
       Tx2 GPIO17  --- Rx       
         -   GND   --- GND

*/

/* English (... my english !)

Dev ESP32 connected to JSY-MK-194R module withing UART2 port - MODBUS RTU protocol

Fistly with TTL to USB board, I used Modbus emulator to test and directly set de baudrate to 19200 , you can do what you want but don't forget to change in the code accordingly (19200 by default)

Bordeaux - 07 Mars 2024 

Modbus table 0x48H + offset below  (from JSY_MK_194_Manual_english.pdf  5/5/2023)

      Voltage circuit 1   = 3, 4, 5, 6  >> Value on 4 bytes
      Current circuit 1   = 7, 8, 9, 10
      Power circuit 1     = 11, 12, 13, 14
      PositiveEnergy 1    = 15, 16, 17, 18
      powerfactor C1      = 19, 20, 21, 22
      Energy - circuit 1  = 23, 24, 25, 26
      Direction circuit 1 = 27                 >> if = 0x01 >  negative;  0x00 >  positive , One Byte
      Direction circuit 2 = 28
      not used            = 29, 30
      ....
      circuit 2 : Add 32 to Circuit 1 addresses

  Wiring      
  
            ESP32 <-> JSY-MK-194
       Power 3.3V --- VCC
       Rx2 GPIO16  --- Tx
       Tx2 GPIO17  --- Rx
         -   GND   --- GND
         
*/
