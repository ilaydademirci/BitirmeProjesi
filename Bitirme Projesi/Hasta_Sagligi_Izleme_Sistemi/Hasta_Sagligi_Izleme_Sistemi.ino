#define USE_ARDUINO_INTERRUPTS true              // Pulse kütüphanesinin daha doğru ölçüm yapabilmesi için bu ayarı etkinleştiriyoruz.
#define DEBUG true
#define SSID "TurkcellSuperOnline_"              // SSID-WiFi Adı
#define PASS "Efib1997" // "password"            // WiFi Şifresi
#define IP "184.106.153.149"                     // thingspeak.com IP Adresi
#include <SoftwareSerial.h>
#include "Timer.h"                               // Sistem tanımlı zamanlayıcıyı kütüphaneye ekledik.
#include <PulseSensorPlayground.h>               // Nabız hızı sensörünü kütüphaneye ekledik.
Timer t;
PulseSensorPlayground pulseSensor;               // Sensörümüzü kodumuzda kullanabilmek için onu obje olarak oluşturduk.

String msg = "GET https://api.thingspeak.com/update?api_key=HMHIJNH8MQEEPUAS";
SoftwareSerial esp8266(10,11);                   // Seri haberleşme pin ayarlarını yaptık.

//Variables
const int PulseWire = A0;                        // Pulse sensörümüzü bağlamış olduğumuz Analog pinini belirledik.
const int LED13 = 13;                            // Arduino üzerindeki ledin nabzımızla birlikte yanıp sönmesi için bu değişkeni 13 numaralı pin olarak ayarladık.
int Threshold = 580;                             // Belirlemiş olduğumuz eşik değerini bu değişkene atadık.
float myTemp;
int myBPM;
String BPM;
String temp;
int error;
int panic;
int raw_myTemp;
float Voltage;
float tempC;
void setup()
{
 
  Serial.begin(9600);                                    // Bilgisayarımızla olan seri iletişimi başlattık.
  esp8266.begin(115200);                                 // ESP8266 ile seri haberleşmeyi başlattık.
  pulseSensor.analogInput(PulseWire);                    // Pulse sensörünün bağlı olduğu pini belirledik.
  pulseSensor.blinkOnPulse(LED13);                       // Arduino üzerindeki ledin nabzımızla birlikte yanıp sönmesini sağladık.
  pulseSensor.setThreshold(Threshold);                   // Değişkene atamış olduğumuz eşik değerini uyguladık.

 
   if (pulseSensor.begin()) {
    Serial.println("Pulse sensörü objesini yarattık.");  // Arduino açıldığında veya Arduino sıfırlandığında bir kez yazdırır.
  }
  Serial.println("AT");                                  // "AT" komutu ile modül kontolünü yaptık.
  esp8266.println("AT");

  delay(3000);                                           // 3 sn bekle.

  if(esp8266.find("OK"))
  {
    connectWiFi();
  }
  t.every(1000, getReadings);
   t.every(1000, updateInfo);
}

void loop()
{
  panic_button();                                         // panic_button() fonksiyonu sürekli çalışacaktır.
start:                                                    // Etiket değeri oluşturduk.
    error=0;
   t.update();                                            // Zamanlayıcı güncellenecektir.
   
    if (error==1)
    {
      goto start;                                         // Start etiket değerine git.                          
    } 
 delay(4000);                                             // 4 sn bekle.
}

void updateInfo()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";                   // Thingspeak'e bağlandık.
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);                                     // "cmd" içerisindeki değeri yazdırdık. 
  esp8266.println(cmd);
  delay(2000);                                             // 2 sn bekle.
  if(esp8266.find("Error"))                                // Bağlantı hatası kontrolünü yaptık.
  {
    return;
  }
  cmd = msg ;
  cmd += "&field1=";                                       // Alan 1: BPM
  cmd += BPM;
  cmd += "&field2=";                                       // Alan 2: Sıcaklık                                      
  cmd += temp;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");                             // "AT+CIPSEND=" değerini yazdırdık.
  esp8266.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  esp8266.println(cmd.length());
  if(esp8266.find(">"))                                    // ESP8266 hazır olduğunda içindeki komutların çalışmasını sağladık.
  {
    Serial.print(cmd);
    esp8266.print(cmd);
  }
  else
  {
    Serial.println("AT+CIPCLOSE");                         // Bağlantıyı kapattık.
    esp8266.println("AT+CIPCLOSE");
   
    error=1;
  }
}

boolean connectWiFi()
{
  Serial.println("AT+CWMODE=1");
  esp8266.println("AT+CWMODE=1");                          // ESP8266 modülünü client olarak ayarladık.
  delay(2000);                                             // 2 sn bekle. 
  String cmd="AT+CWJAP=\"";                                // Komutun amacı, erişim noktasına (WiFi yönlendiricisi) bağlanmaktır.
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);                                     // "cmd" içerisindeki değeri yazdırdık.
  esp8266.println(cmd);
  delay(5000);                                             // 5 sn bekle. 
  if(esp8266.find("OK"))
  {
    return true;                                           // ConnectWiFi() fonksiyonu, WiFi'nin bağlı olmasına bağlı olarak "doğru" değerini döndürür.
  }
  else
  {
    return false;                                          // ConnectWiFi() fonksiyonu, WiFi'ye bağlı değilse "yanlış" değerini döndürür.
  }
}

void getReadings(){
  raw_myTemp = analogRead(A1);
  Voltage = (raw_myTemp / 1023.0) * 5000; // 5000 to get millivots.
  tempC = Voltage * 0.1; 
  myTemp = tempC;                                          // C derece olarak dönüştürme yaptık.
  Serial.println(myTemp);
  int myBPM = pulseSensor.getBeatsPerMinute();             // pulseSensor olarak oluşturduğumuz nesnedeki BMP'yi, "int" olarak döndüren işlevi çağırır.
                                                           // "myBPM" değişkenini BPM değerini tutacak şekilde ayarladık. 
if (pulseSensor.sawStartOfBeat()) {                        // Kalp atışının olup olmadığını görmek için "IF" yapısını kullanarak sürekli test edecek şekilde ayarladık. 
Serial.println(myBPM);                                     // "myBPM" içerisindeki değeri yazdırdık.
}

  delay(20);                                               // 20 milisaniye bekle.
    char buffer1[10];                                      // BPM ve temp değerleri için char dizisini tanımladık.
     char buffer2[10];
    BPM = dtostrf(myBPM, 4, 1, buffer1);
    temp = dtostrf(myTemp, 4, 1, buffer2);                 // Bu sensörlerin float değerini dtostrf() kullanarak string değişkenine dönüştürdük.
  }

void panic_button(){
  panic = digitalRead(8);
    if(panic == HIGH){
    Serial.println(panic);
      String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);                                     // "cmd" içerisindeki değeri yazdırdık.
  esp8266.println(cmd);
  delay(2000);                                             // 2 sn bekle.
  if(esp8266.find("Error"))
  {
    return;
  }
  cmd = msg ;
  cmd += "&field3=";    
  cmd += panic;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");                             // "AT+CIPSEND=" değerini yazdırdık.
  esp8266.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  esp8266.println(cmd.length());
  if(esp8266.find(">"))
  {
    Serial.print(cmd);                                     // Koşulu sağlarsa "cmd" içerisindeki değeri yazdırmasını sağladık.
    esp8266.print(cmd);
  }
  else
  {
    Serial.println("AT+CIPCLOSE");                         // Koşulu sağlamazsa, "AT+CIPCLOSE" değerini yazdırdık.
    esp8266.println("AT+CIPCLOSE");
    //Resend...
    error=1;
  }
}
}
