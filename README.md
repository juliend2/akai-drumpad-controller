README
======

## Ok, so basically, how this work is this:

first, i need `alsa` and `alsa-lib`, or something like that, to be installed.
then, i can do `aconnect -l` to list the stuff that handle sound (in and out)
It will give something like:

```
client 20: 'LPD8' [type=kernel,card=1]
        0 'LPD8 MIDI 1     '
                Connecting To: 14:0
```

as you can see, there is a 20 and a 0.
this is the input.

and as you can see, i also have an output which is:

```
client 14: 'Midi Through' [type=kernel]
        0 'Midi Through Port-0'
                Connected From: 20:0
```

asyou can see, it's already connected. but it's 14.0.

so, in a terminal, do this, to connect both together:

```
aconnect 20:0 14:0
```

then you're good to go. you can then tap some drums and adjust some knobs, and it will show up in STDIN!

