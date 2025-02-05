// ========================= 라이브러리 포함 =========================
// Wi-Fi 및 센서 관련 라이브러리 포함
#include <WiFi.h>                // ESP32의 Wi-Fi 기능을 위한 라이브러리
#include <Wire.h>                // I2C 통신을 위한 라이브러리
#include <Adafruit_BME280.h>     // Adafruit BME280 센서 라이브러리
#include <Adafruit_Sensor.h>     // 센서 데이터를 처리하기 위한 라이브러리

// ========================= 상수 및 전역 변수 =========================
// BME280 센서의 해수면 기압 기준값 설정 (hPa 단위)
#define SEALEVELPRESSURE_HPA (1013.25)

// BME280 객체 생성 (I2C 사용)
Adafruit_BME280 bme; 

// Wi-Fi 네트워크 정보 (사용자가 SSID와 비밀번호를 설정해야 함)
const char* ssid = " ";       // Wi-Fi SSID (네트워크 이름)
const char* password = " ";   // Wi-Fi 비밀번호

// 웹 서버 객체 생성 (포트 80에서 실행)
WiFiServer server(80);

// 클라이언트 요청을 저장하는 변수
String header;

// 현재 시간 및 이전 시간 변수
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000; // 타임아웃 시간(2초) 설정

// ========================= 초기 설정 (setup 함수) =========================
void setup() {
  Serial.begin(115200);  // 시리얼 모니터와 통신 시작 (속도: 115200 bps)
  bool status;

  // **BME280 센서 초기화**
  if (!bme.begin(0x76)) {  // I2C 주소 0x76에서 센서를 찾음
    Serial.println("Could not find a valid BME280 sensor, check wiring!"); // 센서가 감지되지 않을 경우 메시지 출력
    while (1);  // 오류 발생 시 무한 루프에 빠져 프로그램 정지
  }

  // **Wi-Fi 연결 시작**
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  // 주어진 SSID와 비밀번호로 Wi-Fi 네트워크 연결

  // **Wi-Fi 연결 대기 (연결될 때까지 . 출력)**
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // 연결될 때까지 점(.) 출력
  }

  // **Wi-Fi 연결 완료 후 IP 주소 출력**
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  // 연결된 장치의 IP 주소 출력

  // **웹 서버 시작**
  server.begin();
}

// ========================= 메인 루프 (loop 함수) =========================
void loop(){
  // **클라이언트 연결 확인**
  WiFiClient client = server.available();  // 서버에 접속한 클라이언트 확인

  if (client) {  // 새로운 클라이언트가 연결되었을 경우 실행
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");  // 새로운 클라이언트 연결됨을 출력
    String currentLine = "";        // 클라이언트 요청 데이터를 저장할 문자열

    // **클라이언트와 통신하는 동안**
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {  // 클라이언트로부터 데이터가 도착했을 경우
        char c = client.read();  // 한 글자씩 읽음
        Serial.write(c);  // 시리얼 모니터에 출력 (디버깅 용도)
        header += c;  // 전체 HTTP 요청 저장

        if (c == '\n') {  // 개행 문자를 확인
          if (currentLine.length() == 0) {
            // **HTTP 응답 헤더 전송**
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println(); // 빈 줄을 추가하여 헤더 종료

            // **HTML 웹 페이지 전송**
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // **CSS 스타일 추가**
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // **웹 페이지 제목**
            client.println("</style></head><body><h1>BME280 Webserver</h1>");

            // **측정값을 포함하는 HTML 테이블**
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");

            // **온도 (섭씨)**
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bme.readTemperature());
            client.println(" *C</span></td></tr>");  

            // **온도 (화씨)**
            client.println("<tr><td>Temp. Fahrenheit</td><td><span class=\"sensor\">");
            client.println(1.8 * bme.readTemperature() + 32);
            client.println(" *F</span></td></tr>");       

            // **기압**
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(bme.readPressure() / 100.0F);
            client.println(" hPa</span></td></tr>");

            // **고도**
            client.println("<tr><td>Approx. Altitude</td><td><span class=\"sensor\">");
            client.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m</span></td></tr>"); 

            // **습도**
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(bme.readHumidity());
            client.println(" %</span></td></tr>"); 
            
            client.println("</body></html>");  // HTML 문서 종료
            
            // **HTTP 응답 종료**
            client.println();
            break;  // 루프 탈출
          } else {
            currentLine = "";  // 현재 줄 초기화 (개행 문자 처리)
          }
        } else if (c != '\r') {  // 캐리지 리턴이 아니면
          currentLine += c;  // 현재 줄에 문자 추가
        }
      }
    }

    // **헤더 변수 초기화**
    header = "";
    
    // **클라이언트 연결 종료**
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
