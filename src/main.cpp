     /////////////////////////////////////////////////////////////////
    //                    Bitirme Çalışması                        //
   //                   İkram MERT 191005008                      //
  //                WEB kontrollü Robot Süpürge                  //
 //               WEB ÜZERİNDEN ve OTONOM KONTROLÜ              //
/////////////////////////////////////////////////////////////////

//Kütüphaneleri Tanımlıyoruz
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Template ID'miz, Araç ismimiz ve Blynk tokenımız
#define BLYNK_TEMPLATE_ID "TMPL6JiEohPFa"
#define BLYNK_TEMPLATE_NAME "Robot Süpürge With wifi controller"
#define BLYNK_AUTH_TOKEN "tc0Ec4VIuC7G440kvMdmlweGAZMP48DS"
#define BLYNK_PRINT Serial

//oluşan token ı bir dizi içerisine atıyoruz
char auth[] = BLYNK_AUTH_TOKEN;

// Burada Wi-fi Bilgilerimizi giriyoruz
// Eğer Ağ şifresiz ise pass kısmısnı "" bu şekilde bırakın
char ssid[] = "ikram'sPC";
char pass[] = "ikoreis5353";

//Sağ ve Sol motorlar üzere Pin tanımları
int IN1 = 27;
int IN2 = 26;
int IN3 = 14;
int IN4 = 12;

//Ultra Sonic için tanımlamalar
#define trigPin 16
#define echoPin 17
long sure;
long uzaklik;

//Sistem otonom Durumu için tanımlamalar
int IN5 = 4;//Otonom bacağı
int state =0;

//Ölçümü Göster
int olcumistegi =0;

//5 numaralı bacak röle
int IN6 = 5;
int supurgeIstegi =0;

BLYNK_WRITE(V1) { //Wi-Fi ileri hareketi  
  digitalWrite(IN2, param.asInt());
  digitalWrite(IN4, param.asInt());
}

BLYNK_WRITE(V2) { //Wi-Fi Geri Hareketi 
  digitalWrite(IN1, param.asInt());
  digitalWrite(IN3, param.asInt());
}

BLYNK_WRITE(V3) { //Wi-Fi Sola Hareketi
  digitalWrite(IN2, param.asInt());
  digitalWrite(IN3, param.asInt());
}

BLYNK_WRITE(V4) { //Wi-Fi Sağa Hareketi
  digitalWrite(IN1, param.asInt());
  digitalWrite(IN4, param.asInt());  
}

BLYNK_WRITE(V0) { //Sistem Otonom
  state=param.asInt();
  //digitalWrite(IN5, param.asInt()); //Basıldığında State=1 Basılmadığında State 0
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW); 
}

BLYNK_WRITE(V6) {
  olcumistegi=param.asInt();
}

BLYNK_WRITE(V7) {
  supurgeIstegi=param.asInt();
}

void setup()
{
  // Haberleşme Hızı ve pinleri tanımlıyoruz
  // Çıkışlarımızı Lojik 0'a Çekiyoruz
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);

  //otonom bacak
  pinMode(IN5, OUTPUT);
  digitalWrite(IN5,LOW);

  //Sürüpge
  pinMode(IN6, OUTPUT);
  digitalWrite(IN6, LOW);

  //Ultra Sonic Sensör Kurulumu
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW); //trigi Başlangıçta Lojik 0'a Çekiyoruz

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 8080);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}

// Uzaklık sensörünün çalışması için bir Fonksiyon
void okuUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  sure = pulseIn(echoPin, HIGH);
  uzaklik = sure / 29.1 / 2;
  if (uzaklik > 400) 
  {
    uzaklik = 400;
  }
  //Serial.printf("Mesafe : %ld CM\n", uzaklik);
  Blynk.virtualWrite(V5, uzaklik);
  delay(200);
}

void Otonomileri(){
  digitalWrite(IN2, HIGH);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN1,LOW);
  digitalWrite(IN3,LOW);
}

void OtonomGeri(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN4,LOW);
}

void OtonomSag() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN4, HIGH);
}

void OtonomSol() {
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
}

void OtonomDurum() {
  if (uzaklik < 35) // Uzaklık 15'den küçük ise,
  {
    OtonomGeri();  // 150 ms geri git
    delay(250);
    OtonomSag();  // 250 ms sağa dön
    delay(150);
  }
  else {  // değil ise,
    Otonomileri(); // ileri git
  }
}

void loop()
{
  Blynk.run();

  if (supurgeIstegi==HIGH){
    digitalWrite(IN6, HIGH);
  }
  else{
    digitalWrite(IN6, LOW);  
  }
  
  //UtraSonik Sensör Veri Okuması
  if (olcumistegi==HIGH){
    okuUltrasonic();
  }
  
  if (state==HIGH){
    okuUltrasonic();//ilk mesafe okuyacak sonra o duruma göre hareket edicek
    OtonomDurum();  
  }

  if (!Blynk.connected()){
    Serial.println("\tBağlantı kesildi! Yeniden başlatılıyor...\n\tLütfen Bekleyiniz...");
    delay(1000);
    ESP.restart();    
  }
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}