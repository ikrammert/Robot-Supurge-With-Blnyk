#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <iostream>
#include <sstream>

//Ultra Sonic için tanımlamalar
#define trigPin 16
#define echoPin 17
long sure;
long uzaklik;

struct MOTOR_PINS
{
  int pinEn;  
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {22, 26, 27},  //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  {23, 12, 14},  //LEFT_MOTOR  Pins (EnB, IN3, IN4)
};

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define OtoNo 5
#define OtoYes 6
int OtoDurum = 0;

int IN6 = 5;
#define SupurON 8
#define SupurOFF 7
int SupurDurum = 0;

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;

const char* ssid     = "MyWiFiCar";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
        .arrows {
            font-size: 40px;
            color: red;
        }

        td.button {
            background-color: black;
            border-radius: 25%;
            box-shadow: 5px 5px #888888;
        }

        td.button:active {
            transform: translate(5px, 5px);
            box-shadow: none;
        }

        .noselect {
            -webkit-touch-callout: none;
            /* iOS Safari */
            -webkit-user-select: none;
            /* Safari */
            -khtml-user-select: none;
            /* Konqueror HTML */
            -moz-user-select: none;
            /* Firefox */
            -ms-user-select: none;
            /* Internet Explorer/Edge */
            user-select: none;
            /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
        }

        .slidecontainer {
            width: 100%;
        }

        .slider {
            -webkit-appearance: none;
            width: 100%;
            height: 20px;
            border-radius: 5px;
            background: #d3d3d3;
            outline: none;
            opacity: 0.7;
            -webkit-transition: .2s;
            transition: opacity .2s;
        }

        .slider:hover {
            opacity: 1;
        }

        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }

        .slider::-moz-range-thumb {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }
    </style>

</head>

<body class="noselect" align="center" style="background-color:white">

    <h1 style="color: teal;text-align:center;">Hash Include Electronics</h1>
    <h2 style="color: teal;text-align:center;">WiFi Tank Control</h2>

    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
        <tr>
            <td></td>
            <td class="button" ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8679;</span></td>
            <td></td>
        </tr>
        <tr>
            <td class="button" ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8678;</span></td>
            <td class="button"></td>
            <td class="button" ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8680;</span></td>
        </tr>
        <tr>
            <td></td>
            <td class="button" ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8681;</span></td>
            <td></td>
        </tr>
        <tr/>
        <tr/>
        <tr/>
        <tr/>
        <tr/>
        <tr/>
        <tr>
            <tr>
                <tr>
                    <td style="text-align:left;font-size:25px"><b>Hız:</b></td>
                    <td colspan=2>
                        <div class="slidecontainer">
                            <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendButtonInput("Speed",value)'>
                        </div>
                    </td>
                </tr>
                <table id="mainTable" style="width:350px;height:100px;margin:auto;table-layout:fixed" CELLSPACING=10>
                <tr>
                    <td>
                    <h2 style=>Otonom Mod</h2>
                    <td class="button" onclick='sendButtonInput("MoveCar","6")'><span>&#8679;</span>
                            <h1 style="color: white;">Aç</h01>
                    <td class="button" onclick='sendButtonInput("MoveCar","5")'><span>&#8679;</span>
                    <h1 style="color: white;">Kapa</h1>
                </table>
                 </tr>
                 <tr>
                <table id="mainTable" style="width:350px;height:100px;margin:auto;table-layout:fixed" CELLSPACING=10>
                <tr>
                    <td>
                    <h2 style=>Süpürge</h2>
                    <td class="button" onclick='sendButtonInput("MoveCar","8")'><span>&#8679;</span>
                            <h1 style="color: white;">Aç</h01>
                    <td class="button" onclick='sendButtonInput("MoveCar","7")'><span>&#8679;</span>
                    <h1 style="color: white;">Kapa</h1>
                </table>

                <script>
                    var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";
                    var websocketCarInput;

                    function initCarInputWebSocket() {
                        websocketCarInput = new WebSocket(webSocketCarInputUrl);
                        websocketCarInput.onopen = function(event) {
                            var speedButton = document.getElementById("Speed");
                            sendButtonInput("Speed", speedButton.value);
                        };
                        websocketCarInput.onclose = function(event) {
                            setTimeout(initCarInputWebSocket, 2000);
                        };
                        websocketCarInput.onmessage = function(event) {};
                    }

                    function sendButtonInput(key, value) {
                        var data = key + "," + value;
                        websocketCarInput.send(data);
                    }

                    window.onload = initCarInputWebSocket;
                    document.getElementById("mainTable").addEventListener("touchend", function(event) {
                        event.preventDefault()
                    });
                </script>
</body>

</html>
)HTMLHOMEPAGE";


void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

void OtonomDurum() {
  if (uzaklik < 35) // Uzaklık 15'den küçük ise,
  {
    rotateMotor(RIGHT_MOTOR, BACKWARD);
    rotateMotor(LEFT_MOTOR, BACKWARD);  // 150 ms geri git
    delay(250);
    rotateMotor(RIGHT_MOTOR, BACKWARD);
    rotateMotor(LEFT_MOTOR, FORWARD);  // 250 ms sağa dön
    delay(150);
  }
  else {  // değil ise,
    rotateMotor(RIGHT_MOTOR, FORWARD);
    rotateMotor(LEFT_MOTOR, FORWARD); // ileri git
  }
}

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
  Serial.printf("Mesafe : %ld CM\n", uzaklik);
  delay(200);
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue);  
  switch(inputValue)
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                  
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD); 
      break;
 
    case STOP:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;

    case OtoNo:
      Serial.printf("Oto No basıldı");
      OtoDurum = 0;
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
      delay(200);
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
      break;

    case OtoYes:
      Serial.printf("Oto Yes Basıldı");
      OtoDurum = 1;
      break;

    case SupurON:
      Serial.printf("SupurON aktif");
      SupurDurum=1;
      break;

    case SupurOFF:
      Serial.printf("SupurOFF aktif");
      SupurDurum = 0;
    break;

    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(STOP);
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str());     
        if (key == "MoveCar")
        {
          moveCar(valueInt);        
        }
        else if (key == "Speed")
        {
          ledcWrite(PWMSpeedChannel, valueInt);
        }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setUpPinModes()
{
  //Set up PWM
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
      
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinEn, OUTPUT);    
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  

    /* Attach the PWM Channel to the motor enb Pin */
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }
  moveCar(STOP);

  //Ultra Sonic Sensör Kurulumu
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW); //trigi Başlangıçta Lojik 0'a Çekiyoruz
  //Sürüpge
  pinMode(IN6, OUTPUT);
  digitalWrite(IN6, LOW);
}


void setup(void) 
{
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
      
  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("HTTP server started");

}


void loop() 
{
  wsCarInput.cleanupClients();
  if (OtoDurum==1){
    okuUltrasonic();//ilk mesafe okuyacak sonra o duruma göre hareket edicek
    OtonomDurum();
  }
  if (SupurDurum==1){
    digitalWrite(IN6, HIGH);
  }
  else{
    digitalWrite(IN6, LOW);
  }
}