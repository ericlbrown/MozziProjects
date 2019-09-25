#include <MozziGuts.h>   // at the top of your sketch
#include <Oscil.h> // oscillator template
#include <PinChangeInt.h> //for interrupt  routine with mozzi needs code from here: http://code.google.com/p/arduino-pinchangeint/
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <EventDelay.h>
uint8_t inputPins[] = {2, 3, 4, 5, 6, 7, 8, 10}; //pins that we are using
float Pitches[] = {1046.5, 1174.66, 1318.51, 1396.91, 1567.98, 1760, 1975.53, 2093}; //standard pitches for an octave starting at C5 --> C6
const byte intensity = 255;
uint8_t volumes[sizeof(inputPins)] = {0, 0, 0, 0, 0, 0, 0, 0};
byte WaveTableIU = 1; //keeps track of which waveform we are currently using for our synth
#define CONTROL_RATE 128
//here we will setup all of our oscillators for each of the individual notes
Oscil <2048, AUDIO_RATE> key1(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key2(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key3(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key4(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key5(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key6(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key7(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key8(SIN2048_DATA);

//oscialltor for overall vibrato effect
Oscil <2048, AUDIO_RATE> kVib(SIN2048_DATA);


EventDelay kSwapTablesDelay; //event delay for reading the oscillator switch
void setup() {
  Serial.begin(115200);
  startMozzi(CONTROL_RATE); //instiation for the Mozzi library with the predetermined control rate --> can be changed for more responsive feedback
  kSwapTablesDelay.set(500); //sample the SwapButton at 2Hz
  /*
    for (int i = 0; i << sizeof(inputPins); i++) { //declaring all of our input pins for the keys
     pinMode(inputPins[i], INPUT_PULLUP);
    }
  */
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

  //set all baseline frequencies
  key1.setFreq(Pitches[0]);
  key2.setFreq(Pitches[1]);
  key3.setFreq(Pitches[2]);
  key4.setFreq(Pitches[3]);
  key5.setFreq(Pitches[4]);
  key6.setFreq(Pitches[5]);
  key7.setFreq(Pitches[6]);
  key8.setFreq(Pitches[7]);
  kVib.setFreq(15.f);
}

void updateControl() {
  // your control code

  // kVib.setFreq(int(map(mozziAnalogRead(A0), 0, 1023, 0, 50)));
  volumes[0] = !digitalRead(2);
  volumes[1] = !digitalRead(3);
  volumes[2] = !digitalRead(4);
  volumes[3] = !digitalRead(5);
  volumes[4] = !digitalRead(6);
  volumes[5] = !digitalRead(7);
  volumes[6] = !digitalRead(8);
  volumes[7] = !digitalRead(10);
  if (kSwapTablesDelay.ready()) {
    if (!digitalRead(12)) {
      changeWaveTable();
      kSwapTablesDelay.start();
    }
  }
}
void changeWaveTable()  //function called on pin interrupt
{
  Serial.println("press");
  // key1.setTable(SAW2048_DATA);
  Serial.println("Inside Switch");
  if (WaveTableIU == 1) {
    key1.setTable(SIN2048_DATA);
    key2.setTable(SIN2048_DATA);
    key3.setTable(SIN2048_DATA);
    key4.setTable(SIN2048_DATA);
    key5.setTable(SIN2048_DATA);
    key6.setTable(SIN2048_DATA);
    key7.setTable(SIN2048_DATA);
    key8.setTable(SIN2048_DATA);
    Serial.println("Sine");
    WaveTableIU++;
  }
  else if (WaveTableIU == 2) {
    key1.setTable(SAW2048_DATA);
    key2.setTable(SAW2048_DATA);
    key3.setTable(SAW2048_DATA);
    key4.setTable(SAW2048_DATA);
    key5.setTable(SAW2048_DATA);
    key6.setTable(SAW2048_DATA);
    key7.setTable(SAW2048_DATA);
    key8.setTable(SAW2048_DATA);
    Serial.println("Saw");
    WaveTableIU++;
  }
  else if (WaveTableIU == 3) {
    key1.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key2.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key3.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key4.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key5.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key6.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key7.setTable(SQUARE_NO_ALIAS_2048_DATA);
    key8.setTable(SQUARE_NO_ALIAS_2048_DATA);
    Serial.println("Square");
    WaveTableIU++;
  }
  else if (WaveTableIU == 4) {
    key1.setTable(TRIANGLE2048_DATA);
    key2.setTable(TRIANGLE2048_DATA);
    key3.setTable(TRIANGLE2048_DATA);
    key4.setTable(TRIANGLE2048_DATA);
    key5.setTable(TRIANGLE2048_DATA);
    key6.setTable(TRIANGLE2048_DATA);
    key7.setTable(TRIANGLE2048_DATA);
    key8.setTable(TRIANGLE2048_DATA);
    Serial.println("Triangle");
    WaveTableIU = 1; //bring it back to sine
  }
}

int updateAudio() {
  // your audio code which returns an int between -244 and 243
  Q15n16 asig = (int) //creating an int specifically to be returned for audioHook()

                (key1.next() * volumes[0] +
                 key2.next() * volumes[1] +
                 key3.next() * volumes[2] +
                 key4.next() * volumes[3] +
                 key5.next() * volumes[4] +
                 key6.next() * volumes[5] +
                 key7.next() * volumes[6] +
                 key8.next() * volumes[7]) ; // method of assigning the value to asig with forced type conversion to int
  // Q15n16 vibrato = (Q15n16) intensity * kVib.next();
  //`asig.phMod(vibrato)
  return asig >> 3;
}

void loop() {
  audioHook();
}
