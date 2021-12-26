# arduinoBitwiseSynth
Arduino 8-bit bitwise modulation synthesizer
## Synopsis
A synthesizer for the Arduino that can produce sine, triangle and saw waves and wacky waveforms produced by bitwise modulation between these. For an idea of what this synth can do, see [my blog post](http://kristitty.net/blog/arduino-8-bit-bitwise-modulation-synthesizer/) with pictures of the synth, a demo song made with sounds from the synth and a Youtube video showing the different waveforms on an oldschool oscilloscope.

## What you need
This is not a project for beginners, nor a plug and play build a synthesizer in five minutes tutorial. If you want to follow along, first google "digital direct synthesis" and "timer interrupts". That will give you an idea of what's happening inside the Arduino. Once you have the theory covered, you are in a better position to understand my code. 

Next you need to know how to do MIDI-input on the Arduino. There are a dozen tutorials online for that. The electronics and basic code to do that are not too difficult. In order to get audio out of the Arduino, you need a digital to analog converter, I used a R2R ladder for that. How you connect that to the Arduino inputs can be determined from the source code. If you know how to read analog signals from potentiometers connected to your Arduino, you are now good to go, as far as the electronics are concerned.

Six analog inputs are needed to control this synth. They are:
<ol>
  <li>Adjust main oscillator frequency</li>
  <li>Select main oscillator waveform</li>
  <li>Select sub oscillator waveform</li>
  <li>Adjust offset between the two oscillators</li>
  <li>Detuning of the sub oscillator</li>
  <li>Select the bitwise operator</li>
</ol>

## Running the synth
Upload the code to the Arduino, connect your favorite midi device to the MIDI input, twist the knobs and watch amazing waveforms form on your oscilloscope. Via the pots you can choose the different base waveforms, adjust the offset between them, choose the bitwise modulation operator XOR, OR or AND, and detune the two waveforms. Be bold and experiment.

## TODO
If you have enough digital inputs and outputs on your Arduino, you can easily add button to control some features or add blinking LED lights to indicate incoming MIDI messages for example. Some snippets of code are commented out giving you alternative features, not an elegant solution I admit. One such feature is the snapping of the sub oscillator frequency to semitones. Some may prefer that, but as it is, you can now detune the sub oscillator for an awesome warping sound or tune it to a minor chord by ear or hit a perfect fifth or an octave.

Instead of controlling the synth via the analog knobs, you could achieve the same by reading incoming MIDI CC messages. There is a piece of code commented out that should get you started, if you want to try it out.

The sine wave in the EEPROM table is an approximation of a sine wave, that sounds good enought at 8-bit resolution. It is not feasible to calculate the sine function on the go with my code. If you want to experiment with different waveforms, it is trivial to edit the waveform table to suit your taste.

## Disclaimer
The code worked on my Arduino Duemilanove. I have not tested it on any other board. It may or may not work on later Arduinos. I am sorry to say I no longer have access to my Arduino, so if there are any problems with running the code, I can not do any debugging with the Arduino. I can read the code however and happily follow along. So if there is any confusion with the software, I may be able to help.

