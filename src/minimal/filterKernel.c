/*
 *  compilation directive
 *
 *  emcc -O3 -s WASM=1 filterKernel.c -o filterKernel.wasm --no-entry
 */

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <emscripten.h>

#define WEBEAUDIO_FRAME_SIZE 128
#define WEBEAUDIO_READ_SIZE 960
#define BUFFER 4096 

struct buffer {
	int16_t audio[WEBEAUDIO_FRAME_SIZE];
	double time;
};

static struct buffer *buffer = NULL;
static float write_buffer[WEBEAUDIO_FRAME_SIZE];
static int16_t read_buffer[WEBEAUDIO_READ_SIZE];
static double stime;



static inline uint32_t hash_key(double d)
{
	return lround(d / stime) & (BUFFER - 1);
}


static inline int16_t float2short(float in)
{
	double value;
	int16_t out;

	value = in * (8.0 * 0x10000000);

	if (value >= (1.0 * 0x7fffffff)) {
		out = 32767;
	}
	else if (value <= (-8.0 * 0x10000000)) {
		out = -32768;
	}
	else
		out = (short) (lrint (value) >> 16);

	return out;
}


EMSCRIPTEN_KEEPALIVE
void magic_alloc(double srate) 
{
	buffer = malloc(sizeof(struct buffer) * BUFFER);
	stime = 1 / srate * WEBEAUDIO_FRAME_SIZE; 
}


EMSCRIPTEN_KEEPALIVE
void magic_free(void) 
{
	free(buffer);
}


EMSCRIPTEN_KEEPALIVE
int magic_copy(double d, int start) 
{
	int key = hash_key(d);
	int y = start;

	if (d != buffer[key].time)
		return -1;

	for (int i=0; i < WEBEAUDIO_READ_SIZE; i++) {
		read_buffer[i] = buffer[key].audio[y];

		if (y == (WEBEAUDIO_FRAME_SIZE - 1)) {
			key++;
			y = 0;
		} else {
			y++;
		}
	}

	return y;
}


EMSCRIPTEN_KEEPALIVE
float *magic_write(void) {
	return write_buffer;
}

EMSCRIPTEN_KEEPALIVE
int16_t *magic_read(void) {
	return read_buffer;
}

EMSCRIPTEN_KEEPALIVE
int magic_store(double d) 
{
	int key = hash_key(d);

	if (!buffer)
		return -1;

	for (int i=0; i < WEBEAUDIO_FRAME_SIZE; i++) {
		buffer[key].audio[i] = float2short(write_buffer[i]);
	}

	buffer[key].time = d;

	return key;
}
