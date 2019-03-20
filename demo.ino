    
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
 
#define SS_PIN 10 // Define o pino 10 como escravo
#define RST_PIN 9 // Define o pino 9 como Reset
#define RX 3
#define TX 2
#define pino_botao_le A0 // Define o pino A0 para o botao ler
//#define pino_botao_grava A1 // Define o pino A1 para o botao gravar
#define led_pinR 7 // Define o pino 7 para o LED
#define led_pinG 6 // Define o pino 6 para o LED
#define led_pinB 5 // Define o pino 5 para o LED
#define buzzer 4
#define BLUETOOTH_SPEED 9600

MFRC522 mfrc522(SS_PIN, RST_PIN); // Cria um objeto MFRC522
SoftwareSerial BTSerial(RX,TX); // Inicia modo Bluetooth

MFRC522::MIFARE_Key key; 
 
void setup() // Funcao de configuracao geral da pinagens
{
  pinMode(pino_botao_le, INPUT); // Inicia o botao para leitura
//  pinMode(pino_botao_grava, INPUT); // Inicia o botao para gravacao
  pinMode(led_pinR, OUTPUT); // Inicia o LED a ser piscado
  pinMode(led_pinG, OUTPUT);
  pinMode(led_pinB, OUTPUT);
  
  pinMode(buzzer, OUTPUT);
  
  BTSerial.begin(BLUETOOTH_SPEED); // Inicia HC 05
  Serial.begin(9600);   //Inicia a serial
  SPI.begin();      //Inicia  SPI bus
  
  mfrc522.PCD_Init();   //Inicia MFRC522
  mensageminicial(); //Mostra a mensagem inicial na tela
 
  //Prepara chave - padrao de fabrica = FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop() // Funcao principal
{
  int modo_le = digitalRead(pino_botao_le);
  if (modo_le != 0)
  {
    Serial.println("\nModo leitura");
    while(digitalRead(pino_botao_le) == 1) {}
    delay(1000);
    modo_leitura();
  }
 // Verifica se o botao modo gravacao foi pressionado
/*  int modo_gr = digitalRead(pino_botao_grava);
  if (modo_gr != 0)
  {
     Serial.println("\nModo gravacao");
     while (digitalRead(pino_botao_grava) == 1) {}
     delay(3000);
     modo_gravacao();
    }*/
}
  
void mensageminicial() // Funcao de mensagem inicial
  {
    Serial.println("\nPressione o botao para leitura");
    //Serial.println("\nSelecione o modo leitura ou gravacao");
  }
  
void ledpisca_ok() // Funcao para o LED piscar
  {
      //LED acende
    analogWrite(led_pinB, 0);
    analogWrite(led_pinG, 100);
    analogWrite(led_pinR, 0);
    tone(buzzer,500);
    delay(300);
    noTone(buzzer);
    delay(300);
    //LED apaga
    analogWrite(led_pinB, 0);
    analogWrite(led_pinG, 0);
    analogWrite(led_pinR, 0);
  }
void ledpisca_nao_ok() // Funcao para o LED piscar com erro
  {
      //LED acende
    analogWrite(led_pinB, 100);
    analogWrite(led_pinG, 0);
    analogWrite(led_pinR, 0);
    tone(buzzer,500);
    delay(300);
    noTone(buzzer);
    delay(300);
    tone(buzzer,500);
    delay(300);
    noTone(buzzer);
    delay(300);
    //LED apaga
    analogWrite(led_pinB, 0);
    analogWrite(led_pinG, 0);
    analogWrite(led_pinR, 0);
  }
  
void mensagem_inicial_cartao()
{
  Serial.println("\nAproxime o dispositivo do leitor...");
}

void modo_leitura() // Funcao de leitura do modulo RFID
{
  mensagem_inicial_cartao();
  //Aguarda cartao
  while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Mostra UID na serial
  Serial.print("UID da tag : ");
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    conteudo.concat(String(mfrc522.uid.uidByte[i]<0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
 
  //Obtem os dados do setor 1, bloco 4 = Nome
  byte sector         = 1;
  byte blockAddr      = 4;
  byte trailerBlock   = 7;
  byte status;
  byte buffer[18];
  byte size = sizeof(buffer);
 
  //Autenticacao usando chave A
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Autenticaçao falhou: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    ledpisca_nao_ok();
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Autenticaçao falhou: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    ledpisca_nao_ok();
  }
  for (byte i = 1; i < 16; i++)
  {
    Serial.print(char(buffer[i]));
  }
  Serial.println();
 
  //Obtem os dados do setor 0, bloco 1 = Sobrenome
  sector         = 0;
  blockAddr      = 1;
  trailerBlock   = 3;
 
  //Autenticacao usando chave A
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Autenticaçao falhou: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    ledpisca_nao_ok();
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Autenticaçao falhou: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    ledpisca_nao_ok();
  }

  for (byte i = 0; i < 16; i++)
  {
    Serial.print(char(buffer[i]));
  }
  Serial.println();
 
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
 // delay(3000);
  mensageminicial();
  ledpisca_ok();
}
/*void modo_gravacao() // Funcao para gravacao do modulo RFID
  {
    mensagem_inicial_cartao();
 //   Aguarda cartao
    while ( ! mfrc522.PICC_IsNewCardPresent()) {
      delay(100);
    }
    if ( ! mfrc522.PICC_ReadCardSerial())    return;
 
 //   Mostra UID na serial
    Serial.print(F("UID do Cartao: "));    //Dump UID
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
   // Mostra o tipo do cartao
    Serial.print(F("\nTipo do PICC: "));
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
 
    byte buffer[34];
    byte block;
    byte status, len;
 
    Serial.setTimeout(20000L) ;
    Serial.println(F("Digite o sobrenome,em seguida o caractere #\n"));
    len = Serial.readBytesUntil('#', (char *) buffer, 30) ;
    for (byte i = len; i < 30; i++) buffer[i] = ' ';
 
    block = 1;
   // Serial.println(F("Autenticacao usando chave A..."));
    status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
  //  Grava no bloco 1
    status = mfrc522.MIFARE_Write(block, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
    block = 2;
   // Serial.println(F("Autenticacao usando chave A..."));
    status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
  //  Grava no bloco 2
    status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
    Serial.println(F("Digite o nome, em seguida o caractere #\n"));
    len = Serial.readBytesUntil('#', (char *) buffer, 20) ;
    for (byte i = len; i < 20; i++) buffer[i] = ' ';
 
    block = 4;
    //Serial.println(F("Autenticacao usando chave A..."));
    status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
  //  Grava no bloco 4
    status = mfrc522.MIFARE_Write(block, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
    block = 5;
    //Serial.println(F("Autenticando usando chave A..."));
    status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() falhou: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
 
  //  Grava no bloco 5
    status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() falhou: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    //return;
    }
    else
    {
      Serial.println(F("\nDados gravados com sucesso!"));
    }
 
    mfrc522.PICC_HaltA(); // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    delay(3000);
    mensageminicial();
  }*/

