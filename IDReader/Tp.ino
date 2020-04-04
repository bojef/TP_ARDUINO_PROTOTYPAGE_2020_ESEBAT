#include <LiquidCrystal.h> //Librerie necessaire pour la gestion des ecrans

#include <SPI.h> // SPI, Librerie necessaire pour la gestion de la carte RFID
#include <MFRC522.h> // RFID, Librerie necessaire pour la gestion de la carte RFID


LiquidCrystal LCD(12,11,5,4,3,2); //Definition d'une variable LCD representant notre ecrant
String data[63];


#define SS_PIN 10
#define RST_PIN 9
    
#define AccesFlag_PIN 2 //suppose que l'alarme est sur le port 2
#define Gate_PIN 3
#define Max_Acces 3

byte Count_acces=0; 
byte CodeVerif=0; 
byte Code_Acces[4]={0x20, 0x12, 0x23, 0x2B}; 

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Création de l'instance MFRC522.
MFRC522::MIFARE_Key key;

//MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// Init array that will store new NUID 
byte nuidPICC[4];

void setup() 
{ 
  SPI.begin();        // Initialisation du bus SPI
  mfrc522.PCD_Init(); // Inititilisation de la carte MFRC522
  
  // Init LEDs 
  pinMode(AccesFlag_PIN, OUTPUT);
  pinMode(Gate_PIN, OUTPUT);
  
  digitalWrite(AccesFlag_PIN, LOW);
  digitalWrite(Gate_PIN, LOW);

  LCD.begin(56,5);
  LCD.print("******* Presenté une carte *******");

}
 
void loop() 
{
  // Initialisé la boucle si aucun badge n'est présent 
  if ( !rfid.PICC_IsNewCardPresent())
    return;

  // Vérifier la présence d'un nouveau badge 
  if ( !rfid.PICC_ReadCardSerial())
    return;

  // Afffichage 
  LCD.clear();
  LCD.print("Un badge est détecté");

  // Enregistrer l’ID du badge (4 octets) 
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  // Vérification du code 
  CodeVerif= GetAccesState(Code_Acces,nuidPICC); 
  if (CodeVerif!=1)
  {
    Count_acces+=1;
    if(Count_acces==Max_Acces)
    {
     // Dépassement des tentatives (clignotement infinie) 
     while(1)
     {
      digitalWrite(AccesFlag_PIN, HIGH);
      delay(200); 
      digitalWrite(AccesFlag_PIN, LOW);
      delay(200); 
      // Affichage 
        LCD.clear();
        LCD.print("Alarme!");
     }
    }
    else
    {
      // Affichage 
      LCD.clear();
      LCD.print("Code érroné");
      // Un seul clignotement: Code erroné 
      digitalWrite(AccesFlag_PIN, HIGH);
      delay(1000); 
      digitalWrite(AccesFlag_PIN, LOW);
    }
  }
  else
  {
    // Affichage 
    LCD.clear();
    LCD.print("Ouverture de la porte");
    // Ouverture de la porte & Initialisation 
    digitalWrite(Gate_PIN, HIGH);
    delay(3000); 
    digitalWrite(Gate_PIN, LOW);
    Count_acces=0; 
  }

  // Affichage de l'identifiant 
  LCD.setCursor(0,10);
  LCD.print(" L'UID du tag est:");
  for (byte i = 0; i < 4; i++) 
  {
    LCD.setCursor(0,10+i);
    LCD.print(nuidPICC[i], HEX);
  }
  // Re-Init RFID
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD
}

byte GetAccesState(byte *CodeAcces,byte *NewCode) 
{
  byte StateAcces=0; 
  if ((CodeAcces[0]==NewCode[0])&&(CodeAcces[1]==NewCode[1])&&
  (CodeAcces[2]==NewCode[2])&& (CodeAcces[3]==NewCode[3]))
    return StateAcces=1; 
  else
    return StateAcces=0; 
}