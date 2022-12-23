# TechTech Technologies Green Cathedral

Note: this plugin is not yet ready for release.

## TODO

* Rethink Tia design
* QMod II

## MVP

* Prometheus II 
* Polyphemus II
* Cobalt I + Sixty I
* Once I
* Tia/Mat I (maybe)
* Labels and panels for all of the above

## Target Modules

| Type | Name |
|-|-|
| Oscillator | Prometheus II |
| Filter | Polyphemus II |
| LFO | Cobalt I |
| Utility | Once I |
| Chromatic Mixer | Tiamat N |
| Frequency Mixer | (QMOD) II |
| Delay | Unwound Timeloop |

## TODO

### Infrastructure

* Trimmer knob widget
* Encoder knob widget
* Visually less ugly of label panels would be nice
* Implement actual documentation of all modules

### Prometheus II

* Design 10 HP Panel
* Port dev version
* Update UI to match HW
* Update coupling filter to match HW (with better knob curve)
* Implement adaptive LFSR overclocking

### Polyphemus II

* Review
* I wonder what would happen if you had some 0's zoom up toward the unit circle at the non-envelope cutoff frequency of the filter? Like some kind of inverse resonance above the knee.
  * It basically just becomes a high-pass filter
* I wonder if I can just add another pole on the negative real axis to make it a bandpass
  * The way these filters work, you need low lpf cutoff and high hps cutoff to get any significant attenuation at the opposite end of the spectrum, so this isn't really viable.
* A complex-conjugate pair bpf is not too hard to normalize. Any sort of bandwidth control would be a pain in the ass to compute, but precise bw control isn't needed here since the center frequency can be set directly.
* Maybe a pingable bandpass mode that puts the envelope on the input instead of the cutoff
  * Set the eye to always fully open, but animate the rays



### Cobalt I

* Update label panel font/positioning for clarity?
* Move selection menus into dropdows line panel select

#### Sixty I

* Create label panel
* Maybe look into allowing other modules to use this as a waveform source

### Once I

* Think about a better name?
* Maybe a very simple label panel.
* Move selection menus into dropdows line panel select

### QMod II

* Come up with a real name
* Design
  * Needs built-in image/harmonic filters
  * Doesn't need to be 4 copies on one panel
  * Existing ext carrier input is useless
    * Maybe implement hilbert transform filter
    * Maybe just implement a small LFO with a few different waveforms
    * Cobalt should have built-in quadrature outputs?
    * Maybe it could use Sixty as an expander that does different carrier waveforms instead
  * Maybe separate controls for modulatre/demodulate carriers, but derive them from the same phase accumulator so their relative phases will always be the same when they have the same frequency
    * Maybe carrier coherence could be a menu option 

### Tiamat

* Review UI issues
  * Multiple channels with different routing and different xfade CV's
* Bigger buttons maybe to help some with mouse problems?

### Unwound Timeloop

* Solve UI design

## Panels

Some modules have multiple panel art options that can be selected from the context menu by right-clicking the module. Some modules also have optional panel labels that can be enabled from the context menu.

## Polyphemus II

IIR filter with v/oct and envelope inputs

The filter consists of a single 1-10'th order real pole with radius computed from cutoff frequency. The pole order can be selected using a knob.

The envelope scales the cutoff frequency by a piecewise linear curve with a configurable knee. 

k is the knee value from 0 to 1

wc is the base cutoff frequency from the v/oct and pitch knobs

p is the frequency gain above knee from 0 to 19

The mapping from envelope voltage to cutoff frequency is

| Envelope Voltage | Cutoff Frequency |
|----|---|
| 0 - 10\*k V   | 0 - wc |
| 10\*kV - 10 V | wc - wc\*(1+p) |

#### Controls

The envelope level knob attenuates the envelope voltage or sets the normal voltage of the envelope input.

The envelope bias knob applies a fixed bias offset to the envelope.

#### Polyphony

Each filter input/output supports polyphony up to 16 channels.

When on of the v/oct or envelope inputs is patched with a polyphonic input, the resulting cutoff frequencies are applied sequentially to the filter channels:
| v/oct/env input | filter cutoff source |
|-|-|
|0|0|
|1|1|
|2|2|
|X|2|
|X|2|

#### TODO

* Make knobs have nice units
  * Frequency for base pitch
  * Volts for bias offset and cutoff knee
* Make a version with just one pane and deprecate this one
  * An octave control and hp/lp switch might be nice (instead of some of the redundant filter ports)
  * Make a panel

### Once I

Button press synchronizer.

Updates gate outputs from button presses only on clock edges. Maybe be rising, falling, or both edges.

When the enable switch isn't activated, the module won't update the outputs.

Button light colors:
* Unlit: Gate low
* Green: Pending high
* White: Gate high
* Blue: Pending low

Button Modes:
* Toggle Gate
  * Toggle the corresponding gate output on the next clock edge.
  * Enable button lights up green when clock low and white when clock high
* Pulse
  * Produce a single pulse on the gate output on the next clock edge.
  * Enable button lights up green and flashes white on clock edges
* Pass Clock
  * On the next edge, start passing the logic state of the clock to the corresponding gate output.
  * Enabled button lights up blue when clock is low and white when clock is high
* Pulse on Clock
  * Procue a pulse on the gate output on every clock edge.
  * Enabled button lights up blue and flashes white on clock edges

TODO:
* Refactor the code so that it doesn't suck
  * I would like to be able to reuse the logic for a multichannel version with different clocks on each button.
  * Or maybe update this version so a polyphonic clock input applies a different clock to each button
  * Right now the code is very, umm, terrible

### Cobalt I

Subharmonic sequence oscillator

Generates waveforms with periods that are consecutive integer multiples of a fundamental period. Controls are provided for

* Combined waveform period in seconds
* Starting multiple from 1 to 7
* Sequence length from 1 to 7
* Waveform starting/intersection phase
* Square wave pulse width
* Waveform scale and offset

The phase control allows control of the intersection point of the generated waveforms.

Consider using it to drive the crossfaders in Tia I.

TODO:
* Align square wave phase to other waveforms more better


#### Sixty I

Expander adds a bunch of additional waveforms
* Asymmetric triangle wave with symmetry control (from rising ramp to triangle to falling ramp)
* Raised cosine wave with rolloff control (from cosine to square wave)
* Slew-limited square wave with slope control (from trangle wave to square wave)
* RC/exponential decay wave with decay rate control
* Exponential converter (for making waveforms peakier)
* 1-X converter (for reversing minimum and maximum values of waveforms)
* Subdivision clock (integer multiple of fastest output frequency)

TODO:
* Make expanders stackable (search through -60 instances to find Cobalt instance)
* Make exponential converter factor configurable


