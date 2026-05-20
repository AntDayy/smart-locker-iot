#include <WiFi.h>
#include <PubSubClient.h>

// Cấu hình mạng WiFi ảo của Wokwi (giữ nguyên không sửa)
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Cấu hình MQTT Server trung gian (Phải trùng thông số với bên JSFiddle)
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_topic = "smartbox/cabinet/control_998877"; // Trùng với JSFiddle

const int relayPin = 13; // Chân nối với Relay điều khiển Đèn

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Đang kết nối mạng WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi đã kết nối thành công!");
}

// Hàm xử lý khi có lệnh gửi từ giao diện JSFiddle đến
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Tin nhắn mới nhận từ web [");
  Serial.print(topic);
  Serial.print("]: ");

  String message = "";
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Thực hiện lệnh đóng / mở tủ
  if (message == "OPEN")
  {
    digitalWrite(relayPin, HIGH); // Bật relay -> Đèn sáng (Mở tủ)
    Serial.println(">>> LỆNH: MỞ TỦ (ĐÈN SÁNG)");
  }
  else if (message == "CLOSE")
  {
    digitalWrite(relayPin, LOW); // Tắt relay -> Đèn tắt (Đóng tủ)
    Serial.println(">>> LỆNH: ĐÓNG TỦ (ĐÈN TẮT)");
  }
}

// Hàm tự động kết nối lại nếu bị ngắt mạng dữ liệu
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Đang kết nối lại với Máy chủ điều khiển...");
    String clientId = "ESP32SmartBox-";
    clientId += String(random(0, 1000));

    if (client.connect(clientId.c_str()))
    {
      Serial.println(" Đã kết nối!");
      client.subscribe(mqtt_topic); // Đăng ký lắng nghe kênh lệnh
    }
    else
    {
      Serial.print(" Lỗi kết nối, thử lại sau 5 giây. Mã lỗi: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Mặc định ban đầu đóng tủ (đèn tắt)

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop(); // Duy trì kết nối liên tục để nhận lệnh tức thời
}