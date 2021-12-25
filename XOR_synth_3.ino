// Arduino 8-bit bitwise modulation synthesizer
// By Ozfir Izmgzoz, 2021, late@sapatti.fi
// See License for the MIT License
//
// Happy hacking!
 
// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

// Waveforms set up at setup
#define WAVE_TYPE_SINE 1
#define WAVE_TYPE_SQUARE 2
#define WAVE_TYPE_TRIANGLE 3
#define WAVE_TYPE_SAW_UP 4
#define WAVE_TYPE_SAW_DOWN 5
#define WAVE_TYPE_RANDOM 6

// Functions
//
void setup();
void loop();
int setFrequency(float frequency);
int setSubOsc(float semitones, int octave);
void setTimer();
void generateWaveTable(int style);

// Variables inside ISR declared as volatile
// Oscillator:
volatile unsigned long phaseAccumulator;   // phase accumulator
volatile unsigned long tuningWordM;        // dds tuning word m
volatile byte phaseAccumulatorMSBs;         // last 8 bits of phaccu
volatile int signal1;                      // what eventually is xorred

// Sub oscillator
volatile unsigned long subAccumulator; // sub oscillator counter
volatile unsigned long subTuningWord;
volatile byte subAccumulatorMSBs;
volatile int signal2;

// Select the start of waveform in the eeprom
// for OSC 1
volatile int oscWaveSel = 300;

volatile int playNote = 1; // DEBUG - start with note playing ("middle c" 48)

// Select waveform of sub OSC
volatile int subOscWaveSel = 64;

// Set sub osc phase offset 0 - 360\u00b0
volatile int subPhase = 0;

// Bitwise AND, OR, XOR selector
volatile int bitwiseOperator = 0;

const int timerCount = 512; // 256 Timer compare register overflow

const int prescaler = 1; // Update this if you change this in timer setup

// CPU_frequency/upperCountingValue/prescaler
const double refFrequency = 16000000/(timerCount+1)/prescaler; 
// const double refFrequency = 62310; // questimated

// R2R pins
int dacPins[] = {6,7,8,9,10,11,12,13}; // LSB -> MSB

int dacBits = 8;  // DAC bits
int dacMaxVal = 255; // Max DAC value 63

const int stepsPerCycle = 256; // phase resolution

// Other oscillator called "sub" because it was
// initially a sub oscillator. Change it or leave it

// Analog pins
int subPin = A0;  // sub oscillator tune
int freqPin = A1; // "main" frequency
int oscWavePin = A2; // oscillator waveform
int subWavePin = A5; // sub osc waveform 
int subPhasePin = A4; // phase between two osc
int opSelPin = A3; // AND/OR/XOR selector

// Digital pins -- deprecated
// just select bitwise operator with a pot
// int bitwisePin = 2;
// int led1Pin = 3;
// int led2Pin = 4;
// int bitwiseLast = 0;

// MIDI in variables
  int note =48;
  int velocity = 127;
  int notePlaying= 48;
  int noteStop = 0; // note # of the note to be stopped

double notes[] = {
// 55hz = A #33 - A #81 880hz
55.000,
58.270,
61.735,
65.406,
69.296,
73.416,
77.782,
82.407,
87.307,
92.499,
97.999,
103.826,
110.000,
116.541,
123.471,
130.813,
138.591,
146.832,
155.563,
164.814,
174.614,
184.997,
195.998,
207.652,
220.000,
233.082,
246.942,
261.626,
277.183,
293.665,
311.127,
329.628,
349.228,
369.994,
391.995,
415.305,
440.000,
466.164,
493.883,
523.251,
554.365,
587.330,
622.254,
659.255,
698.456,
739.989,
783.991,
830.609,
880.000
};

/*
 * deprecated, would allow you to set osc frequency to semitones
 *
 * deprecated:
 *
float subOscFreqs[] = {
 1,   // The same pitch
 1,
 1,
 1.059463,    // -1
 1.122462,       // -2
 1.189207,    // -3
 1.259921,    // -4
 1.334839,    // -5 e
 1.414213,    // -6
 1.498307,    // -7
 1.587401,     // -8
 1.681793,     // -9
 1.781797,     // -10
 1.887749,     // -11
 2,             // -12
 2,
};
*/

/*
 * SETUP
 *
 * Set up some crazy shit here
 */
void setup()
{
 
  // Serial.begin(9600); // DEBUG
  Serial.begin(31250); // MIDI baudrate
  
  /*
  // deprecated
  pinMode(bitwisePin, INPUT);  // Select bitwise mode
  pinMode(led1Pin, OUTPUT);  // MIDI LED
  pinMode(led2Pin, OUTPUT);  // MIDI LED
  */
  
  // initialize the digital pins as output.
  for (int i = 0; i < dacBits; i++) {
    pinMode(dacPins[i], OUTPUT);
  }

  
  // generate the wave table
  // write to EEPROM 0...1023
  // generateWaveTable(WAVE_TYPE_SINE);  
  // generateWaveTable(WAVE_TYPE_SQUARE);
  // generateWaveTable(WAVE_TYPE_TRIANGLE);
  // generateWaveTable(WAVE_TYPE_SAW_UP);  
  // generateWaveTable(WAVE_TYPE_SAW_DOWN); 
  // generateWaveTable(WAVE_TYPE_RANDOM);

  // DEBUG first, then set timer etc.
  
  /*
  Serial.println("Start here FOOL!");
  for (int i = 0; i < 1024; i++) {
     Serial.println(EEPROM.read(i)); 
  }
  */
  
  
  // Set the timer
  setTimer();

  setFrequency(440);
  
  setSubOsc(1, 1);

}
 
 
/**
 *  MAIN PROGRAM
 *
 *
 */
void loop()
{
  // Da main loop

  // Read sensors
  int subVal = analogRead(subPin);
  float freqVal = analogRead(freqPin); // something funny happens here :)
  int oscWaveVal = analogRead(oscWavePin);
  int subWaveVal = analogRead(subWavePin);
  int subPhaseVal = analogRead(subPhasePin);
  int opSelVal = analogRead(opSelPin);
  
  // Set subOSC tuning
  if (subVal < 451) {
    float semitones = (float) subVal / 45;
    setSubOsc(semitones, 0);
  } else if (subVal < 550) {
    setSubOsc(1, 1);
  } else if (subVal < 973 ){
    subVal = map(subVal, 550, 973, 999, 250);
    float semitones = (float) (subVal) /1000;
    setSubOsc(semitones, 1);
  } else {
    float semitones = (float) 250 /1000;
    setSubOsc(semitones, 1);
  }

  // float semitones = (float) (subVal) /1000;
  // setSubOsc(semitones, 1); // forget subOctVal
  

  //
  // select waveform for main oscillator
  if (oscWaveVal < 256) {
     // Set sine
     oscWaveSel = 0;
  } else if (oscWaveVal < 512) {
    // Set triangle
    oscWaveSel = 256;     
  } else if (oscWaveVal < 768) {
    // Set saw
    oscWaveSel = 512;      
  } else if (oscWaveVal < 896) {
    // Set square wave 100% cycle
    oscWaveSel = 768; 
  } else {
    // Make this and other map functions more efficient when you have time
    oscWaveSel = map(oscWaveVal, 896, 1023, 768, 1024);
  }
  
  //
  // Set subOSC waveform
  if (subWaveVal < 256) {
     // Set sine
    subOscWaveSel = 0;
  } else if (subWaveVal < 512) {
    // Set triangle
    subOscWaveSel = 256;     
  } else if (subWaveVal < 768) {
    // Set saw
    subOscWaveSel = 512;      
  } else if (subWaveVal < 896) {
    // Set square wave 100% cycle
    subOscWaveSel = 768; 
  } else {
    // Make this and other map functions more efficient when you have time
    subOscWaveSel = map(subWaveVal, 896, 1023, 768, 1024);
  }  

  // Set sub OSC phase 0 ... 360\u00b0, or in fact 0 - 255 offset
  // essentially this is just the phase difference between the two waves,
  // though technically we are adjusting the phase of the sub osc
  subPhase = map(subPhaseVal, 0, 1023, 0, 255);

  // Set bitwiseOperator AND or OR or XOR
  // bitwiseOperator
  if (opSelVal < 341) {
    // AND
    bitwiseOperator = 0;
  } else if (opSelVal < 682) {
    // OR
    bitwiseOperator = 1;     
  } else {
    // XOR
    bitwiseOperator = 2;
  }
  
  // Set bitwise LEDs deprecated
  /*
  switch (bitwiseOperator) {
    case 0:
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, LOW);
    break;
    case 1:
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, HIGH);
    break;
    case 2:
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, HIGH);
    break;
    
  }
  */
  
  
  // See if play button is pressed // obsolete
  // playNote = (digitalRead(playPin)?1:0);
  
  // Blink midiLed
  // digitalWrite(midiPin, digitalRead(midiPin)?0:1);
  
  // MIDI in
  if (Serial.available() > 0) {
  
  // read the incoming byte:
  int incomingByte = Serial.read();
  
  // wait for as status-byte, channel 1, note on or off
  if (incomingByte == 144){ // note on message starting
    while (Serial.available() == 0) {
    }
    note = Serial.read();
    while (Serial.available() == 0) {
    }
    velocity = Serial.read();
    setFrequency(notes[note-33]);
    playNote = 1;
    notePlaying = note;
    // digitalWrite(midiPin, HIGH);
  }else if (incomingByte == 128){ // note off message starting
    while (Serial.available() == 0) {
    }
    noteStop = Serial.read();
    if (notePlaying == noteStop) { // only stop a note if it's the note that's actually playing
      playNote = 0;
      // digitalWrite(midiPin, LOW);
    }  

/*
  // For reading CC messages do something like this:
  }else if (incomingByte == 176){ // cc message ch 1
  while (Serial.available() == 0) {
    }
    ccF = Serial.read(); // accept any cc message
    while (Serial.available() == 0) {
    }
    // Read CC value and adjust subOsc freq
    ccV = Serial.read(); // 0..127
    // Now you can do something with the CC message...
*/

  }else{
    //nada
    // some other message
  }

  }
  
  setFrequency(notes[note-33]  * ((freqVal + 1023)/2046));  

}
 
 
/**
 *  ISR - INTERRUPT SERVICE ROUTINE - ISR
 *
 * Don't mess about too much here
 */
ISR(TIMER1_COMPA_vect)
{

    // Update phase accumulator, I'm told it will automagically overflow at 2^32
    phaseAccumulator += tuningWordM;
              
    subAccumulator += subTuningWord;    
  
    if (playNote) {
      // PLAY NOTE    
      
      // Set R2R DAC using lookup table (2^8 bits in length)
      // waveSel selects the right waveform from the table

      // Use the 8 MSBs from phase accumulator as lookup index for the wavetable
      phaseAccumulatorMSBs = (phaseAccumulator >> 24);

      // Use 8 MSB for sub osc
      // Also add sub osc phase
      subAccumulatorMSBs = (subAccumulator >> 24) + subPhase;


      //
      // Check if we want to do a square wave or read wave from table
      //
      if (oscWaveSel < 768) {
        // Read wave from table
        signal1 = EEPROM.read(phaseAccumulatorMSBs + oscWaveSel);        
      } else {
        // do a variable width square wave
        if (phaseAccumulatorMSBs > oscWaveSel - 769) {
            signal1 = dacMaxVal;      
        } else {
          signal1 = 0;   
        } 
      } 

      if (subOscWaveSel < 768) {
        // Read wave from table
        signal2 = EEPROM.read(subAccumulatorMSBs + subOscWaveSel);        
      } else {
        // do a variable width square wave
        if (subAccumulatorMSBs > subOscWaveSel - 769) {
            signal2 = dacMaxVal;      
        } else {
          signal2 = 0;   
        } 
      }      


        // read osc 1 from eeprom, AND or OR or XOR with sub osc wave, to which phase is added
        switch (bitwiseOperator) {
          case 0:
            // AND
            PORTB = signal1 & signal2;
            PORTD = (signal1 & signal2) & B11000000;
            break;
          case 1:
            // OR
            PORTB = signal1 | signal2;
            PORTD = (signal1 |signal2) & B11000000;
            break;
          case 2:
            // XOR
            PORTB = signal1 ^ signal2;
            PORTD = (signal1 ^ signal2) & B11000000;
            break;
        }

    
    } else {
      // DON*T PLAY NOTE
      PORTB = 0;
      PORTD &= B00111111;
    }
        
}


/***
 * setFrequency(herz)
 *
 */
int setFrequency(float frequency){
  // Calculate tuning word value
  tuningWordM = (unsigned long) ((pow(2, 32) * frequency) / refFrequency);
}


/***
 * setSubOsc(semitones, octave)
 *
 * freq of two 12tet notes X semitones apart = 2^(1/12)^X
 *
 * tuningWordM is simply divided by the octave. Clever.
 * Note that
 */
int setSubOsc(float frequency, int octave){
  // Calculate tuning word value
  if (octave == 1) {
    subTuningWord = (unsigned long) tuningWordM / frequency;
  } else {
    subTuningWord = (unsigned long) ((pow(2, 32) * frequency) / refFrequency);
  }  
}


/*
 *  Set up the timer
 *
 */
void setTimer() {
    // initialize Timer1
  cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
  
  // Reset timer/counters 0 and 2 in order to reduce noise on timer/counter1
  TIMSK0 = 0;
  TIMSK2 = 0;  
 
  // set compare match register to desired timer count:
  OCR1A = timerCount;
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS11 
  // CS10 = 1x, CS11 = 8x, CS10 & CS11 = 64x
  TCCR1B |= (1 << CS10);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  // enable global interrupts:
  sei(); 
  
}


/*********************************
 * void generateWaveTable(int style)
 * generate square, tri, sine, saw or
 * rand waves and write in eeprom
 *
 * Usually only run this at setup
 *
 * Not portable or pretty code
 */
void generateWaveTable(int style){
  
  // weird metallic hollow trashed sound
  // genWave.pl harmonics 3,5,7,11,13
  int oddTable[] = {
0, 62, 0, 58, 0, 38, 0, 50, 0, 56, 0, 21, 1, 2, 1, 29, 
2, 41, 3, 0, 3, 57, 4, 15, 5, 23, 6, 29, 7, 32, 8, 59, 
9, 7, 10, 61, 11, 4, 12, 0, 13, 0, 15, 45, 16, 60, 18, 55, 
19, 3, 20, 6, 22, 26, 23, 23, 25, 15, 26, 7, 28, 8, 29, 21, 
31, 33, 33, 47, 34, 62, 36, 56, 37, 50, 39, 61, 40, 20, 42, 0, 
43, 61, 44, 27, 46, 13, 47, 58, 49, 61, 50, 0, 51, 62, 52, 17, 
53, 52, 54, 23, 55, 14, 56, 27, 57, 35, 58, 59, 59, 18, 59, 52, 
60, 17, 61, 50, 61, 62, 62, 1, 62, 4, 62, 10, 62, 9, 62, 61, 
62, 0, 62, 4, 62, 24, 62, 12, 62, 6, 62, 41, 61, 60, 61, 33, 
60, 21, 59, 62, 59, 5, 58, 47, 57, 39, 56, 33, 55, 30, 54, 3, 
53, 55, 52, 1, 51, 58, 50, 62, 48, 62, 47, 17, 46, 2, 44, 7, 
43, 59, 42, 56, 40, 36, 39, 39, 37, 47, 36, 55, 34, 54, 33, 41, 
31, 29, 29, 15, 28, 0, 26, 6, 25, 12, 23, 1, 22, 42, 20, 62, 
19, 1, 18, 35, 16, 49, 15, 4, 13, 1, 12, 62, 11, 0, 10, 45, 
9, 10, 8, 39, 7, 48, 6, 35, 5, 27, 4, 3, 3, 44, 3, 10, 
2, 45, 1, 12, 1, 0, 0, 61, 0, 58, 0, 52, 0, 53, 0, 1
  };  

  // A genuine sine wave
  byte sineTable[] = {
0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 
9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 26, 28, 30, 33, 35, 
37, 39, 41, 44, 46, 49, 51, 54, 56, 59, 61, 64, 67, 70, 72, 75, 
78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 115, 118, 121, 124, 
127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 
176, 179, 182, 184, 187, 190, 193, 195, 198, 200, 203, 205, 208, 210, 213, 215, 
217, 219, 221, 224, 226, 228, 229, 231, 233, 235, 236, 238, 239, 241, 242, 244, 
245, 246, 247, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 254, 254, 
255, 254, 254, 254, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 246, 
245, 244, 242, 241, 239, 238, 236, 235, 233, 231, 229, 228, 226, 224, 221, 219, 
217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 193, 190, 187, 184, 182, 179, 
176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130, 
127, 124, 121, 118, 115, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 
78, 75, 72, 70, 67, 64, 61, 59, 56, 54, 51, 49, 46, 44, 41, 39, 
37, 35, 33, 30, 28, 26, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10, 
9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0
  };  

  // Sine wave at half amplitude
  int sineHalfTable[] = {
123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 
123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 
124, 124, 124, 124, 124, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 
125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 
127, 127, 127, 127, 127, 127, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 
129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 130, 130, 130, 130, 
130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 131, 131, 131, 131, 
131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 
131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 
131, 131, 131, 131, 131, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 
130, 130, 130, 130, 130, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 
129, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 127, 127, 127, 127, 127, 
127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 
125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 124, 124, 124, 124, 
124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 123, 123, 123, 123, 
123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123
  };
  
 switch(style){  
  case WAVE_TYPE_SINE: // Sine
  for (int i = 0; i < stepsPerCycle; i++) {
    
    EEPROM.write(i, sineTable[i]);
    
    // EEPROM.write(i, ((dacMaxVal*(sin((i*6.2831)/stepsPerCycle)+1))/2) );
    
    // Serial.println(waveTable[i]); // DEBUG
  }
  break;
  
  case WAVE_TYPE_SQUARE: // Square
    for (int i=0; i < stepsPerCycle/2; i++) {
      EEPROM.write(i + 256, 0);
          // Serial.println(waveTable[i]); // DEBUG
    }
    for (int i=stepsPerCycle/2; i < stepsPerCycle; i++) {
      EEPROM.write(i + 256, dacMaxVal);
          // Serial.println(waveTable[i]); // DEBUG
    }
  break;  

  case WAVE_TYPE_TRIANGLE: // Triangle
    for (int i=0; i < stepsPerCycle/2; i++) {
      EEPROM.write(i + 256, (i*dacMaxVal*2)/stepsPerCycle );
          // Serial.println(waveTable[i]); // DEBUG      
    }  
    for (int i=stepsPerCycle/2; i < stepsPerCycle; i++) {
      EEPROM.write(i + 256, ((stepsPerCycle-i)*dacMaxVal*2)/stepsPerCycle );
          // Serial.println(waveTable[i]); // DEBUG      
    }
  break;

  case WAVE_TYPE_SAW_UP: // Saw tooth positive slope
    for (int i=0; i < stepsPerCycle; i++) {
      EEPROM.write(i + 512, (i*dacMaxVal)/stepsPerCycle );
          // Serial.println(waveTable[i]); // DEBUG 
    }
  break;

  case WAVE_TYPE_SAW_DOWN: // Saw tooth negative slope
    for (int i=0; i < stepsPerCycle; i++) {
      EEPROM.write(i + 512, ((stepsPerCycle-i)*dacMaxVal)/stepsPerCycle );
          // Serial.println(waveTable[i]); // DEBUG 
    }
  break;
  
  case WAVE_TYPE_RANDOM: // Random (Noise)
    // randomSeed(millis());
    for (int i=0; i < stepsPerCycle; i++) {
      
      EEPROM.write(i + 768, sineHalfTable[i]);
      
      // EEPROM.write(i + 768, random(0, dacMaxVal)) ;
          // Serial.println(waveTable[i]); // DEBUG 
    }
  break;

 }

}


