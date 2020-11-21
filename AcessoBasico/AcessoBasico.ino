
/*  Autor: Alisson Rodolfo Leite
    Permitida cópia e reprodução desde que referenciado o nome de Alisson Rodolfo Leite

    Descrição: Fazer a leitura de cartões RFID e a executar a abertura ou fechamento de uma cancela controlada por um servo motor
   
    Biblioteca:  https://github.com/miguelbalboa/rfid
*/
// cartao padrao
byte gravado [4] = {0xB9, 0x94, 0x2D, 0xB3};
bool abrir = false;
//Pinos para comunicação RFID
//Signal      Pin8266   ESP32
//RST/Reset   D2        12
//SPI SDA     D4        13
//SPI MISO    D6        19
//SPI MOSI    D7        23
//SPI SCK     D5        18
//SCL         D5        22
//SDA         D3        21

#define RFID_SS_PIN     12
#define RFID_RST_PIN    13
#define PinPortao       15
#define PinPresenca     5
#define PinFalha        2

#define aberto 90 // angulo para aberto 
#define fechado 180 // angulo para fechado 

//#define COMMON_ANODE
#define Pres_Ativo LOW // definicao de quando é ativado a presença 

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif



/*
   Inclusão das Bibliotecas
*/
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>

//instancias
Servo portao;  //servo motor instancia
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);  // MFRC522 instancia

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // display


void cartao() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.println(F("Aproxime"));
  display.println(F("o cartao"));
  display.display();
}

void setup() {
  pinMode(PinPresenca, INPUT);
  pinMode(PinFalha, OUTPUT);
  digitalWrite(PinFalha, LED_OFF);
  Serial.begin(9600); // comunicação serial em 9600
  delay(2000);
  Serial.println("Bem Vindo");
  portao.attach(PinPortao);  // definição de conexão do Servo
  portao.write(fechado);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("Falha de Conexão com display OLED"));
  } else {
    Serial.println(F("Display OLED funcionando"));
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("Alisson RL"));
    display.setTextSize(2);
    display.setCursor(5, 30);
    display.println(F("IFSP"));
    display.display();
    delay(2000);
  }
  delay(1000); // evitar inferencia
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(1000); // evitar inferencia
  cartao();
}
//ALTERAR AQUI PARA MAIS CARTOES
bool confere(byte lido[]) {
  Serial.println("Entrou para conferir");
  for ( uint8_t k = 0; k < 4; k++ ) {
    if (lido[k] != gravado[k]) {
      return false;
    }
  }
  return true;
}

void falha() {
  Serial.println("Falha de Leitura");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.println(F("*DESCONHECIDO*"));
  display.display();
  for ( uint8_t k = 0; k < 30; k++ ) {
    digitalWrite(PinFalha, LED_ON);
    delay(100);
    digitalWrite(PinFalha, LED_OFF);
    delay(100);
  }
  cartao();
}

void sucesso(void) {
  Serial.println("Abrindo");
  portao.write(aberto);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(F("BEM VINDO"));
  display.println(F("Alisson RL"));
  display.println(F("AP 12 BL 1"));
  display.display();
  do {
    delay(2000);
  } while (digitalRead(PinPresenca) == Pres_Ativo);
  delay(5000);
  cartao();
}

void loop() {
  delay(1000);
  Serial.print("*");
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  Serial.println("Detectado Cartao");

  Serial.println("Leitura do Cartao");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  abrir = confere(mfrc522.uid.uidByte);
  Serial.println();
  Serial.println("Libera Cartao");
  mfrc522.PICC_HaltA();
  if (abrir) {
    sucesso();
    portao.write(fechado);
  } else {
    portao.write(fechado);
    falha();
  }
}
