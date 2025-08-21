#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
typedef struct{
	uint16_t frame_sequence_counter, frame_sequence_step;
	float audio_cycles_1, audio_cycles_2, audio_cycles_3, audio_cycles_4;
	uint16_t audio_length_1, audio_length_2, audio_length_3, audio_length_4;
	uint8_t ch1_duty_position, ch2_duty_position;
	uint8_t ch1_envelope_volume, ch2_envelope_volume, ch4_envelope_volume;
	int ch1_envelope_timer, ch2_envelope_timer, ch4_envelope_timer;
	int ch3_wave_position;
	bool ch4_wave_state;
	bool ch1_triggered, ch2_triggered, ch3_triggered, ch4_triggered;
	bool ch1_sweep_enabled;
    int ch1_sweep_timer;
    uint16_t ch1_sweep_shadow_freq;
} APU;
void audio_callback(void* userdata, uint8_t* stream, int len);
void update_audio(int cycles);
void audio_set_sample_rate(int freq);
APU apu;
