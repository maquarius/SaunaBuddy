#include <WiFi.h>

const char* ssid = "";
const char* password = "";

WiFiServer server(80);

#include <Wire.h>
#include "ClosedCube_HDC1080.h"
#include <ESP32Servo.h>

//creating servo object
Servo myServo; 
int servoPin = 13;

ClosedCube_HDC1080 hdc1080;

String header;

float output2Temp = 0;
float output27Humidity = 0;

String output26State = "off";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("ClosedCube HDC1080 arduino test");

  hdc1080.begin(0x40);

  myServo.attach(servoPin);

  Serial.print("connecting to ");
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

void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient client = server.available();
  output2Temp = hdc1080.readTemperature();
  output27Humidity = hdc1080.readHumidity();
  Serial.println(output2Temp);
  Serial.println(output27Humidity);
  
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
   while (client.connected()) {
       if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /26/on") >= 0) {
                output26State = "on";
                myServo.write(180);
                delay(250); 
            
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
                output26State = "off";
                 myServo.write(0);
                 delay(250);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size   attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color:#555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");


            //Displays the humidty and temperature
            client.println("<p>Humidity:");
            client.println(output27Humidity);
            client.println("</p>");
            client.println("<p>Temperature:");
            client.println(output2Temp);
            
            //Adds a button that turns on servo
            //Servo doesn't work, since the ESP32 can't provide 5V. When that is sorted out it should work.
            client.println("</p>");
             if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button  button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  delay(2000);  //Delay 3s
}
