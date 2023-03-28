#include "RF24.h"

///////// Назначение контактов драйвера L298N с цифровыми пинами Arduino
#define PIN_ENA 6 
#define PIN_ENB 5 
#define PIN_IN1 8 
#define PIN_IN2 7 
#define PIN_IN3 4 
#define PIN_IN4 3 

RF24 radio(9, 10);                                //Создаём объект radiо, указывая номера выводов радиомодуля (CE, SS).
byte pipeNo;                                      //Переменная номера трубы
int mas1[13];                                     //Массив для получения данных
int mas2[13];                                     //Массив для обработки данных

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //Массив для труб передачи данных
byte speed=150;                                   //Переменная скорости машинки (0-255)
byte turn=150;                                    //Переменная скорости повора (0-255)
bool engine=true;                                 //Режим управления машинки true - стандартное управление, false - управление отдельными колёсами

void setup() {
  radio.begin();                                  //Инициализация работы радиомодуля
  radio.setAutoAck(1);                            //Отправка пакетов о подтверждения приёма данных.   
  radio.setRetries(0, 15);                        //Указать максимальное количество попыток отправки данных и время ожидания.
  radio.enableAckPayload();                       //Разрешить размещать данные пользователя в пакете подтверждения приёма. 
  radio.setPayloadSize(32);                       //Установить статичный размер блока данных пользователя в байтах.
  radio.openReadingPipe(1, address[1]);           //Открыть трубу для получения данных.
  radio.setChannel(96);                           //Установить радиочастотный канал связи, номер канала, указывается числом от 0 до 125.
  radio.setPALevel (RF24_PA_MAX);                 //Установить уровень усиления мощности передатчика.
  radio.setDataRate (RF24_250KBPS);               //Установить скорость передачи данных по радиоканалу.
  radio.powerUp();                                //Выйти из режима пониженного энергопотребления.
  radio.startListening();                         //Переключиться в режим передатчика.
  
///////// Задаём режим работы пинов, назначенных на драйвер двигателя
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

///////// Подаём низкий сигнал на пины двигателя
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void loop() {
    
  while ( radio.available(&pipeNo)) {  // Открыть режим приёма на трубе
    
    radio.read( &mas1, sizeof(mas1) ); // Получаем данные массива из полученных пакетов 

    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, LOW);
///////// Записываем в переменные данные из полученного массива
    engine=mas1[0]; 
    mas2[1] = map(mas1[1], -100, 100, -100, 100); 
    mas2[2] = map(mas1[2], -100, 100, -500, 500); 
    mas2[3] = map(mas1[3], -100, 100, -500, 500);
    mas2[4] = map(mas1[4], -100, 100, -500, 500);
    speed=mas1[11]*50;
    turn=mas1[12]*50;


///////// Управление движением платформы в зависимости от выбранного режима управления 
    if ( engine == true) { 
      if (mas2[2]>20){  
      analogWrite(PIN_ENA, speed); 
      analogWrite(PIN_ENB, speed); 
      digitalWrite(PIN_IN1, HIGH);
      digitalWrite(PIN_IN2, LOW);
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, HIGH); 
        }
      if (mas2[2]<-20){ 
      analogWrite(PIN_ENA, speed); 
      analogWrite(PIN_ENB, speed); 
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, HIGH);
      digitalWrite(PIN_IN3, HIGH);
      digitalWrite(PIN_IN4, LOW); 
        }
       if (mas2[3]>30){
      analogWrite(PIN_ENA, turn); 
      analogWrite(PIN_ENB, turn); 
      digitalWrite(PIN_IN1, HIGH);
      digitalWrite(PIN_IN2, LOW);
      digitalWrite(PIN_IN3, HIGH);
      digitalWrite(PIN_IN4, LOW);
        }
      if (mas2[3]<-30){
      analogWrite(PIN_ENA, turn); 
      analogWrite(PIN_ENB, turn); 
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, HIGH);
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, HIGH);
        }
  }
    if ( engine == false) {
      if (mas2[4]>10){ 
      analogWrite(PIN_ENA, map(mas2[4], -100, 100, 0, 255)); 
      digitalWrite(PIN_IN1, HIGH);
      digitalWrite(PIN_IN2, LOW);
        }
      if (mas2[4]<-10){ 
      analogWrite(PIN_ENA, map(mas2[4], 100, -100, 0, 255)); 
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, HIGH);
        }
      if (mas2[2]>10){ 
      analogWrite(PIN_ENB, map(mas2[2], -100, 100, 0, 255)); 
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, HIGH); 
        }
      if (mas2[2]<-10){ 
      analogWrite(PIN_ENB, map(mas2[2], 100, -100, 0, 255)); 
      digitalWrite(PIN_IN3, HIGH);
      digitalWrite(PIN_IN4, LOW); 
        }    
    }
  }
}
