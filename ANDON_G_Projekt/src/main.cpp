#include "main.h"

// Declare software-objects for the physical location
AndonLine andon_line_G = AndonLine();

// Declare the GPIO Pins of the ESP32
uint8_t error_pin = 13;
uint8_t input_pins[] = {23,14,27,26,25,33,32,34};
uint8_t output_pins[] = {19,18,5,17,16,4,0,15};

// Arrays for Inverting Pin ploarity
bool invert_inputs[] = {1,1,1,1,1,1,1,1};
bool invert_outputs[] = {0,0,0,0,0,0,0,0};

// Declare Hardware Abstraction Layser class
HAL hal;

// Declare Webserver for Over-the-Air updates
AsyncWebServer server(80);

void setup_ota_server() {
  // Setup simple webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Update-Server ANDON Linie G -> /update");
  });

  // Setup ElegantOTA page on server at /update
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  // Setup Serial Monitor with 11500 baudrate
  Serial.begin(115200);
  
  // Setup the HAL with de declared pins
  hal.setup(input_pins, output_pins, error_pin);
  hal.set_invert_inputs(invert_inputs);
  hal.set_invert_outputs(invert_outputs);

  // Setup the WiFi and MQTT connection
  setup_wifi();

  // Setup the OTA server for software-spdates
  setup_ota_server();

  // Inizialise software objects
  // The ANDON Line object gets pointers to the HAL variables for direct access
  andon_line_G = AndonLine(LOCATION, &hal.inputs[0], &hal.outputs[0], &hal.outputs[1], &hal.outputs[3]);
}

// loop must contain the .loop() method for every object
void loop() {
  hal.input();
  andon_line_G.loop();
  hal.output();
  check_connection(RECONNECT_CHECK_TIME);
}

// Quellen:
// https://github.com/marvinroger/async-mqtt-client
// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
// https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/
// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
// https://randomnerdtutorials.com/solved-reconnect-esp32-to-wifi/
// https://stackoverflow.com/questions/2298242/callback-functions-in-c
// https://stackoverflow.com/questions/3167272/how-does-c-stdvector-work/3167372
// https://stackoverflow.com/questions/7663709/how-can-i-convert-a-stdstring-to-int
// https://stackoverflow.com/questions/17660095/iterating-over-a-struct-in-c
// https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
// https://stackoverflow.com/questions/50383732/mqtt-msg-payload-funny-characters
// https://stackoverflow.com/questions/12702561/iterate-through-a-c-vector-using-a-for-loop/12702744
// https://www.includehelp.com/stl/vector-empty-function-with-example.aspx
// https://www.csee.umbc.edu/courses/202/fall04/Projects/CommonErrors.shtml#:~:text=collect2%3A%20ld%20returned%201%20exit,error%2C%20not%20a%20compiler%20error.&text=This%20phrase%20is%20generally%20found,member%20in%20a%20const%20object.
// https://www.tutorialspoint.com/bool-to-int-conversion-in-cplusplus
// https://stackoverflow.com/questions/15092504/how-to-time-a-function-in-milliseconds-without-boosttimer
// https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=msvc-160
// https://stackoverflow.com/questions/13225208/matching-elements-in-a-vector
// https://stackoverflow.com/questions/1127396/struct-constructor-in-c
// https://www.geeksforgeeks.org/vector-in-cpp-stl/
// https://www.tutorialspoint.com/cplusplus/cpp_passing_arrays_to_functions.htm
// https://stackoverflow.com/questions/1789807/function-pointer-as-an-argument
// https://www.w3schools.com/cpp/cpp_classes.asp
// https://www.w3schools.com/cpp/cpp_constructors.asp
// https://www.w3schools.com/cpp/cpp_access_specifiers.asp
// https://www.w3schools.com/cpp/cpp_encapsulation.asp
// https://www.w3schools.com/cpp/cpp_switch.asp
// https://stackoverflow.com/questions/48434575/switch-debouncing-logic-in-c
