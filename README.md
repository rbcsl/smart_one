# Универсальный пульт дистанционного управления на Arduino Nano

### Установите Arduino IDE
Страница IDE на официальном сайте: <a href="https://www.arduino.cc/en/software" target="_blank">откройте ссылку в новой вкладке</a>

В ходе устновки IDE разрешайте установку драйверов нажимая на кнопку «Установить», эти драйверы позволят определять и работать с платами Arduino подключёнными по шине USB.

### Установите драйвер CH340

Ссылка на драйвер <a href="http://www.wch-ic.com/downloads/CH341SER_EXE.html" target="_blank">откройте ссылку в новой вкладке</a>

После установки перезагрузите компьютер.
### Установите библиотеки
Для работы скетча требуются следующие библиотеки:

+ RF24
+ Adafruit_SSD1306

Библиотеки можно найти в IDE и установить, либо распаковать папку из готового архива с библиотеками в директорию: *Этот компьютер > Документы > Arduino*

Ссылка на архив: [откройте ссылку в новой вкладке](https://github.com/rbcsl/Remote-controller-One/raw/main/libraries.zip)

### Загрузите скетч в Arduino Nano

Код с комментариями:
```C++
#include "RF24.h"               
#include <Adafruit_SSD1306.h>      

#define OLED_RESET  -1

RF24 radio(9, 10);                                //Создаём объект radiо, указывая номера выводов радиомодуля (CE, SS).
Adafruit_SSD1306 display(OLED_RESET);             //Создаём объект display для работы с OLED
  

int mas1[13];                                     //Массив для передачи данных
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //Массив для труб передачи данных
byte speed=3;                                     //Переменная скорости машинки (1-5)
byte turn=3;                                      //Переменная скорости повора (1-5)
byte state=1;                                     //Переменная состояния пульта, 1 - штатная работа мульта, 2 - настройка скорости машинки, 3 - настройка скорости поворота
bool engine=false;                                //Режим управления машинки true - стандартное управление, false - управление отдельными колёсами
bool update=true;                                 //Переменная статуса обновления дисплея, постоянно обновление дисплея значительно замедляет работу скетча, поэтому дисплей обновляется только если мы изменяем настройки пульта

bool b1,b2,b3,b4,b5,b6=0;                         //Переменные для работы с кнопками
bool flag1,flag2,flag3,flag4,flag5,flag6=false;
long t1,t2,t3,t4,t5,t6;
  
void setup() {
///////// Задаём режим работы пинов, назначенных на кнопки
  pinMode(2, INPUT_PULLUP);                      
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP); 
  pinMode(5, INPUT_PULLUP);  
  pinMode(7, INPUT_PULLUP); 
  pinMode(8, INPUT_PULLUP);
/////////

  radio.begin();                                  //Инициализация работы радиомодуля
  radio.setRetries(0, 15);                        //Указать максимальное количество попыток отправки данных и время ожидания.
  radio.enableAckPayload();                       //Разрешить размещать данные пользователя в пакете подтверждения приёма. 
  radio.setPayloadSize(32);                       //Установить статичный размер блока данных пользователя в байтах.
  radio.openWritingPipe(address[1]);              //Открыть трубу для передачи данных.
  radio.setChannel(96);                           //Установить радиочастотный канал связи, номер канала, указывается числом от 0 до 125.
  radio.setPALevel (RF24_PA_MAX);                 //Установить уровень усиления мощности передатчика.
  radio.setDataRate (RF24_250KBPS);               //Установить скорость передачи данных по радиоканалу.
  radio.powerUp();                                //Выйти из режима пониженного энергопотребления.
  radio.stopListening();                          //Прекратить прослушивание труб и переключиться в режим передатчика.

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);      //Инициализируем дисплей по адресу I2C
  display.clearDisplay();                         //Очистка дисплея
  display.setTextSize(2);                         //Задаём размер символов на дисплее
  display.setTextColor(WHITE);                    //Задаём цвет символов на дисплее, в монохромных дисплеях доступны WHITE и BLACK
  display.setCursor(0, 0);                        //Задаём положение курсора в пикселях 
  display.print ("Loading..");                    //Выводим текст в буфер дисплея
  display.display();                              //Отображаем всё что было в буфере дисплея на физический экран
  delay(1000);
}

void loop() { 
///////// Записываем в массив данные с потенциометров на джойстиках
  mas1[1]= map(analogRead(A2),0,1023,-100,100);  //ось
  mas1[2]= map(analogRead(A3),0,1023,100,-100); 
  mas1[3]= map(analogRead(A0),0,1023,-100,100); 
  mas1[4]= map(analogRead(A1),0,1023,100,-100); 
/////////

///////// Записываем в массив данные с кнопок
  mas1[5] = digitalRead(8);                       
  mas1[6] = digitalRead(7);
  mas1[7] = digitalRead(2); 
  mas1[8] = digitalRead(3);
  mas1[9] = digitalRead(4); 
  mas1[10] = digitalRead(5); 
/////////

///////// У каждого потенциометра имеется погрешность в несколько единиц, назначаем диапазон "мёртвой зоны" от -10 до 10
if (mas1[1]<10&&mas1[1]>-10)mas1[1]=0;           
if (mas1[2]<10&&mas1[2]>-10)mas1[2]=0;
if (mas1[3]<10&&mas1[3]>-10)mas1[3]=0;
if (mas1[4]<10&&mas1[4]>-10)mas1[4]=0;
/////////

//Участок ниже нужно оптимизировать дополнительно, ввести отдельную функцию для кнопок
/////////Выполняем необходимые действия при нажатии определённой кнопки, попутно мимимизируя вероятность повторных нажатий кнопки
if (b1 && !flag1 && millis() - t1 > 100) {flag1 = true;t1 = millis();
// Действия при нажатии кнопки b1
update = true;
engine=!engine;

}if (!b1 && flag1 && millis() - t1 > 100) {flag1 = false;t1 = millis();}b1 = digitalRead(8);

if (b2 && !flag2 && millis() - t2 > 100) {flag2 = true;t2 = millis(); 
// Действия при нажатии кнопки b2
if(state==1){
  display.clearDisplay(); 
  display.display();
}

}if (!b2 && flag2 && millis() - t2 > 100) {flag2 = false;t2 = millis();}b2 = digitalRead(7);

if (b3 && !flag3 && millis() - t3 > 100) {flag3 = true;t3 = millis();
// Действия при нажатии кнопки b3
update = true;
state++;
}if (!b3 && flag3 && millis() - t3 > 100) {flag3 = false;t3 = millis();}b3 = digitalRead(2);

if (b4 && !flag4 && millis() - t4 > 100) {flag4 = true;t4 = millis();
// Действия при нажатии кнопки b4
update = true;
state--;
}if (!b4 && flag4 && millis() - t4 > 100) {flag4 = false;t4 = millis();}b4 = digitalRead(3);

if (b5 && !flag5 && millis() - t5 > 100) {flag5 = true;t5 = millis();
// Действия при нажатии кнопки b5
update = true;
if (state==2){
 speed--; 
}
if (state==3){
 turn--; 
}
}if (!b5 && flag5 && millis() - t5 > 100) {flag5 = false;t5 = millis();}b5 = digitalRead(4);

if (b6 && !flag6 && millis() - t6 > 100) {flag6 = true;t6 = millis();
// Действия при нажатии кнопки b6
update = true;
if (state==2){
 speed++; 
}
if (state==3){
 turn++; 
}
}if (!b6 && flag6 && millis() - t6 > 100) {flag6 = false;t6 = millis();}b6 = digitalRead(5);
/////////



///////// Здесь контролируется диапазон рабочих переменных
if(turn>5)turn=5;
if(turn<1)turn=1;
if(state>3)state=1;
if(state<1)state=3;
if(speed>5)speed=5;
if(speed<1)speed=1;
/////////

///////// Записываем в массив данные режима работы двигателя, скорости движения, скорости поворота
mas1[0]=engine;
mas1[11]=speed;
mas1[12]=turn;
/////////


///////// Работа дисплея в состоянии 1
if (state==1){ 
if (update == true){
  display.clearDisplay(); 
  display.setCursor(10, 0);
  display.setTextColor(WHITE);
  if(engine==true){
  display.print ("[L]---[R]");
  }
  else {
  display.print ("[L]   [R]");
  }
  display.setCursor(16, 16);
  display.print (speed);
  display.setCursor(100, 16);
  display.print (turn);
  display.display(); 
  update = false;
}}
/////////

///////// Работа дисплея в состоянии 2
if (state==2){ 
if (update == true){
  display.clearDisplay(); 
  display.setCursor(10, 0);
  display.setTextColor(WHITE);
  display.fillRoundRect(-10, 15, 54, 16, 4, WHITE); 
  if(engine==true){
  display.print ("[L]---[R]");
  }
  else {
  display.print ("[L]   [R]");
  }
  display.setTextColor(BLACK);
  display.setCursor(16, 16);
  display.print (speed);
  display.setTextColor(WHITE);
  display.setCursor(100, 16);
  display.print (turn);
  display.display(); 
  update = false;
}}
///////// 


///////// Работа дисплея в состоянии 3
if (state==3){ 
if (update == true){
  display.clearDisplay(); 
  display.setCursor(10, 0);
  display.setTextColor(WHITE);
  display.fillRoundRect(84, 15, 54, 16, 4, WHITE); 
  if(engine==true){
  display.print ("[L]---[R]");
  }
  else {
  display.print ("[L]   [R]");
  }
  display.setCursor(16, 16);
  display.print (speed);
  display.setTextColor(BLACK);
  display.setCursor(100, 16);
  display.print (turn);
  display.display(); 
  update = false;
}}
/////////

  radio.write(&mas1, sizeof(mas1)); //Отправляем массив данных mas1[] по радиоканалу. В скобках указывается переменная для передачи и размер отправляемых данных в байтах.
  
}
```


### Полезная информация
Подробная информация по функционалу библиотеки RF24: [откройте ссылку в новой вкладке](https://micro-pi.ru/подключение-модуля-nrf24l01-к-arduino/)



