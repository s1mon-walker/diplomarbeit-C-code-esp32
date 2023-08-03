#include "main.h"
// todo: restart subscribtion
// licht aus modus
PPlace PP = PPlace();

void setup() {
  Serial.begin(115200);

  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  connection_setup();

  PP = PPlace(LOCATION, R_LED, G_LED, B_LED);

  add_subscription([](String dist) {PP.set_teach_distance(dist);}, MQTT_TEACH_SUBSCRIBTION);
  add_subscription([](String value){PP.set_debug_mode(value);}, MQTT_DEBUG_SUBSCRIBTION);
}

void loop() {
  connection_handler();
  PP.loop();
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
