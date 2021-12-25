# arduinoBitwiseSynth
Arduino 8-bit bitwise modulation synthesizer
## Synopsis
A synthesizer for the Arduino that can produce sine, triangle and saw waves and wacky waveforms produced by bitwise modulation between these. For an idea of what this synth can do, see [my blog post](http://kristitty.net/blog/arduino-8-bit-bitwise-modulation-synthesizer/) with pictures of the synth, a demo song made with sounds from the synth and a Youtube video showing the different waveforms on an oldschool oscilloscope.

## What you need
This is not a project for beginners, nor a plug and play build a synthesizer in five minutes tutorial. If you want to follow along, first google "digital direct synthesis" and "timer interrupts". That will give you an idea of what's happening inside the Arduino. Once you have the theory covered, you are in a better position to understand my code. 

Next you need to know how to do midi-input on the Arduino. There are a dozen tutorials online for that. The electronics and basic code to do that are not too difficult. In order to get audio out of the Arduino, you need a digital to analog converter, I used a R2R ladder for that. How you connect that to the Arduino inputs I can not help you with. If you know how to read analog signals from potentiometers connected to your Arduino, you are now good to go, as far as the electronics are concerned.

## Running the synth
Upload the code to the Arduino, connect your favorite midi device to the midi input, twist the knobs and watch amazing waveforms form on your oscilloscope. Via the pots you can choose the different base waveforms, adjust the offset between them, choose the bitwise modulation operator XOR, OR or AND, and detune the two waveforms. Be bold and experiment.

## Disclaimer
The code worked on my Arduino Duemilanove. I have not tested it on any other board. It may or may not work on later Arduinos. I am sorry to say I no longer have access to my Arduino, so if there are any problems with running the code, I can not do any debugging with the Arduino. I can read the code however and happily follow along. So if there is any confusion with the software, I may be able to help.

