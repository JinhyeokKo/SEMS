# SEMS-ESP 프로젝트

## 소개
SEMS-ESP 프로젝트는 ESP32 보드를 사용하여 주기적으로 온도를 측정하고, SQLite 데이터베이스에 저장한 후 SD 카드에 기록하는 시스템입니다.  
블루투스를 통해 wifi정보를 전송하여 wifi를 자동 연결 합니다.  
또한, Wi-Fi를 통해 보드에 내장된 웹사이트에 접속하여 데이터베이스를 제어할 수 있습니다.  
이 프로젝트는 IoT 및 데이터 로깅 애플리케이션에 적합합니다.

## 주요 기능
- **온도 측정**: DS18B20 센서를 사용하여 주기적으로 온도를 측정합니다.
- **데이터베이스 저장**: 측정된 온도를 SQLite 데이터베이스에 저장합니다.
- **SD 카드 기록**: SD카드 어댑터를 사용하여 데이터베이스 파일을 마운트된 SD 카드에 저장합니다.
- **BLE 프로비저닝**: 블루투스를 통해 초기 Wi-Fi 설정이 가능합니다.
- **웹 인터페이스**: Wi-Fi를 통해 ESP32 보드에 내장된 웹사이트에 접속하여 데이터베이스를 제어할 수 있습니다.
- **자동 네트워크 구성**: 고정 IP(192.168.0.3) 및 mDNS(sems.local) 지원으로 쉽게 접속할 수 있습니다.

## 프로젝트 구조
```
SEMS-ESP/
├── main/
│   ├── CMakeLists.txt
│   ├── main/idf_component.yml
│   ├── main/main.c
│   └── main.h
├── user_components/
│   ├── ble_prov/
│   ├── ds18b20/
│   ├── ntp/
│   ├── sdcard/
│   ├── sqlite/
│   └── websocket/
├── CMakeLists.txt
├── dependencies.lock
├── partitions.csv
└── sdkconfig
└── sdkconfig.ci
└── sdkconfig.defaults
```

## 네트워크 설정
1. **초기 설정**: 
   - 블루투스를 통한 Wi-Fi 프로비저닝
   - 디바이스 이름: "ESP_WIFI_PROV"로 검색 가능
   
2. **접속 방법**:
   - 고정 IP: `192.168.0.3`
   - mDNS 주소: `sems.local`

3. **보안**:
   - BLE 연결 시 보안 페어링 필요
   - Wi-Fi 자격증명은 암호화되어 전송

## 하드웨어 요구사항
1. ESP32-S3 보드 혹은 코드가 호환되는 보드
    > ESP32-S3-DevKitC-1-N32R8V보드를 기준으로 작성되었습니다.  
    sdkconfig.defaults와 partitions를 사용 보드에 맞게 수정해주세요.
2. DS18B20 온도 센서
3. SD 카드 모듈(SPI 방식)
4. 기타 연결용 점퍼 와이어

## 핀 연결
- DS18B20 : GPIO_NUM_4
- SD카드 모듈 :
    - MOSI : GPIO_NUM_11
    - MISO : GPIO_NUM_13
    - SCK : GPIO_NUM_12
    - CS : GPIO_NUM_10

## 기술 스택
- **프레임워크**: ESP-IDF v5.0+
- **데이터베이스**: SQLite3
- **통신 프로토콜**: 
  - BLE (프로비저닝)
  - Wi-Fi (웹 인터페이스)
  - WebSocket (실시간 데이터 통신)
- **파일 시스템**: FATFS (SD 카드)


## 설치 및 설정
1. **ESP-IDF 설치**: ESP32 개발을 위해 [ESP-IDF](https://github.com/espressif/esp-idf)를 설치합니다.
2. **프로젝트 클론**: 이 저장소를 클론합니다.
   ```
   git clone https://github.com/JinhyeokKo/sems-esp.git
   cd sems-esp
    ```
3. 종속성 설치: 필요한 종속성을 설치합니다.
    ```
    idf.py install
    ```
4. 빌드 및 플래시: 프로젝트를 빌드하고 ESP32 보드에 플래시합니다.
    ```
    idf.py build
    idf.py flash
    ```

## 사용 방법
1. 블루투스로 'ESP_WIFI_PROV'와 페어링(보드 자동 연결 허용)
2. 페어링 후 wifi 정보(SSID, PASSWORD) 전송
    > uuid가 존재하는 custom service로 write모드로 전송  
    형식 : ssid,pw

3. 웹 브라우저에서 `http://192.168.0.3` 또는 `http://sems.local` 접속하여 db 조회 및 제어

## 라이선스
이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 LICENSE 파일을 참조하세요.

### 외부 라이브러리 라이선스
이 프로젝트는 다음 외부 라이브러리를 사용하며, 각 라이브러리는 해당 라이선스에 따라 배포됩니다:
- [esp32-idf-sqlite3](https://github.com/nopnop2002/esp32-idf-sqlite3) : Apache 2.0 라이선스
- ESP-IDF 라이브러리 : Apache 2.0 라이선스

각 외부 라이브러리의 라이선스 조건을 준수해야 합니다. 자세한 내용은 각 라이브러리의 라이선스 파일을 참조하세요.