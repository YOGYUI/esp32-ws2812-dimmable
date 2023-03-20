# esp32-ws2812-dimmable
ESP32를 이용한 WS2812 RGB LED 제어 예제 프로젝트<br>
PWM 입력 가능한 LED 드라이버를 사용해 밝기 제어도 가능

요구사항
---
espressif의 esp-idf가 필요하다. (git submodule로 정의)<br>
esp-32 관련 모든 기능은 esp-idf를 통해 구현되었다.<br>
:warning: 추후 Matter 연동 프로젝트를 위해 v4.4.3으로 서브모듈 추가해둠
```shell
git submodule update --init
source ./sdk/esp-idf/install.sh
```

보드 핀맵
---
```c

```

RGB LED (WS2812S)
---
자세한 내용은 데이터시트 참고

구현내용
---
- 단일 GPIO로 RGB LED Data Line 제어
- LED 밝기 제어를 위한 PWM 제어
- Wi-Fi SoftAP 모드 활성화 (SSID: **YOGYUI-ESP32-TEST**)
- HTTP 웹 호스팅을 통한 LED 색상 및 밝기 제어 (HTTP Port: **80**)
    - Vue front-end framework
    - Vuetify UI framework
    - SoftAP 접속 후 브라우저에서 **10.11.12.1**로 접속 (DHCP)
    - Web 리소스는 빌드 후 바이너리 파일을 SPIFFS에 플래시

펌웨어 빌드 및 업로드
---
1. esp-idf 빌드 환경 준비
2. 빌드 타겟 지정 (ESP32): sdkconfig 설정파일 생성
3. 펌웨어 소스코드 빌드
4. 펌웨어 바이너리 파일 업로드
5. (optional) 웹서버용 이미지 생성 및 업로드 (SPIFFS)

데모
---
