#include <ESP8266WiFi.h>

const char* ssid = "rpi";
const char* password = "12345678";

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");

    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();
        Serial.println("WiFi received: " + cmd);

        // Forward to Uno R3
        Serial.println(cmd);  // This goes to Uno R3 over Serial

        // Optional: respond to client
        client.println("ACK:" + cmd);
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
