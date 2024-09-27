#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>  // 서보 모터 라이브러리

Servo myservo;  // 서보 모터 객체 생성

int servoPosOn = 53;          // 서보 모터가 스위치를 누르는 각도
int servoPosWait = 90;
int servoPosOff = 122;        // 서보 모터가 스위치를 끄는 각도
int servoPin = 16;            // 서보 모터가 연결된 핀

BLEServer* pServer;
BLECharacteristic* pCharacteristic;
BLEAdvertising* pAdvertising;

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;  // 장치가 연결되었음을 알림
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;  // 장치가 연결되지 않았음을 알림
    Serial.println("Device disconnected");
    
    // 연결이 끊긴 후 광고 다시 시작
    pAdvertising->start();
    Serial.println("Advertising restarted");
  }
};

void setup() {
  Serial.begin(115200);
  
  // 서보 모터 초기 설정
  myservo.attach(servoPin);    
  myservo.write(servoPosWait);  // 서보 모터를 대기 위치로 설정 (스위치 OFF)
  
  // BLE 장치 초기화
  BLEDevice::init("ESP32_BLE_Servo");  // BLE 장치 이름 설정 (스마트폰에서 보이는 이름)
  pServer = BLEDevice::createServer();  // BLE 서버 생성
  pServer->setCallbacks(new MyServerCallbacks());  // 연결 및 해제 콜백 설정
  
  // BLE 서비스 생성
  BLEService* pService = pServer->createService("12345678-1234-1234-1234-123456789012");  // 서비스 UUID

  // BLE 특성 생성 (읽기/쓰기 가능)
  pCharacteristic = pService->createCharacteristic(
                      "87654321-4321-4321-4321-210987654321",  // 특성 UUID
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  
  // 기본 특성 값 설정
  pCharacteristic->setValue("Hello from ESP32!");
  
  // 서비스 시작
  pService->start();
  
  // BLE 광고 시작
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  
  Serial.println("BLE Device is Ready to Pair");
}

void loop() {
  // BLE 특성 값 읽기 (std::string 대신 char array 사용)
  String value = pCharacteristic->getValue();
  
  // 데이터를 String으로 변환
  String command = String(value.c_str());  // std::string을 Arduino String으로 변환

  if (command.length() > 0) {  // 값이 있을 때만 처리
    Serial.println("Received Value: " + command);  // 수신한 값을 시리얼 모니터에 출력
    
    if (command == "on") {  // 명령이 "on"이면
      myservo.write(servoPosOn);  // 서보 모터를 스위치를 누르는 각도로 이동
      delay(2000);
      myservo.write(servoPosWait);  // 대기 위치로 돌아감
      Serial.println("Switch ON");
      pCharacteristic->setValue("Switch ON");  // 응답 값 설정
    } 
    else if (command == "off") {  // 명령이 "off"이면
      myservo.write(servoPosOff);  // 서보 모터를 스위치를 끄는 각도로 이동
      delay(2000);
      myservo.write(servoPosWait);  // 대기 위치로 돌아감
      Serial.println("Switch OFF");
      pCharacteristic->setValue("Switch OFF");  // 응답 값 설정
    } 
    else {
      Serial.println("Unknown command");
      pCharacteristic->setValue("Unknown command");  // 알 수 없는 명령에 대한 응답
    }

    // 특성 값을 초기화하여 다시 수신 준비
    pCharacteristic->setValue("");
  }
}
