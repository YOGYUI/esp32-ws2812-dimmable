# esp32-ws2812-dimmable
ESP32를 이용한 WS2812 RGB LED 제어 예제 프로젝트<br>
PWM 입력 가능한 LED 드라이버를 사용해 밝기 제어 가능

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
[definition.h 헤더파일](https://github.com/YOGYUI/esp32-ws2812-dimmable/blob/main/main/include/definition.h)에 ESP32 핀맵이 정의되어 있다.
```c
#define PIN_WS2812_PWM   19
#define PIN_WS2812_DATA  18
```

RGB LED (WS2812S)
---
자세한 내용은 데이터시트([WS2812S](http://doc.switch-science.com/datasheets/WS2812S+preliminaryV2.0.pdf)) 참고.<br>
High Bit, Low Bit를 위한 under 1ns delay는 모두 
```c
__asm__ __volatile__(
    "nop; nop; nop; nop; nop; nop; nop; nop;"
);
````
구문을 통해 구현하였다.

구현내용
---
- Single GPIO로 RGB LED Data Line 제어
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
    ```shell
    source ./sdk/esp-idf/export.sh
    ```
2. 빌드 타겟 지정 (ESP32): sdkconfig 설정파일 생성<br>
   플래시 영역 사이즈 4MB 설정, 커스텀 파티션 파일 (partitions.csv) 설정 (for SPIFFS)
    ```shell
    idf.py set-target esp32
    ```
3. 펌웨어 소스코드 빌드
    ```shell
    idf.py build
    ```
4. 펌웨어 바이너리 파일 업로드
    ```shell
    idf.py -p /dev/ttyUSB0 flash
    ```
5. (optional) 웹서버용 이미지 생성 및 업로드 (SPIFFS)
    esp-idf의 parttool.py, spiffsgen.py, esptool.py를 이용해야 하는데, 과정이 복잡하여 쉘스크립트로 자동화해두었다
    - 웹서버용 리소스 빌드 (npm)
        ```shell
        cd main/web
        npm run build
        ```
    - 웹서버용 리소스 바이너리 파일로 컴파일
        ```shell
        source ./script/build_web_resource.sh
        ```
    - 웹서버 파티션 이미지 플래시 업로드
        ```shell
        source ./script/flash_web_resource.sh
        ```
