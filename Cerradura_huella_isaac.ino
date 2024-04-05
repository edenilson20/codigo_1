#include <SD.h>

/*  Webpage: https://www.electronoobs.com/eng_arduino_tut41.php
 *  Schematic: https://www.electronoobs.com/eng_arduino_tut40_sch1.php
 *  Code: https://www.electronoobs.com/eng_arduino_tut40_code3.php
 *  Youtube Channel: https://www.youtube.com/c/Electronoobs */

////////////////////////////////////INCLUDE ALL LIBRARIES/////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  //sometimes the adress is not 0x3f. Change to 0x27 if it dosn't work.
//Fingerprint libraries
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
int getFingerprintIDez();
SoftwareSerial mySerial(10, 11);  // pin #2 is IN from sensor (GREEN wire)/ pin #3 is OUT from arduino (WHITE wire)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//Real time clock libraries
#include <DS3231.h>
DS3231  rtc(SDA, SCL);// Init the DS3231 using the hardware interface
//SD card read/write libraries
#include <SPI.h>
#include <SD.h>
File myFile;
//servo library
//#include <Servo.h>
//Servo miServo;


/////////////////////////////////////Input and outputs////////////////////////////////////////////////
int scan_pin = 13;      //Pin para el pulsador de escaneo
int add_id_pin = 12;    //Anclar para el botón pulsador agregar nuevo ID
int close_door = 9;     //Pin para cerrar el botón de la puerta
int green_led = 8;      //LEDs adicionales para etiquetas de puertas abiertas o cerradas
int red_led = 7;
int relay = 6;        //Pin para la señal Servo PWM

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////Editable variables//////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
int main_user_ID = 1;                 //Cambie este valor si desea un usuario principal diferente
String file_name_to_save = "blackBox.txt";
//////////////////////////////////////////////////////////////////////////////////////////////////////

//Otras variables utilizadas en el código. ¡No cambies!
bool scanning = false;
int counter = 0;
int id_ad_counter = 0;
bool id_ad = false;
uint8_t num = 1;
bool id_selected = false;
uint8_t id;
bool first_read = false;
bool main_user = false;
bool add_new_id = false;
bool door_locked = true;



void setup() {

  Serial.begin(57600);        //Inicie la comunicación serie para los datos de huellas dactilares TX RX.
  rtc.begin();                //Inicie el reloj en tiempo real (recuerde establecer la hora antes de este código)
  SD.begin(53);               //Inicie el módulo de tarjeta SD con el pin CS conectado al D53 del Arduino MEGA. Los otros pines son pines SPI

  /* Ahora abrimos el nuevo archivo creado, escribimos los datos y lo cerramos de nuevo */
  myFile = SD.open(file_name_to_save, FILE_WRITE);   //Create a new file on the SD card named before
  myFile.print("Door lock system started at ");
  myFile.print(rtc.getTimeStr()); myFile.print(" and day ");  myFile.print(rtc.getDateStr());
  myFile.println(" ");myFile.println(" ");
  myFile.close(); 
   
  lcd.init();                     //Inicie la pantalla LCD con comunicación i2c e imprima texto
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(" Presione  SCAN  ");
  lcd.setCursor(0,1);
  lcd.print("-Puerta cerrada-");

  //Definir pines como salidas o entradas
  pinMode(scan_pin,INPUT);  
  pinMode(add_id_pin,INPUT);  
  pinMode(close_door,INPUT);  
  pinMode(relay, OUTPUT);//configurar como salida
  //miServo.attach(servo);//*************
  //miServo.write(door_close_degrees);  //Door close *******
  
  digitalWrite(relay, LOW); // Envia señal de apagado AL RELAY
  digitalWrite(red_led,HIGH);         //Red LED turned ON, shows door CLOSED
  digitalWrite(green_led,LOW);        //Green LED turned OFF
  finger.begin(57600);                // set the data rate for the sensor serial port
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////VOID LOOP////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {
/////////////////////////////////////////BOTÓN DE CIERRE DE PUERTA PRESIONADO///////////////////////////////////
  if(digitalRead(close_door))
  {
    door_locked = true;
    //miServo.write(door_close_degrees); //Puerta cerrada << Grados de cierre de la puerta **********
    //digitalWrite(relay, LOW); //RELAY APAGADO
    digitalWrite(relay, LOW); //señal apagado
    digitalWrite(red_led,HIGH);       //LED rojo encendido, muestra la puerta CERRADA
    digitalWrite(green_led,LOW);       //LED verde apagado
    lcd.setCursor(0,0);
    lcd.print(" ¡Puerta cerrada! ");
    lcd.setCursor(0,1);
    lcd.print("                "); 
    delay(2000);     
    lcd.setCursor(0,0);
    lcd.print(" PRESIONE SCAN ");
    lcd.setCursor(0,1);
    lcd.print(" -Puerta cerrada- ");
    myFile = SD.open(file_name_to_save, FILE_WRITE);
    myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
    myFile.println(" -- Door was closed by a user"); //La puerta fue cerrada por un usuario
    myFile.close(); 
  }


////////////////////////////////Botón de escaneo presionado/////////////////////////////////////////////////
 if(digitalRead(scan_pin) && !id_ad)
 {
  myFile = SD.open(file_name_to_save, FILE_WRITE);
  myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
  myFile.println(" --Intento de abrir la puerta");
  myFile.close(); 
  scanning = true;
  lcd.setCursor(0,0);
  lcd.print(" Coloque el dedo");
  lcd.setCursor(0,1);
  lcd.print("SCANNING--------");
 }
  
 while(scanning && counter <= 60)
 {
  getFingerprintID();
  delay(100); 
  counter = counter + 1;
  if(counter == 10)
  {
    lcd.setCursor(0,0);
    lcd.print(" Coloque el dedo ");
    lcd.setCursor(0,1);
    lcd.print("SCANNING  ------");
  }

  if(counter == 20)
  {
    lcd.setCursor(0,0);
    lcd.print("  Place finger  ");
    lcd.setCursor(0,1);
    lcd.print("SCANNING    ----");
  }

  if(counter == 40)
  {
    lcd.setCursor(0,0);
    lcd.print("  Place finger  ");
    lcd.setCursor(0,1);
    lcd.print("SCANNING      --");
  }

  if(counter == 50)
  {
    lcd.setCursor(0,0);
    lcd.print("  Place finger  ");
    lcd.setCursor(0,1);
    lcd.print("SCANNING        ");
  }
  if(counter == 59)
  {
    lcd.setCursor(0,0);
    lcd.print(" Interrupción!  ");
    lcd.setCursor(0,1);
    lcd.print("Vuelve a intentarlo! ");
    myFile = SD.open(file_name_to_save, FILE_WRITE);
    myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
    myFile.println(" -- Scanning timeout!");//Tiempo de espera de análisis
    myFile.close(); 
    delay(2000);
    if(door_locked) //PUERTA CERRADA
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print(" -door  locked- ");//Puerta cerrada
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print("  -door  open-  "); //PUERTA ABIERTA
      digitalWrite(relay, HIGH);
      delay(5000);
      digitalWrite(relay, LOW);
      delay(5000);
     

    }
   }
   
 }
 scanning = false;
 counter = 0;
///////////////////////////////TERMINE CON LA PARTE DE ESCANEO



//////////////////////////////////////////Agregar nuevo botón presionado///////////////////////////////////////////
if(digitalRead(add_id_pin) && !id_ad)
 {
  myFile = SD.open(file_name_to_save, FILE_WRITE);
  myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
  myFile.println(" -- Agregar nuevo usuario. Requerir ident!");
  myFile.close(); 

  add_new_id = true;

  lcd.setCursor(0,0);
  lcd.print(" ESCAN usuario principal ");
  lcd.setCursor(0,1);
  lcd.print(" dedo primero!");  
  
  while (id_ad_counter < 40 && !main_user)
  {
    getFingerprintID();
    delay(100);  
    id_ad_counter = id_ad_counter+1;
    if(!add_new_id)
    {
      id_ad_counter = 40;
    }
  }
  id_ad_counter = 0;
  add_new_id = false;

  
  if(main_user)
  {
    lcd.setCursor(0,0);
    lcd.print(" Add new ID# to ");
    lcd.setCursor(0,1);
    lcd.print("  the database  ");  
    delay(1500);
    print_num(num);  
    id_ad = true;
    myFile = SD.open(file_name_to_save, FILE_WRITE);
    myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
    myFile.println(" -- Add new user permission granted");
    myFile.close(); 
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print("ERROR! Only main");
    lcd.setCursor(0,1);
    lcd.print("user can add IDs");  
    delay(1500); 
    if(door_locked)
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print(" -door  locked- ");   
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print("  -door  open-  "); 
      digitalWrite(relay, HIGH); // SEÑAL ABIERTA
    }
    id_ad = false;
    myFile = SD.open(file_name_to_save, FILE_WRITE);
    myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
    myFile.println(" -- The user does not have permission to add new user");
    myFile.close(); 
  }
 }

if(digitalRead(scan_pin) && id_ad)
 {  
  
  id=num;
  while (!  getFingerprintEnroll() );
  id_ad = false;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  New ID saved  ");
  lcd.setCursor(4,1);
  lcd.print("as ID #"); 
  lcd.setCursor(11,1);
  lcd.print(id);  
  delay(3000);
  if(door_locked)
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print(" -door  locked- ");   
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print("  -door  open-  ");  
      digitalWrite(relay, HIGH);
      delay(5000);
      digitalWrite(relay, LOW);
      delay(5000);
    }
  add_new_id = false;
  main_user = false;
  id_ad = false;
 
  
 }

if(digitalRead(add_id_pin) && id_ad)
 {
  num = num + 1;
  if(num > 16)
  {
    num=1;
  }
  print_num(num);  
 }







}//end of void


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


/*Esta función imprimirá los números de identificación al agregar una nueva identificación*/
void print_num(uint8_t)
{
  if (num == 1)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(">1  2   3   4   ");  
    delay(500);
  }
  if (num == 2)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 1 >2   3   4   ");  
    delay(500);
  }
  if (num == 3)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 1  2  >3   4   ");  
    delay(500);
  }
  if (num == 4)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 1  2   3  >4   ");  
    delay(500);
  }
  if (num == 5)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(">5  6   7   8   ");  
    delay(500);
  }
  if (num == 6)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 5 >6   7   8   ");  
    delay(500);
  }
  if (num == 7)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 5  6  >7   8   ");   
    delay(500);
  }
  if (num == 8)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 5  6   7  >8   ");   
    delay(500);
  }
  if (num == 9)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(">9  10  11  12  ");  
    delay(500);
  }
  if (num == 10)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 9 >10  11  12  ");  
    delay(500);
  }
  if (num == 11)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 9  10 >11  12  ");  
    delay(500);
  }
  if (num == 12)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 9  10  11 >12  ");  
    delay(500);
  }
  if (num == 13)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(">13  14  15  16 ");  
    delay(500);
  }
  if (num == 14)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 13 >14  15  16 ");  
    delay(500);
  }
  if (num == 15)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 13  14 >15  16 ");  
    delay(500);
  }
  if (num == 16)
  {
    lcd.setCursor(0,0);
    lcd.print("Select ID number");
    lcd.setCursor(0,1);
    lcd.print(" 13  14  15 >16 ");  
    delay(500);
  }
}









/*Esta función leerá la huella digital colocada en el sensor*/
uint8_t getFingerprintID()
{
  uint8_t p = finger.getImage();
  switch (p)
  {
    case FINGERPRINT_OK:
    break;
    case FINGERPRINT_NOFINGER: return p;
    case FINGERPRINT_PACKETRECIEVEERR: return p;
    case FINGERPRINT_IMAGEFAIL: return p;
    default: return p; 
  }
// OK success!

  p = finger.image2Tz();
  switch (p) 
  {
    case FINGERPRINT_OK: break;    
    case FINGERPRINT_IMAGEMESS: return p;    
    case FINGERPRINT_PACKETRECIEVEERR: return p;  
    case FINGERPRINT_FEATUREFAIL: return p;  
    case FINGERPRINT_INVALIDIMAGE: return p;    
    default: return p;
  }
// OK converted!

p = finger.fingerFastSearch();

if (p == FINGERPRINT_OK)
{
  scanning = false;
  counter = 0;
  if(add_new_id)
  {
    if(finger.fingerID == main_user_ID)
    {
      main_user = true;
      id_ad = false;
    }
    else
    {
      add_new_id = false;
      main_user = false;
      id_ad = false;
    }
    
  }
  else
  {
  //miServo.write(door_open_degrees); //grados para que la puerta esté abierta ***************** door open drees
  digitalWrite(relay, HIGH);
  digitalWrite(red_led,LOW);       //Red LED turned OFF
  digitalWrite(green_led,HIGH);    //Green LED turned ON, shows door OPEN
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("   User match   ");
  
  lcd.setCursor(0,1);
  lcd.print(" ID: #");
  
  lcd.setCursor(6,1);
  lcd.print(finger.fingerID);

  lcd.setCursor(9,1);
  lcd.print("%: ");

  lcd.setCursor(12,1);
  lcd.print(finger.confidence);

  myFile = SD.open(file_name_to_save, FILE_WRITE);
  myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
  myFile.print(" -- User match for ID# "); myFile.print(finger.fingerID);
  myFile.print(" with confidence: ");   myFile.print(finger.confidence); myFile.println(" - door open");
  myFile.close(); 
  door_locked = false;
  delay(4000);
  lcd.setCursor(0,0);
  lcd.print("   Press SCAN   ");
  lcd.setCursor(0,1);
  lcd.print("  -door  open-  ");
  delay(50);
  digitalWrite(relay, HIGH);
      delay(5000);
      digitalWrite(relay, LOW);
      delay(5000);
  }
}//dedo final OK

else if(p == FINGERPRINT_NOTFOUND)
{
  scanning = false;
  id_ad = false;
  counter = 0;
  lcd.setCursor(0,0);
  lcd.print("    No match    ");
  lcd.setCursor(0,1);
  lcd.print("   Try again!   ");
  add_new_id = false;
  main_user = false;  

  myFile = SD.open(file_name_to_save, FILE_WRITE);
  myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
  myFile.print(" -- No match for any ID. Door status still the same"); 
  myFile.close(); 
  delay(2000);
  if(door_locked)
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print(" -door  locked- ");   
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("   Press SCAN   ");
      lcd.setCursor(0,1);
      lcd.print("  -door  open-  ");  
      digitalWrite(relay, HIGH);
      delay(5000);
      digitalWrite(relay, LOW);
      delay(5000);
    }
  delay(2);
  return p;
}//end finger error
}// returns -1 if failed, otherwise returns ID #


int getFingerprintIDez() {
uint8_t p = finger.getImage();
if (p != FINGERPRINT_OK) return -1;
p = finger.image2Tz();
if (p != FINGERPRINT_OK) return -1;
p = finger.fingerFastSearch();
if (p != FINGERPRINT_OK) return -1;
// found a match!
return finger.fingerID;
}









//////////////////////////////////////////










/*Esta función agregará un nuevo ID a la base de datos*/
uint8_t getFingerprintEnroll() {

  int p = -1;
  if(!first_read)
  {
  lcd.setCursor(0,0);
  lcd.print("Add new as ID#  ");
  lcd.setCursor(14,0);
  lcd.print(id);
  lcd.setCursor(0,1);
  lcd.print("  Place finger  ");  
  }
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0,0);
      lcd.print("  Image taken!  ");
      lcd.setCursor(0,1);
      lcd.print("                ");  
      delay(100);
      first_read = true;
      break;
    case FINGERPRINT_NOFINGER:
      lcd.setCursor(0,0);
      lcd.print("Add new as ID#  ");
      lcd.setCursor(14,0);
      lcd.print(id);
      lcd.setCursor(0,1);
      lcd.print("  Place finger  "); 
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0,0);
      lcd.print("  Comunication  ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      lcd.setCursor(0,0);
      lcd.print("     -Image     ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    default:
      lcd.setCursor(0,0);
      lcd.print("    -Unknown    ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0,0);
      lcd.print("Image converted!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.setCursor(0,0);
      lcd.print("Image too messy!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      delay(1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0,0);
      lcd.print("  Comunication  ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0,0);
      lcd.print(" No fingerprint ");
      lcd.setCursor(0,1);
      lcd.print("features found  ");
      delay(1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0,0);
      lcd.print(" No fingerprint ");
      lcd.setCursor(0,1);
      lcd.print("features found  ");
      delay(1000);
      return p;
    default:
      lcd.setCursor(0,0);
      lcd.print("    -Unknown    ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      return p;
  }
  
  lcd.setCursor(0,0);
  lcd.print(" Remove finger! ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  lcd.setCursor(0,1);
  lcd.print("ID# ");
  lcd.setCursor(4,1);
  lcd.print(id);
  p = -1;
  lcd.setCursor(0,0);
  lcd.print("Place again the ");
  lcd.setCursor(0,1);
  lcd.print("same finger     ");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0,0);
      lcd.print("  Image taken!  ");
      lcd.setCursor(0,1);
      lcd.print("                "); 
      break;
    case FINGERPRINT_NOFINGER:
      lcd.setCursor(0,0);
      lcd.print("Place again the ");
      lcd.setCursor(0,1);
      lcd.print("same finger     ");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0,0);
      lcd.print("  Comunication  ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      lcd.setCursor(0,0);
      lcd.print("     -Image     ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    default:
      lcd.setCursor(0,0);
      lcd.print("    -Unknown    ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0,0);
      lcd.print("Image converted!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.setCursor(0,0);
      lcd.print("Image too messy!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      delay(1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0,0);
      lcd.print("  Comunication  ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0,0);
      lcd.print(" No fingerprint ");
      lcd.setCursor(0,1);
      lcd.print("features found  ");
      delay(1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0,0);
      lcd.print("  Comunication  ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      return p;
    default:
      lcd.setCursor(0,0);
      lcd.print("    -Unknown    ");
      lcd.setCursor(0,1);
      lcd.print("     ERROR!     ");
      delay(1000);
      return p;
  }
  
  // OK converted!
 lcd.setCursor(0,0);
 lcd.print(" Creating model ");
 lcd.setCursor(0,1);
 lcd.print("for ID# ");
 lcd.setCursor(8,1);
 lcd.print(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0,0);
    lcd.print(" Print matched! ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.setCursor(0,0);
    lcd.print("  Comunication  ");
    lcd.setCursor(0,1);
    lcd.print("     ERROR!     ");
    delay(1000);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.setCursor(0,0);
    lcd.print("Fingerprint did ");
    lcd.setCursor(0,1);
    lcd.print("not match       ");
    delay(1000);
    return p;
  } else {
    lcd.setCursor(0,0);
    lcd.print("    -Unknown    ");
    lcd.setCursor(0,1);
    lcd.print("     ERROR!     ");
    delay(1000);
    return p;
  }   
  
  lcd.setCursor(0,1);
  lcd.print("ID# ");
  lcd.setCursor(4,1);
  lcd.print(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0,0);
    lcd.print("     Stored     ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    myFile = SD.open(file_name_to_save, FILE_WRITE);
    myFile.print(rtc.getDateStr()); myFile.print(" -- "); myFile.print(rtc.getTimeStr());
    myFile.print(" -- New fingerprint stored for ID# "); myFile.println(id);
    myFile.close(); 
    delay(1000);
    first_read = false;
    id_ad = false;
    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.setCursor(0,0);
    lcd.print("  Comunication  ");
    lcd.setCursor(0,1);
    lcd.print("     ERROR!     ");
    delay(1000);
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    lcd.setCursor(0,0);
    lcd.print("Could not store ");
    lcd.setCursor(0,1);
    lcd.print("in that location");
    delay(1000);
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    lcd.setCursor(0,0);
    lcd.print("Error writing to");
    lcd.setCursor(0,1);
    lcd.print("flash           ");
    delay(1000);
    return p;
  } else {
    lcd.setCursor(0,0);
    lcd.print("    -Unknown    ");
    lcd.setCursor(0,1);
    lcd.print("     ERROR!     ");    
    delay(1000);
    return p;
  }   
}



