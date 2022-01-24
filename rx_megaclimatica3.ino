/*
 * Creado por JGal94 
 * Si hay algun error o tiene alguna recomendacion hagamelo saber 
 */


#include <UbidotsEthernet.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h> 
#include <LiquidCrystal.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(24,22,26); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

char datestring[20];  //string del modulo rtc ds1302

long intervalState1 = 3000; //variables necesarias para implementar funcionamiento por estados
unsigned long previousMillisState1 = 0;
long intervalState2 = 6000;
unsigned long previousMillisState2 = 0;
long intervalState3 = 300000;
unsigned long previousMillisState3 = 0;

int currentState = 0; //estados declarados
 #define nullstate 0
 #define state1 1
 #define state2 2
 #define state3 3

 int lcde = 0;    //estados para un switch y case para controlar el display
 #define disp1 0
 #define disp2 2
 #define disp3 3
 #define disp4 4
 

extern volatile unsigned long timer0_millis;  // variable necesaria para resetear las millis al pasar las necesarias
unsigned long new_value = 0;

char const * TOKEN = "aca va el token de ubidots"; 
char const * VARIABLE_LABEL_1 = "aca va el label de ubidots"; 
char const * VARIABLE_LABEL_2 = "aca va el label de ubidots"; 
char const * VARIABLE_LABEL_3 = "aca va el label de ubidots"; 
char const * VARIABLE_LABEL_4 = "aca va el label de ubidots"; 
char const * VARIABLE_LABEL_5 = "aca va el label de ubidots"; 

Ubidots client(TOKEN);

#define   nrf24_ce       21  // se definen los pines del nrf24
#define   nrf24_csn      20

const int rs = 6, en = 3, d4 = 5, d5 = 4, d6 = 7, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const uint64_t pipeIn = 0xE8E8F0F0E1LL;   //codigo unico para la comunicacion entre los nrf24

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED    
};
IPAddress ip(192, 168, 100, 200);  //se establece una ip fija para el arduino esto dependera de su servicio de internet
EthernetServer server(80);

struct MyData {  //se crea la estructura MyData que es lo que se recive del nrf24 transmisor
  byte h; 
  byte t;
  byte lluvia;
  byte p;
  byte alt;
  byte v;
};
MyData data;

RF24 radio(nrf24_ce, nrf24_csn);
void setup()
{
  Serial.begin(9600); 
  radio.begin();
  lcd.begin(16, 2);
  Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); 
    printDateTime(compiled);// usa la fecha compilada anteriormente, por lo que hay que usar uno de los codigos de ejemplo de la libreria para establecerle una fecha y hora correcta 
    Serial.println();
 Ethernet.begin(mac, ip);
 
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening();  //se abre a la escucha el nrf24
   server.begin();
}

void recvData()  //funcion que se encarga de recibir datos del nrf24
{
  if ( radio.available() ) {
    radio.read(&data, sizeof(MyData));
    } 
}
void loop()
{
    Ethernet.maintain();

    
    
  recvData();              
  Serial.print("Humidity: "); //se imprimen en el monitor serial los datos recibidos por el nrf24
  Serial.println(data.h);
  Serial.print("Temperature: ");
  Serial.println(data.t);
  Serial.print("Lluvia: ");
  if (data.lluvia > 100){
    Serial.println("Si");
  }else{
        Serial.println("No");
}
Serial.print("Viento: ");
Serial.println(data.v);
 
  EthernetClient client = server.available();  //se inicializa el servidor web 
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // envia una respuesta htpp
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // se cierra la conexion despues de la respuesta
          client.println("Refresh: 60");  //refresca la pgina cada 1 minutoa
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
          client.println("<style type='text/css'>");
          client.println("h1 { text-align: center;");
          client.println("background-color: #DEDEDE;");
          client.println("margin: 0;}");
          client.println("p { text-align: center;}");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("");
          client.println("</style>");
          client.println("<meta charset='UTF-8'>");
          client.println("<title>Meteorología</title>");
          client.println("<h1>Estacion Meteorológica Rohrmoser</h1>");
          client.println("</head>");
          client.println("<p>Temperatura: ");
          client.println(data.t);
          client.println("°C");
          client.println("</p>");
          client.println("<br>");
          client.println("<p>Humedad: ");
          client.println(data.h);
          client.println("%");
          client.println("</p>");
          client.println("<br>");
          client.println("<p>Lluvia: ");
          if (data.lluvia > 50){
            client.println("Si </p>");
          } else{
            client.println("No </p>");
          }
          client.println("<br>");
          client.println("<p>");
          client.println("Presion: ");
          client.println(data.p*8);
          client.println("hPa");
          client.println("</p>");
          client.println("<br>");
          client.println("<p>");
          client.println("Altitud aproximada: ");
          client.println(data.alt*8);
          client.println("m");
          client.println("</p>");
          client.println("<br>");
          client.println("<p>");
          client.println("Viento: ");
          client.println(data.v);
          client.println("km/h");
          client.println("</p>");
          client.println("<br>");
          client.println("<p>");
          client.println("Ultima vez actualizado los datos: ");
          client.println(datestring);
          client.println("</p>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // crea una nueva linea
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // recibe los caracteres de la linea actual
          currentLineIsBlank = false;
        }
      }
    }
    //da un tiempo para recibir datos
    delay(1);
    // cierra la conexion
    client.stop();
    Serial.println("client disconnected");
  }
  controlStates();
  states();
 if(millis() >= 300000){ // resetea las millis para evitar un desbordamiento
  setMillis(new_value);
 }
 
  
}
void controlStates(){ // funcion encargada de la maquina de estados
  unsigned long currentMillis = millis();
   if (((currentMillis - previousMillisState1) >= intervalState1) && (currentState == nullstate)) 
  {
    previousMillisState1 = currentMillis;
    currentState = state1;
  }

  if (((currentMillis - previousMillisState2) >= intervalState2) && (currentState == nullstate))
  {
    previousMillisState2 = currentMillis;
    currentState = state2;
  }
  if (((currentMillis - previousMillisState3) >= intervalState3) && (currentState == nullstate))
  {
    previousMillisState3 = currentMillis;
    currentState = state3;
  }
  
}
void states(){  // se encarga del cambio de los estados
  switch (currentState){
  case nullstate:
  break;
  case state1:
  stateone();
  break;
  case state2:
  statetwo();
  break;
   case state3:
  statethree();
  break;
  default:
  break;
}
if (currentState != nullstate)
  currentState = nullstate;
}
  
void stateone(){  // estado encargado de administrar el lcd
  switch (lcde){
    
    case disp1:
    lcd1();
    break;
    
    case disp2:
    lcd2();
    break;

    case disp3:
    lcd3();
    break;

    case disp4:
    lcd4();
    break;
    
default:
    break;
  
  }
}

void statetwo(){  // estado que limpia el lcd
    switch (lcde){
    case disp1:
    lcd.clear();
    lcde = disp2;  
    break;
    
    case disp2:
    lcd.clear();
    lcde = disp3;   
    break;

    case disp3:
    lcd.clear();
    lcde = disp4;   
    break;

    case disp4:
    lcd.clear();
    lcde = disp1;   
    break;
    
default:
    break;
  
  }
}
void statethree(){ // estado que envia cada cierto tiempo los datos a ubidots, esto para evitar sobre pasar los 4000 datos diarios que se pueden enviar a una cuenta gratuita de ubidots
 
 if( data.t > 0){
  client.add(VARIABLE_LABEL_1, data.t);}
  if(data.h > 0){
  client.add(VARIABLE_LABEL_2, data.h);}
  
  client.add(VARIABLE_LABEL_3, data.lluvia);
  client.add(VARIABLE_LABEL_4, data.p*8);
  
  client.sendAll();
  Serial.println(millis());
}
void setMillis(unsigned long new_millis){  // funcion que resetea el arduino 
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}

void lcd1(){             // primera pantalla del lcd
    lcd.setCursor(0,0);
  lcd.print(" T:");
  lcd.print(data.t);
  lcd.print("C ");
  lcd.print(" H:");
  lcd.print(data.h);
  lcd.print("%");
  lcd.setCursor(1,1);
  lcd.print("Lluvia: ");
   if (data.lluvia > 100){
   lcd.print("Si");
  }else{
      lcd.print("No");
      
}
}
void lcd2(){                         // segunda pantalla del lcd
    lcd.setCursor(0,0);
    lcd.print("Presion: ");
    lcd.print(data.p*8);
    lcd.print("hPa");
    lcd.setCursor(0,1);
    lcd.print("Alt aprox: ");
    lcd.print(data.alt*8);
    lcd.print("m");

}

void lcd3(){                                // tercera pantalla del lcd
  RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();
}

void lcd4(){                            // cuarta pantalla del lcd
  lcd.setCursor(3,0);
  lcd.print("Viento:");
  lcd.setCursor(3,2);
  lcd.print(data.v);
  lcd.print("km/h");
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)          //funcion encargada de recibir los datos del modulo reloj
{
    

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
            lcd.clear();
            lcd.setCursor(0,0);       
            lcd.print("Fecha:");
            lcd.print(dt.Month());
            lcd.print("/");
            lcd.print(dt.Day());
            lcd.print("/");
            lcd.print(dt.Year());
            
            lcd.setCursor(0,1);
            lcd.print("Hora:");
            lcd.print(dt.Hour());
            lcd.print(":");
            lcd.print(dt.Minute());
            lcd.print(":");
            lcd.print(dt.Second());
            
    Serial.print(datestring);         // muestra en el monitor serial los datos del modulo reloj
}
  
