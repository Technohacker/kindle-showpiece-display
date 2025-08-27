#define ARDUINO_HTTP_SERVER_NO_BASIC_AUTH

#include <ArduinoHttpServer.h>
#include <WiFiNINA.h>
#include <pico/multicore.h>

#include "arduino_secrets.h"

// Framebuffer
const uint16_t WIDTH = 600;
const uint16_t HEIGHT = 700;

// This header uses the above defined constants
#include "framebuffer.h"
framebuffer_t framebuffer;

// WIFI
const char ssid[] PROGMEM = SECRET_SSID;
const char pass[] PROGMEM = SECRET_PASS;

WiFiServer server(80);

// Rudimentary flag for core-sync
volatile bool hold_core1 = false;

void setup() {
  status_blink_setup();

  // Initialize serial
  Serial.begin(9600);

  // Check for WiFi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));

    while (true) {
      fatal_error_blink();
    }
  }

  // Make sure the WiFi module's firmware is up to date
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print(F("Please upgrade your WiFiNINA firmware. Current version: "));
    Serial.print(fv);
    Serial.print(F(". Available: "));
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
  }

  // Attempt to connect to the network
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);

    // Wait a little for the connection
    delay(1000);
  }

  // Start the HTTP Server
  server.begin();
  if (!server) {
    Serial.println(F("Creating server socket failed: error"));

    while (true) {
      fatal_error_blink();
    }
  }

  // Send details to the Serial console
  print_wifi_details();

  // Clear the framebuffer to white for aesthetics
  memset(framebuffer, 0b11111111, sizeof(framebuffer));

  // And launch the compute core
  multicore_launch_core1(core1_entry);
}

void loop() {
  waiting_blink();

  // Wait for any client
  WiFiClient client = server.available();

  if (!client) {
    return;
  }

  // Read the HTTP Request
  ArduinoHttpServer::StreamHttpRequest<512> http_request(client);

  bool request_valid = http_request.readRequest();
  if (!request_valid) {
    client.stop();
    return;
  }

  active_blink();

  // FIXME: Can't use this library for responses unless it can allow sending multiple buffers at a time
  // ArduinoHttpServer::StreamHttpReply http_reply(client, "image/bmp");

  Serial.println(F("Got a request, sending response"));

  // Start with the HTTP meta
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: image/bmp"));
  client.println(F("Connection: close"));
  client.println();

  Serial.println(F("  HTTP Meta done"));

  // Then the BMP header
  client.write((const uint8_t *) &FINAL_BMP_HEADER, sizeof(FINAL_BMP_HEADER));

  Serial.println(F("  BMP Header done"));

  // And the framebuffer
  Serial.println(F("  Holding Compute core"));
  hold_core1 = true;
  Serial.println(F("  Compute core held"));

  for (size_t row = 0; row < HEIGHT; row += 1) {
    // Send the framebuffer line-by-line since WiFiNINA has a limit
    client.write((const uint8_t *) framebuffer[row], sizeof(framebuffer[row]));
  }

  Serial.println(F("  Framebuffer done"));

  Serial.println(F("  Resuming Compute core"));
  hold_core1 = false;
  Serial.println(F("  Compute core resumed"));

  client.stop();
  Serial.println(F("Client done"));
}

void print_wifi_details() {
  // Print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
