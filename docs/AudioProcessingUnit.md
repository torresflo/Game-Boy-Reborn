# The Game Boy APU: How Sound Works, and How We Emulate It

This document explains how the original Game Boy hardware produces sound, and how
this emulator reproduces that behavior in `src/Core/AudioProcessingUnit.h/.cpp` and
`src/Core/AudioProcessingUnitTypes.h/.cpp`. It assumes you can read C++ but know
nothing about digital audio — every audio concept is explained from first principles
before we map it onto the code.

## 1. Digital audio in five minutes

A speaker makes sound by physically moving back and forth, pushing on the air. A
single push-and-release makes a click, not a tone — to hear a continuous pitch, the
speaker has to oscillate (move back and forth) *repeatedly*, many times per second.

- **How fast** it oscillates is the **frequency** (pitch). More oscillations per
  second = higher pitch.
- **How far** it moves on each oscillation is the **amplitude** (volume).
- **The exact shape** of one oscillation — does it move smoothly, or snap
  instantly from one extreme to the other? — is the **waveform**, and it's why a
  flute and a square-wave Game Boy beep can play the exact same musical note and
  still sound completely different. Same frequency and amplitude, different shape.

Digital audio represents that moving-speaker-position as a sequence of numbers, one
per moment in time. Played back through a sound card fast enough, those numbers
recreate the original motion. How often we need a new number is the **sample
rate** — 44100 Hz ("44.1kHz", CD quality) means 44,100 numbers per second, per audio
channel (left/right for stereo, interleaved: `L, R, L, R, ...`).

One crucial, slightly counter-intuitive point that comes up later: **a sound card
playing a constant, unchanging number produces silence**, no matter what that
number is. Hearing anything at all requires the *value to change* over time — only
oscillation is audible. A signal that holds steady at "weird offset X" is just as
silent as one that holds steady at zero; it just has a different inaudible DC bias.
This is what allows our mixer's "centered around an arbitrary chosen value, not
literally zero" math (section 6.4) to still produce correct silence.

## 2. The big picture: four independent sound generators

The Game Boy doesn't have anything resembling a modern audio API. It has four tiny,
independent circuits called **channels**, each constantly producing a small digital
number (0–15, i.e. 4 bits) that represents "where its waveform currently is". Each
channel has its own little DAC ("Digital-to-Analog Converter") that turns that 4-bit
number into a real, tiny voltage. A simple analog mixer circuit adds the four
channels' voltages together, applies a volume knob, and sends the result to the
left/right speaker terminals.

```
Channel 1 (pulse+sweep) ----\
Channel 2 (pulse)        ----+--> [mixer: add, apply volume, route L/R] --> speakers
Channel 3 (wave)         ----+
Channel 4 (noise)        ----/
```

There's no "play this note" function call — the CPU controls all of this purely by
writing bytes into a handful of memory-mapped registers (conventionally named
`NR10`–`NR52`, at addresses `0xFF10`–`0xFF26`, plus a 16-byte "wave RAM" at
`0xFF30`–`0xFF3F`). The hardware reacts to those register values in real time, tick
by tick, off the same 4,194,304 Hz master clock that drives the CPU and the PPU.
Our `AudioProcessingUnit::tick()` is called once per T-cycle from
`CentralProcessingUnit::emulateCycles()`, exactly like `PixelProcessingUnit::tick()`
— the APU is just another piece of hardware riding the same clock.

## 3. The pulse channels (1 and 2) — square waves

A pulse channel's waveform is dead simple: it's either "high" (digital value =
current volume) or "low" (digital value = 0), and it flips between the two on a
fixed schedule. The *proportion* of time spent high vs. low in each cycle is the
**duty cycle**, and the Game Boy supports four of them:

```
12.5%:  _______-        (high for 1 of 8 steps)
25%:    -______-        (high for 2 of 8 steps)
50%:    -____---        (high for 4 of 8 steps) <- classic "square wave" sound
75%:    _------_        (high for 6 of 8 steps)
```

(read left-to-right as one repeating cycle, 8 steps wide; `-`=high, `_`=low)

Each duty cycle has a distinctly different timbre even at the same pitch — the
"thin, nasal" Game Boy lead sound is usually a 12.5% or 25% duty pulse. In code
this is `DutyWaveforms` in `AudioProcessingUnitTypes.cpp`, indexed by `waveDuty`
(0-3) and the channel's current step, `dutyPosition` (0-7).

**Pitch** is controlled by an 11-bit `periodValue` register (split across two
hardware bytes). Counter-intuitively, a *smaller* `periodValue` means a *higher*
pitch: the hardware counts a timer down from `(2048 - periodValue) * 4` T-cycles to
zero, and every time it reaches zero, it advances `dutyPosition` by one step and
reloads. A smaller `periodValue` means a smaller countdown, so the waveform steps
through its cycle faster — and stepping faster means a higher frequency. This is
exactly what `tickChannel1()`/`tickChannel2()` do every T-cycle: decrement a
counter, and when it hits zero, reload it and advance one step.

Two more things every pulse (and noise) channel has:

- **Volume envelope**: the volume can automatically ramp up or down over time
  (think a fade-in or a fade-out), at a configurable pace. This is what makes a
  plucked-string or drum-hit sound natural instead of an abrupt on/off blip.
  Implemented by `clockEnvelope()`, called from the frame sequencer (section 4).
- **Length counter**: an optional "auto turn-off after N units of time" — useful
  for short sound effects the game can trigger and then forget about, instead of
  having to manually silence the channel later. Implemented by
  `clockLengthCounter()`.

Channel 1 additionally has a **frequency sweep**: it can automatically slide its
own pitch up or down over time — the classic Game Boy "laser"/"siren" sound effect.
Each sweep step recomputes the period as `shadowFrequency ± (shadowFrequency >>
slope)`; if that ever pushes the period out of the valid 11-bit range, the hardware
disables the channel outright (`calculateSweepFrequency()` in
`AudioProcessingUnit.cpp`).

## 4. The wave channel (3) — your own custom waveform

Instead of a fixed duty cycle, channel 3 plays back a *user-supplied* waveform: 32
samples, each 4 bits, stored two-per-byte in the 16-byte "wave RAM"
(`0xFF30`–`0xFF3F`). The game writes any repeating shape it likes into that RAM —
this is a tiny version of **wavetable synthesis** — and the channel loops through
those 32 samples at a rate controlled by `periodValue`, the same way the pulse
channels step through their 8-step duty pattern (just a different timer constant:
`(2048 - periodValue) * 2`, since there are 32 steps instead of 8).

There's no envelope on this channel; volume is a simple bit-shift of each sample
("output level": 100%, 50%, or 25%), done in `WaveChannel::getCurrentSample()`.

## 5. The noise channel (4) — hiss, percussion, explosions

Channel 4 doesn't play a waveform at all — it plays *noise*, generated by a neat,
fully deterministic trick called a **Linear Feedback Shift Register (LFSR)**.

The idea: keep a register of bits. On every step, look at bits 0 and 1, XOR them
together, shift the whole register right by one, and feed that XOR result back in
at the top. Repeat. The bit that falls out at position 0 each step looks
statistically random (it isn't really — given the same starting value it always
produces the exact same sequence — but it's "noisy enough" to sound like static,
hissing, or percussion), and that's literally where this channel's audio signal
comes from: bit 0 clear = on, bit 0 set = off.

```cpp
bool xorResult = bit(reg, 0) != bit(reg, 1);
reg >>= 1;
setBit(reg, 14, xorResult);              // 15-bit mode
if (shortMode) setBit(reg, 6, xorResult); // 7-bit mode: shorter repeat, more "metallic"
```

This is `stepNoiseShiftRegister()` in `AudioProcessingUnit.cpp`. "Short mode" also
folds the feedback into bit 6, which makes the bit pattern repeat much sooner (after
127 steps instead of 32767), producing a higher-pitched, more "metallic" noise
instead of white-noise hiss. How fast the LFSR steps (i.e. the noise's "pitch"/
texture) is set by a small lookup table of divisors combined with a shift amount
(`NoiseChannel::getPeriodTimerReloadValue()`), rather than the `2048 - period`
formula the other channels use.

Channel 4 also has the same volume envelope and length counter as the pulse
channels — just no pitch/duty concept, since there's no waveform to step through,
only the shift register.

## 6. The conductor: the frame sequencer, and the mixer

### 6.1 Why you need a second, slower clock

Envelope, sweep, and length all need to change at fixed rates that are *much*
slower than the 4.19MHz main clock — and they're not nice round divisions of it
either (64 Hz, 128 Hz, 256 Hz). Re-deriving "should the envelope fire on this exact
T-cycle?" from scratch every tick would be wasteful and fiddly. So real hardware
(and our emulation) keeps a simple, second clock-divider just for this — exactly
the same idea as the CPU's own `HardwareTimer` (`Timer.h`), just for the APU.

This is the **frame sequencer**: a counter that ticks once every 8192 T-cycles
(4,194,304 / 8192 = 512 Hz) and walks through 8 steps, each of which fires a subset
of the slower clocks:

| Step | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| Length (256 Hz)   | ✓ | | ✓ | | ✓ | | ✓ | |
| Sweep (128 Hz)     | | | ✓ | | | | ✓ | |
| Envelope (64 Hz)   | | | | | | | | ✓ |

`AudioProcessingUnit::stepFrameSequencer()` is the direct translation of this
table into code.

### 6.2 The mixer registers (NR50/NR51/NR52)

- **`NR51`** (panning): one bit per channel per stereo side — does this channel's
  sound go to the left speaker, the right, both, or neither?
- **`NR50`** (master volume): a 0–7 "loudness knob" for each side. Surprising
  quirk, worth remembering: **0 is not silence** — it's just the *quietest
  non-zero* volume (a ×1 multiplier; 7 is ×8). The only ways to get true silence
  are turning a channel's DAC off, routing it to neither side via `NR51`, or
  powering the whole APU down.
- **`NR52`** (master power + status): bit 7 turns the entire APU on/off (and per
  real hardware, powering off forcibly silences every channel and clears the
  mixer settings — see `AudioProcessingUnit::powerOff()`); bits 0–3 are read-only
  "is this channel currently active" flags, handy for games (and our
  `getChannelEnabledMask()`) to poll.

## 7. From four 4-bit numbers to one stereo PCM stream

Each channel exposes its *current* 0–15 digital value via `getCurrentSample()`.
Mixing means, for each stereo side: sum up the channels routed there (via
`NR51`), multiply by the side's master volume, then rescale that into the
signed 16-bit range a real audio API expects (`mixSample()` in
`AudioProcessingUnit.cpp`). We don't bother modeling individual analog voltages —
working entirely in integers and combining the digital values directly is both
simpler and indistinguishable in practice.

### 7.1 Downsampling: from 4.19 million updates/second to 44,100

The four channels' digital values can change up to 4,194,304 times per second, but
a typical sound card only wants 44,100 numbers per second. We don't need to record
every single change — just take a "snapshot" of the current mixed value at a
steady rate of 44,100 times per second, the same way a camera doesn't need to
record every photon, just one frame every so often.

The wrinkle: 4,194,304 isn't evenly divisible by 44,100, so "snapshot every Nth
tick" with a fixed whole-number `N` would slowly drift out of sync over time.
`generateSampleIfDue()` solves this with a running-total trick (sometimes called a
"Bresenham" technique, after the line-drawing algorithm that has the same shape):
every tick, add 44,100 to a counter; whenever that counter reaches 4,194,304 or
more, take a snapshot and subtract 4,194,304 back off (keeping whatever's left
over for next time). This guarantees *exactly* 44,100 snapshots per second on
average, using only integer addition — no floating point, no drift, no division in
the hot path.

### 7.2 Getting samples out of the emulator and into your speakers

`AudioProcessingUnit` only ever *accumulates* samples into a buffer
(`sampleBuffer`); something else has to actually play them. That's `src/UI/`'s job:

- **`AudioRingBuffer`**: a small thread-safe queue. `Application::updateEmulation()`
  calls `emulator.drainAudioSamples()` once per emulated frame (~60 times/second)
  and pushes whatever was generated into this buffer.
- **`GameBoyAudioStream`** (an `sf::SoundStream`): SFML runs its *own* background
  thread that continuously asks "give me more samples" via `onGetData()`. That
  callback pulls from the same ring buffer (padding with silence if we haven't
  generated enough yet — an "underrun" — rather than stalling).

Two different threads (the emulation loop, and SFML's audio thread) need to share
this data safely, which is exactly what the ring buffer's internal mutex is for.

## 8. Putting it all together: tracing one beep from register write to speaker

Say a game wants to play a short beep on channel 2:

1. It writes `NR21` (`0xFF16`): duty 50%, plus a length value.
2. It writes `NR22` (`0xFF17`): initial volume, envelope settings.
3. It writes `NR23` (`0xFF18`): the low byte of the desired pitch.
4. It writes `NR24` (`0xFF19`): the high bits of the pitch, **plus the trigger bit**.
5. That last write is the magic moment: `AudioProcessingUnit::writeNR24()` sees the
   trigger bit set and calls `channel2.trigger()`, which flips `channelEnabled` on
   and reloads the period timer, volume, and length counter from everything written
   in steps 1–3.
6. From here on, every T-cycle, `tickChannel2()` counts the period timer down and
   advances `dutyPosition` whenever it hits zero — the channel is now actively
   "playing" its waveform at the chosen pitch.
7. Every 8192 T-cycles the frame sequencer runs; on the right steps it clocks the
   length counter, and once it reaches its target, channel 2 turns itself back off
   automatically.
8. 44,100 times per second (independent of what any channel is doing),
   `generateSampleIfDue()` reads channel 2's current 0–15 value via
   `getCurrentSample()`, mixes it with the other three channels per `NR50`/`NR51`,
   and appends a left/right sample pair to the buffer.
9. ~60 times per second, `Application` drains that buffer into the
   `AudioRingBuffer`.
10. In the background, SFML's audio thread keeps asking `GameBoyAudioStream` for
    more data, which it gets from that same ring buffer — and *that* stream of
    numbers is what physically drives your speakers.

## 9. Known simplifications (deliberate v1 scope cuts)

- The frame sequencer is a free-running 8192-tick counter rather than derived from
  the DIV timer register, so the rare "writing to DIV at the wrong moment causes an
  extra frame-sequencer step" hardware quirk isn't reproduced.
- Channel 3's "wave RAM gets corrupted if you retrigger it while it's already
  reading" hardware bug isn't modeled.
- Register writes are not blocked while the APU is powered off (real hardware
  ignores most of them in that state); ours still applies them.
- `TargetPeakAmplitude` (the headroom constant in `mixSample()`) is a hand-picked
  tuning value, not something derived from real hardware — there's no "correct"
  digital loudness for an analog signal that never existed digitally on real
  silicon.
- `NR50`'s VIN bits (a pass-through for audio from the cartridge slot, unused by
  essentially every commercial game) are stored for register read-back fidelity
  but never mixed into the output.
