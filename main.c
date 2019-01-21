/*
  gcc -Wall -O3 morphin.c -o morphin `pkg-config --libs --cflags jack` -lm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <jack/jack.h>

jack_port_t *input1_port, *input2_port;
jack_port_t *output1_port, *output2_port;
jack_client_t *client;

double 
closed_interval_rand(double x0, double x1)
{
  return x0 + (x1 - x0) * rand() / ((double) RAND_MAX);
}

double a = 1.0;

int
process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in1, *out1, *in2, *out2;
	
	in1 = jack_port_get_buffer (input1_port, nframes);
	out1 = jack_port_get_buffer (output1_port, nframes);

  in2 = jack_port_get_buffer (input2_port, nframes);
	out2 = jack_port_get_buffer (output2_port, nframes);

  int len = sizeof (jack_default_audio_sample_t) * nframes;
	jack_default_audio_sample_t mixed[len];
	

  double b = 1.0 - a;

  a -= 0.010;
  if (a <= 0.0)
    a = 1.0;

  /*
  double a = closed_interval_rand(0, 1);
  double b = 1 - a;
  */

  // param mp
  int i = 0;
  for (; i <  len; i++) {
    mixed[i] = (in1[i] * a) + (in2[i] * b);
  }

	memcpy (out1, mixed, sizeof (jack_default_audio_sample_t) * nframes);
	memcpy (out2, mixed, sizeof (jack_default_audio_sample_t) * nframes);
  //

	return 0;      
}

void
jack_shutdown (void *arg)
{
	exit (EXIT_FAILURE);
}

int
jack_init (void)
{
	const char **ports;
	const char *client_name = "morphin";
	const char *server_name = NULL;

	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, " "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		return -1;
	}

	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}

	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	jack_set_process_callback (client, process, 0);

	jack_on_shutdown (client, jack_shutdown, 0);

	input1_port = jack_port_register (client, "input1",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);

	output1_port = jack_port_register (client, "output1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	input2_port = jack_port_register (client, "input2",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);

	output2_port = jack_port_register (client, "output2",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);


	if ((input1_port == NULL) || (output1_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
    return -1;
	}

	if ((input1_port == NULL) || (output2_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
    return -1;
	}

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
    return -1;
	}

	ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
    return -1;
	}

	if (jack_connect (client, ports[0], jack_port_name (input1_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

  if (jack_connect (client, ports[0], jack_port_name (input2_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
    return -1;
	}

	if (jack_connect (client, jack_port_name (output1_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

  if (jack_connect (client, jack_port_name (output2_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);

  return 0;
}

int
main (int argc, char *argv[])
{

  jack_init();

	sleep (-1);

	jack_client_close (client);

	return EXIT_SUCCESS;
}
