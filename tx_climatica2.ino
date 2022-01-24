/*
 * Creado por JGal94 
 * Si hay algun error o tiene alguna recomendacion hagamelo saber 
 */

#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_Sensor.h>  // incluye librerias para sensor BMP280
#include <Adafruit_BMP280.h>
#include "DHT.h"

Adafruit_BMP280 bmp;


float P0;
float PRESION;
float altitud;

const uint64_t pipeOut = 0xE8E8F0F0E1LL;   // codigo unico para la comunicacion entre los nrf24
#define DHTPIN 3  // se establece el pin del dht11
#define DHTTYPE DHT11 // se define cual sensor se esta usando
DHT dht(DHTPIN, DHTTYPE);
RF24 radio(9, 10); //  CN y CSN  pins del nrf
struct MyData {  // se crea la estructura MyData para enviar los datos por el nrf
  byte h;
  byte t;
  byte lluvia;
  byte p;
  byte alt;
  byte v;
};
MyData data;

double periodo = 60000; //Periodo de un minuto
int contador = 0;
unsigned long tiempoAhora = 0;
int valorFijado = 0;

extern volatile unsigned long timer0_millis;  // variable necesaria para resetear las millis al pasar las necesarias
unsigned long new_value = 0;

void setup()
{
  Serial.begin(9600); 
  dht.begin();
  radio.begin();
  pinMode(2,INPUT);  // entrada del sensor de hall para el anemometro
  if ( !bmp.begin() ) {       // si falla la comunicacion con el sensor mostrar
    Serial.println("BMP280 no encontrado !"); // texto y detener flujo del programa
    while (1);          // mediante bucle infinito
  }
  P0 = 1012;
  
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
}
void loop()
{
   if(millis() > (periodo + tiempoAhora)){
    valorFijado = contador; //Valor fijado se vuelve contador para dejar fijado por un minuto lo que estaba.
    Serial.println(contador); //Imprime el contador en el monitor Serial
    tiempoAhora = millis();
    data.v = contador;
    contador=0; //Reiniciamos contador
  }
  
  data.h = dht.readHumidity();
  data.t = dht.readTemperature();
  data.lluvia = analogRead(A0);
  PRESION = bmp.readPressure()/100;
  altitud = bmp.readAltitude(P0);
  data.p = PRESION/8;  // se divide el dato de la presion para hacer que quepa en 8bits lo que debe ser un valor entre 0 y 255
  data.alt = altitud/8;  // se divide el dato de la altitud para hacer que quepa en 8bits lo que debe ser un valor entre 0 y 255
  
  
  if (isnan(data.h) || isnan(data.t)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print("Humidity: ");        // se imprimen los datos en el monitor serial
  Serial.println(data.h);
  Serial.print("Temperature: ");
  Serial.println(data.t);
  Serial.print("Lluvia: ");
  if (data.lluvia > 100){
    Serial.println("Si");
  }else{
        Serial.println("No");
  }
  Serial.print("Presion: ");
  Serial.println(data.p*8);   // al igual que en el receptor los datos de data.p y data.alt deben multiplicarse por el numero divido para obtener el valor real
  Serial.print("Altura aproximada: ");
  Serial.println(data.alt*8);
  
  radio.write(&data, sizeof(MyData)); // se envian los datos

  if(digitalRead(2) == 0){
      //Espera 300 milisegundos para que no se acumulen muchas vueltas.
      delay(300);
      //Aumento el contador de vueltas en uno
      contador++;
  }
  //Imprime como van avanzando las rpm y al minuto reinicia
  Serial.println(valorFijado);  // mostramos los números o letras en serial
 if(millis() >= 60000){ // resetea las millis para evitar un desbordamiento
  setMillis(new_value);
 }
 
}
