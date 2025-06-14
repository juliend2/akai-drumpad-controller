#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <signal.h>

/*
Ok, so basically, how this work is this:
first, i need alsa and alsa-lib, or something like that, to be installed.
then, i can do aconnect -l to list the stuff that handle sound (in and out)
It will give something like:
	client 20: 'LPD8' [type=kernel,card=1]
			0 'LPD8 MIDI 1     '
					Connecting To: 14:0
as you can see, there is a 20 and a 0.
this is the input.

and as you can see, i also have an output which is:
	client 14: 'Midi Through' [type=kernel]
			0 'Midi Through Port-0'
					Connected From: 20:0
asyou can see, it's already connected. but it's 14.0.

so, in a terminal, do this, to connect both together:
	aconnect 20:0 14:0

then you're good to go. you can then tap some drums and adjust some knobs, and it will show up in STDIN!

*/

static int running = 1;

void signal_handler(int sig) {
    running = 0;
}

int main() {
    snd_seq_t *seq_handle;
    snd_seq_event_t *ev;
    int client_id, port_id;
    int status;
    
    // Set up signal handler for clean exit
    signal(SIGINT, signal_handler);
    
    // Open ALSA sequencer
    status = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    if (status < 0) {
        printf("Error opening ALSA sequencer: %s\n", snd_strerror(status));
        return 1;
    }
    
    // Set our client name
    snd_seq_set_client_name(seq_handle, "LPD8 Monitor");
    
    // Create input port
    port_id = snd_seq_create_simple_port(seq_handle, "Input",
                                        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (port_id < 0) {
        printf("Error creating input port: %s\n", snd_strerror(port_id));
        snd_seq_close(seq_handle);
        return 1;
    }
    
    client_id = snd_seq_client_id(seq_handle);
    
    // Connect to LPD8 (client 20, port 0)
    status = snd_seq_connect_from(seq_handle, port_id, 20, 0);
    if (status < 0) {
        printf("Error connecting to LPD8: %s\n", snd_strerror(status));
        printf("Make sure LPD8 is connected and try: aconnect 20:0 %d:%d\n", client_id, port_id);
        snd_seq_delete_simple_port(seq_handle, port_id);
        snd_seq_close(seq_handle);
        return 1;
    }
    
    printf("Connected to AKAI LPD8 via ALSA sequencer.\n");
    printf("LPD8 -> Our client (%d:%d)\n", client_id, port_id);
    printf("Hit some pads! Press Ctrl+C to exit.\n");
    
    while (running) {
        status = snd_seq_event_input(seq_handle, &ev);
        
        if (status >= 0) {
            switch (ev->type) {
                case SND_SEQ_EVENT_NOTEON:
                    if (ev->data.note.velocity > 0) {
                        printf("Pad hit! Note: %d, Velocity: %d, Channel: %d\n", 
                               ev->data.note.note, ev->data.note.velocity, ev->data.note.channel);
                    } else {
                        printf("Pad released! Note: %d, Channel: %d\n", 
                               ev->data.note.note, ev->data.note.channel);
                    }
                    break;
                    
                case SND_SEQ_EVENT_NOTEOFF:
                    printf("Pad released! Note: %d, Channel: %d\n", 
                           ev->data.note.note, ev->data.note.channel);
                    break;
                    
                case SND_SEQ_EVENT_CONTROLLER:
                    printf("Control change! Controller: %d, Value: %d, Channel: %d\n", 
                           ev->data.control.param, ev->data.control.value, ev->data.control.channel);
                    break;
                    
                case SND_SEQ_EVENT_PGMCHANGE:
                    printf("Program change! Program: %d, Channel: %d\n", 
                           ev->data.control.value, ev->data.control.channel);
                    break;
                    
                default:
                    printf("Other MIDI event: type %d\n", ev->type);
                    break;
            }
            
            snd_seq_free_event(ev);
        }
        
        // Small delay to prevent excessive CPU usage
        usleep(1000);
    }
    
    // Cleanup
    snd_seq_disconnect_from(seq_handle, port_id, 20, 0);
    snd_seq_delete_simple_port(seq_handle, port_id);
    snd_seq_close(seq_handle);
    printf("\nDisconnected from AKAI LPD8.\n");
    
    return 0;
}
