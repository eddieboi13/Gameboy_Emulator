#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
typedef struct{
	uint16_t frame_sequence_counter;
	uint16_t audio_cycles_1, audio_cycles_2, audio_cycles_3, audio_cycles_4;
	uint8_t ch1_duty_position, ch2_duty_position;
	int ch3_wave_position;
	bool ch4_wave_state;
	bool ch1_triggered;
} APU;
void audio_callback(void* userdata, uint8_t* stream, int len);
void update_audio(uint8_t cycles);
APU apu;
