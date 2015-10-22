
// Uno pin 5V  <->   Red wire (right)
// Uno pin GND <-> Black wire
// Uno pin 4   <-> Green wire to read data
// Uno pin 2   <->  Blue wire to reset rfid reader (left)

// ID-12/20 ascii data structure
// +---------+-----------------+--------------------+---------+---------+---------+
// | STX (2) | DATA (10 ascii) | CHECKSUM (2 ascii) | CR (13) | LF (10) | ETX (3) |
// +---------+-----------------+--------------------+---------+---------+---------+

// arduino pin used to reset the ID-12/20 module
#define RESET_PIN 2

#include <SoftwareSerial.h>

int count = 0;

SoftwareSerial mySerial(4, 5);

// RFID tags
int one[13] = {49, 68, 48, 48, 55, 49, 70, 68, 48, 55, 57, 54};
int two[13] = {49, 68, 48, 48, 55, 49, 55, 56, 55, 51, 54, 55};

struct tags {
  String name;
  int *tag;
  // unsigned long to match millis computations without error
  unsigned long elapsed;
} tags[2];

// readed tag, initial value is {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
int tag[13];

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.setTimeout(10);
  delay(50);
  Serial.println("setup");

  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);

  // tags
  tags[0] = {"tag one", one};
  tags[1] = {"tag two", two};
}

void loop() {
  delay(50);
  int index = 0;
  boolean reading = false;
  
  if (mySerial.available()) {
    delay(30);
    while (mySerial.available() > 0) {
      int data = mySerial.read();
      if (data != 255) {
        // beginning of tag
        if (data == 2) reading = true;
        // end of tag
        if (data == 3) reading = false;
        // store the tag
        if (reading && data != 2 && data != 10 && data != 13) {
          tag[index] = data;
          index++;
        }
      }
    }
    if (index != 0) {
      check();
      reset();
    }
  }
}

// log the `tag` array
void output() {
  Serial.print("{");
  for (int i = 0; i < 11; i++) {
    Serial.print(tag[i]);
    Serial.print(", ");
  }
  Serial.print(tag[11]);
  Serial.println("}");
}

// check if `tag` match with all defined tags
void check() {
  for (int i = 0; i < 2; i++) {
    if (match(tags[i].tag)) {
      // millis() is unsigned long, not int
      // if int is used, errors can be done with diff computation
      unsigned long now = millis();
      if (tags[i].elapsed == 0
      // wait before fire the same tag
      || (now - tags[i].elapsed) > 3000) {
        tags[i].elapsed = now;
        Serial.println(tags[i].name);
      }
      return;
    }
  }
  Serial.print("unknown tag: ");
  output();
}

// check if `tag` match with `ref`
boolean match(int ref[]) {
  for (int i = 0; i < 12; i++) {
    if(tag[i] != ref[i]) return false;
  }
  return true;
}

// reset the `tag` array and the RFID reader module
void reset() {
  // reset the tag array by filling with NUL (ascii 0)
  for (int i = 0; i < 12; i++) {
    tag[i] = 0;
  }
  
  // reset the RFID reader module to read again
  // the pin 2 of the ID-12/20 module must be turn off (0 volt)
  digitalWrite(RESET_PIN, LOW);
  // no delay it is ok, 10 ms is safer ?
  delay(10);
  digitalWrite(RESET_PIN, HIGH);
  // 150 ms min required, 200 ms is safer
  delay(200);
}
