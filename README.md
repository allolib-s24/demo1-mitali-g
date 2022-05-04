# Allolib PCM Synth
A lo-fi sampling synthesizer built in Allolib as a synth voice.

My current goal is to create a music video for an original electronic pop music track I composed. Both the song and visuals will be synthesized in real-time within Allolib, and will synchronize together.

At the time, there are no visuals, but I've fully implemented a couple measures of the music.

## Features
- Load WAV sample files (mono, 48khz)
- Play samples at different pitches
- Modify sounds with attack and release envelopes
- Define timbre and drum kit sound patches
- Multisampling: Timbre contains multiple samples at different pitches, and synth will choose closest sample for a given note
- Linear interpolation: When playing samples at slower speeds, create new samples between existing to make sound "smoother"
- Abstraction for Allolib sequencer to easily sequence a pattern in code

To run, use `./run.sh`.

Developed by Jake Delgado
