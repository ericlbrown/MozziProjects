#include <MozziGuts.h>   // at the top of your sketch
#include <Smooth.h>
#include <Oscil.h> // oscillator template
#include <IntMap.h>
//the wavetables that I chose to use for the project; there are more or you can create your own with utilities like MATLAB or an inculded .py script
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/triangle2048_int8.h>

//additional headers for features we make use of
#include <EventDelay.h>
#include <Line.h>

//now we can start declaring all the variables we need

///////////////////////////////////////////////////////////////////////////////////
//TODO: USER CHANGED VALUES BASED ON CIRCUIT SETUP
uint8_t inputPins[] = {15, 10, 6, 3, 7, 14, 2, 5}; //pins that we are using; you will likely need to change which ones you use

///////////////////////////////////////////////////////////////////////////////////////

float Pitches[] = {1046.5, 1174.66, 1318.51, 1396.91, 1567.98, 1760, 1975.53, 2093}; //standard pitches for an octave starting at C5 --> C6
byte controlLoop = 0; //use to create sqwitch case for controlLoop optimization
uint8_t volumes[sizeof(inputPins)] = {0, 0, 0, 0, 0, 0, 0, 0}; //array that we use to collect button states
byte WaveTableIU = 1; //keeps track of which waveform we are currently using for our synth

#define CONTROL_RATE 64 //the control rate is how fast the updateControl function gets called in Hz, 64 or 128 both work, but 128 becomes taxxing

//intmap is a class in Mozzi to create faster intput value mapping to a scale of our choice
const IntMap testIntMap(0, 1023, 0, 25);
const IntMap externalVolume(0, 1023, 0, 5);


//here we will setup all of our oscillators for each of the individual notes
Oscil <2048, AUDIO_RATE> key1(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key2(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key3(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key4(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key5(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key6(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key7(SIN2048_DATA);
Oscil <2048, AUDIO_RATE> key8(SIN2048_DATA);

// control oscillator for tremelo
Oscil<SIN2048_NUM_CELLS, CONTROL_RATE> kTremelo(SIN2048_DATA);

// the number of audio steps the line has to take to reach the next control value
const unsigned int AUDIO_STEPS_PER_CONTROL = AUDIO_RATE / CONTROL_RATE;
// a line to interpolate control tremolo at audio rate
Line <unsigned int> aGain;


int sumTerm = 1; //used to hopefully normalize sound



EventDelay kSwapTablesDelay; //event delay for reading the oscillator switch, fancy way of putting a debounce on the table switch without using any form of delay()

void setup() { //where we take care of all our starting functions and object instantiation
  startMozzi(CONTROL_RATE); //instiation for the Mozzi library with the predetermined control rate --> can be changed for more responsive feedback
  kSwapTablesDelay.set(500); //sets sampling of the WavetableSwapButton at 2Hz, make this way less if you want a funky effect while holding the button

  //code to adjust the octave
  for (int i = 0; i < 8; i++) {
    Pitches[i] *= 1; //change to 2^(a) where a is any whole number, for C-C octave, play around for other possible tunings/scales
  }
  
//setting all the button pins which we set in an array earlier as pullups to avoid using resistors
  pinMode(inputPins[0], INPUT_PULLUP);
  pinMode(inputPins[1], INPUT_PULLUP);
  pinMode(inputPins[2], INPUT_PULLUP);
  pinMode(inputPins[3], INPUT_PULLUP);
  pinMode(inputPins[4], INPUT_PULLUP);
  pinMode(inputPins[5], INPUT_PULLUP);
  pinMode(inputPins[6], INPUT_PULLUP);
  pinMode(inputPins[7], INPUT_PULLUP);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
//         NEED TO CHANGE THIS ONE!!!!!!!!!!!!!!!
  pinMode(8, INPUT_PULLUP); //wavetable pin, write this one on your own since it doesn't belong in the table

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //set all baseline frequencies from the table above
  key1.setFreq(Pitches[0]);
  key2.setFreq(Pitches[1]);
  key3.setFreq(Pitches[2]);
  key4.setFreq(Pitches[3]);
  key5.setFreq(Pitches[4]);
  key6.setFreq(Pitches[5]);
  key7.setFreq(Pitches[6]);
  key8.setFreq(Pitches[7]);
  kTremelo.setFreq(5.5f);
}

void updateControl() {
  // your control code
  sumTerm = 1;
  switch (controlLoop % 4) { //attempt to break up code to speed up the control loop so we only adjust certain params each call
    case 0:
      volumes[0] = !digitalRead(inputPins[0]);
      volumes[1] = !digitalRead(inputPins[1]);
      volumes[2] = !digitalRead(inputPins[2]);
      break;
    // kVib.setFreq(int(map(mozziAnalogRead(A0), 0, 1023, 0, 50)));
    case 1:
      volumes[3] = !digitalRead(inputPins[3]);
      volumes[4] = !digitalRead(inputPins[4]);
      volumes[5] = !digitalRead(inputPins[5]);
      break;
    case 2:
      volumes[6] = !digitalRead(inputPins[6]);
      volumes[7] = !digitalRead(inputPins[7]);
      break;
    case 3:
      kTremelo.setFreq(testIntMap(mozziAnalogRead(A0)));
      //Volume = externalVolume(mozziAnalogRead(A
      break;
  }
  if (kSwapTablesDelay.ready()) { //read the pushutton event
    //Serial.println();
    if (!digitalRead(8)) {
      changeWaveTable();
      kSwapTablesDelay.start();
    }
    unsigned int gain = (128u + kTremelo.next()) << 8;
    aGain.set(gain, AUDIO_RATE / CONTROL_RATE); // divide of literals should get optimised away
    controlLoop ++;
  }
  for (int i = 0; i < 8; i++) {
    sumTerm = sumTerm + volumes[i]; //this will be used to scale the output to prevent clipping
  }
  if ( sumTerm > 1) {
    sumTerm = sumTerm - 1; //averages out our value again
  }
}


void changeWaveTable()  //function called on pin interrupt to change the wave shape for each osccilator
{
  // Serial.println("press");
  // key1.setTable(SAW2048_DATA);
  // Serial.println("Inside Switch");
  if (WaveTableIU == 1) {
    key1.setTable(SIN2048_DATA);
    key2.setTable(SIN2048_DATA);
    key3.setTable(SIN2048_DATA);
    key4.setTable(SIN2048_DATA);
    key5.setTable(SIN2048_DATA);
    key6.setTable(SIN2048_DATA);
    key7.setTable(SIN2048_DATA);
    key8.setTable(SIN2048_DATA);
    //Serial.println("Sine");
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
    //Serial.println("Saw");
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
    //Serial.println("Square");
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
    //Serial.println("Triangle");
    WaveTableIU = 1; //bring it back to sine
  }
}

int updateAudio() {
  // your audio code which returns an int between -244 and 243



  Q15n16 asig =  //creating an int specifically to be returned for audioHook()

    (key1.next() * volumes[0] +
     key2.next() * volumes[1] +
     key3.next() * volumes[2] +
     key4.next() * volumes[3] +
     key5.next() * volumes[4] +
     key6.next() * volumes[5] +
     key7.next() * volumes[6] +
     key8.next() * volumes[7]) / sumTerm ; // method of assigning the value to asig with forced type conversion to int
  asig = int((asig * aGain.next() >> 16)); //*Volume);
  return asig;
}

void loop() {
  audioHook();
}
