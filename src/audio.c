#include "audio.h"
#include "flags.h"
#include "memory.h"

const float CPU_CYCLES_PER_SAMPLE = 4194304.0f / 44100.0f; //(cpu_cycles/s)/(samples/s)
const int FRAME_SEQUENCER_RATE = 4194304 / 512; //(cpu_cycles/s)/(frame seqeuncer cycles/s)

static const uint8_t DUTY_PATTERNS[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

void static inline generate_audio_ticks(){
	
}
void update_audio(uint8_t cycles){
	apu.frame_sequence_counter += cycles;

}
void audio_callback(void* userdata, uint8_t* stream, int len) {
    int16_t* buffer = (int16_t*)stream;
    int num_samples = len / sizeof(int16_t);

    for(int i = 0; i < num_samples; i++){
       	if(apu.ch1_triggered){
        	apu.audio_cycles_1 = 0;
		    apu.ch1_duty_position = 0;
			apu.ch1_triggered = false; 
    	}


		uint16_t freq_data_1 = (read_byte(C1_HIGH_REGISTER) & 0x07) << 8 | read_byte(C1_LOW_REGISTER);
		uint16_t freq_data_2 = (read_byte(C2_HIGH_REGISTER) & 0x07) << 8 | read_byte(C2_LOW_REGISTER);
		uint16_t freq_data_3 = (read_byte(C3_HIGH_REGISTER) & 0x07) << 8 | read_byte(C3_LOW_REGISTER);
		uint16_t freq_data_4 = read_byte(C4_FREQ_REGISTER);

		int clock_shift = freq_data_4 >> 4;
		int divisor_code = freq_data_4 & 0x07;
		int divisors[] = {8, 16, 32, 48, 64, 80, 96, 112};

		int period_1 = (2048 - freq_data_1) * 4;
		int period_2 = (2048 - freq_data_2) * 4;
		int period_3 = (2048 - freq_data_3) * 4;
		int period_4 = divisors[divisor_code] << clock_shift;

		if(apu.audio_cycles_1 <= 0){
            apu.audio_cycles_1 += period_1; 
            apu.ch1_duty_position = (apu.ch1_duty_position + 1) % 8;
		}
		if(apu.audio_cycles_2 <= 0){
            apu.audio_cycles_2 += period_2; 
            apu.ch2_duty_position = (apu.ch2_duty_position + 1) % 8; 
        }
		if(apu.audio_cycles_3 <= 0){
            apu.audio_cycles_3 += period_3; 
            apu.ch3_wave_position = (apu.ch3_wave_position + 1) % 32; 
        }
       	if(apu.audio_cycles_4 <= 0){
            apu.audio_cycles_4 += period_4; 
            apu.ch4_wave_state = !apu.ch4_wave_state; 
        }
		
		uint8_t duty_setting_1 = (read_byte(C1_TIMER_REGISTER) >> 6) & 0x03;
		uint8_t wave_state_1 = DUTY_PATTERNS[duty_setting_1][apu.ch1_duty_position];
		uint8_t duty_setting_2 = (read_byte(C2_TIMER_REGISTER) >> 6) & 0x03;
        uint8_t wave_state_2 = DUTY_PATTERNS[duty_setting_2][apu.ch2_duty_position];


		uint8_t vol_data_1 = read_byte(C1_VOLUME_REGISTER) >> 4;
		uint8_t vol_data_2 = read_byte(C2_VOLUME_REGISTER) >> 4;
        uint8_t vol_data_4 = read_byte(C4_VOLUME_REGISTER) >> 4;

		int16_t amplitude_1 = vol_data_1 * 200;
		int16_t amplitude_2 = vol_data_2 * 200; 
		
		uint16_t wave_addr = 0xFF30 + (apu.ch3_wave_position / 2);
		uint8_t wave_byte = read_byte(wave_addr);

		uint8_t sample_4bit = (apu.ch3_wave_position % 2 == 0) ? (wave_byte >> 4) : (wave_byte & 0x0F);

        uint8_t vol_shift_code = (read_byte(C3_OUTPUT_REGISTER) >> 5) & 0x03;
		int16_t amplitude_3 = 0;
		if(vol_shift_code > 0){
			amplitude_3 = sample_4bit >> (vol_shift_code - 1);
		}


		int16_t amplitude_4 = vol_data_4 * 200;

        int curr = 0;
		curr += wave_state_1 ? amplitude_1 : -amplitude_1;
		curr += wave_state_2 ? amplitude_2 : -amplitude_2;
		curr += amplitude_3;
		curr += apu.ch4_wave_state ? amplitude_4 : -amplitude_4;

		buffer[i] = curr;

        apu.audio_cycles_1 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_2 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_3 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_4 -= CPU_CYCLES_PER_SAMPLE;
    	
	}
}
