// Wi-Fi 라이브러리 로드
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

// 네트워크 자격 증명으로 바꾸기
const char* ssid = " ";
const char* password = " ";

// 웹 서버 포트 번호를 80으로 설정
WiFiServer server(80);

// HTTP 요청을 저장하는 변수
String header;

// 현재시간
unsigned long currentTime = millis();
// 이전시간
unsigned long previousTime = 0; 
// 시간 초과 시간을 밀리초 단위로 정의합니다(예: 2000ms = 2초).
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  bool status;

  // 기본 설정
  // &Wire2와 같은 와이어 라이브러리 객체를 전달할 수도 있습니다.
  //status = bme.begin();  
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // SSID와 비밀번호로 Wi-Fi 네트워크에 연결
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // 들어오는 클라이언트에 귀 기울이기

  if (client) {                             // 새 클라이언트가 연결되는 경우,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // 직렬 포트에서 메시지를 인쇄합니다.
    String currentLine = "";                // 클라이언트에서 들어오는 데이터를 저장할 문자열을 만듭니다.
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  //  루프가 클라이언트가 연결되어 있는 동안
      currentTime = millis();
      if (client.available()) {             // 클라이언트로부터 읽을 바이트가 있는 경우,
        char c = client.read();             // 바이트를 읽은 다음
        Serial.write(c);                    // 직렬 모니터를 인쇄합니다.
        header += c;
        if (c == '\n') {                    // 바이트가 개행 문자인 경우
          // 현재 줄이 비어 있으면 줄 바꿈 문자 두 개가 연속으로 표시됩니다.
          // 이 클라이언트 HTTP 요청의 끝이므로 응답을 보내세요:
          if (currentLine.length() == 0) {
            // HTTP 헤더는 항상 응답 코드(예: HTTP/1.1 200 OK)로 시작합니다.
            // 고객이 무엇이 올지 알 수 있도록 내용 유형을 지정한 다음 빈 줄을 표시합니다:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // HTML 웹 페이지 표시
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the table 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // 웹 페이지 제목
            client.println("</style></head><body><h1>BME280 Webserver</h1>");
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bme.readTemperature());
            client.println(" *C</span></td></tr>");  
            client.println("<tr><td>Temp. Fahrenheit</td><td><span class=\"sensor\">");
            client.println(1.8 * bme.readTemperature() + 32);
            client.println(" *F</span></td></tr>");       
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(bme.readPressure() / 100.0F);
            client.println(" hPa</span></td></tr>");
            client.println("<tr><td>Approx. Altitude</td><td><span class=\"sensor\">");
            client.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m</span></td></tr>"); 
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(bme.readHumidity());
            client.println(" %</span></td></tr>"); 
            client.println("</body></html>");
            
            // HTTP 응답이 다른 빈 줄로 끝납니다
            client.println();
            // 중간 루프에서 벗어나십시오
            break;
          } else { // 새 줄이 있는 경우 현재 줄을 지웁니다
            currentLine = "";
          }
        } else if (c != '\r') {  // 캐리지 리턴 캐릭터 말고 다른 게 있다면,
          currentLine += c;      // 현재 줄 끝에 추가합니다
        }
      }
    }
    // 헤더 변수 지우기
    header = "";
    // 연결을 닫습니다
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
