// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "MishMashLabs";
const char* password = "mishmash";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output15State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output15 = 15;
const int output4 = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Define Authentication
const char* base64Encoding = "TWlzaE1hc2hMYWJzOm1pc2htYXNo";  // base64encoding user:pass - "dXNlcjpwYXNz", MishMashLabs:mishmash - "TWlzaE1hc2hMYWJzOm1pc2htYXNo"

// Define Static IP Settings
IPAddress local_IP(192,168,43,201);
IPAddress gateway(192,168,43,1);
IPAddress subnet(255,255,0,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secondaryDNS(8,8,4,4);

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output15, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output15, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
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
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // check base64 encode for authentication
            // Finding the right credentials
            if (header.indexOf(base64Encoding)>=0)
            {
            
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              
              // turns the GPIOs on and off
              if (header.indexOf("GET /15/on") >= 0) {
                Serial.println("GPIO 15 on");
                output15State = "on";
                digitalWrite(output15, HIGH);
              } else if (header.indexOf("GET /15/off") >= 0) {
                Serial.println("GPIO 15 off");
                output15State = "off";
                digitalWrite(output15, LOW);
              } else if (header.indexOf("GET /4/on") >= 0) {
                Serial.println("GPIO 4 on");
                output4State = "on";
                digitalWrite(output4, HIGH);
              } else if (header.indexOf("GET /4/off") >= 0) {
                Serial.println("GPIO 4 off");
                output4State = "off";
                digitalWrite(output4, LOW);
              }
              
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS to style the on/off buttons 
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #555555;}</style></head>");
              
              // Web Page Heading
              client.println("<body><h1>ESP32 Web Server</h1>");
              
              // Display current state, and ON/OFF buttons for GPIO 15  
              client.println("<p>GPIO 15 - State " + output15State + "</p>");
              // If the output15State is off, it displays the ON button       
              if (output15State=="off") {
                client.println("<p><a href=\"/15/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/15/off\"><button class=\"button button2\">OFF</button></a></p>");
              } 
                 
              // Display current state, and ON/OFF buttons for GPIO 4  
              client.println("<p>GPIO 4 - State " + output4State + "</p>");
              // If the output4State is off, it displays the ON button       
              if (output4State=="off") {
                client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
              client.println("</body></html>");
              
              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            }
            else{
              client.println("HTTP/1.1 401 Unauthorized");
              client.println("WWW-Authenticate: Basic realm=\"Secure\"");
              client.println("Content-Type: text/html");
              client.println();
              client.println("<html>Authentication failed</html>");
            }
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
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
}
