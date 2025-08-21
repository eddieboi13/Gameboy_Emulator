#include "audio.h"
#include "flags.h"
#include "memory.h"
static float CPU_CYCLES_PER_SAMPLE = 4194304.0f / 44100.0f;
void audio_set_sample_rate(int freq){
    CPU_CYCLES_PER_SAMPLE = 4194304.0f / (float)freq;
}
const int FRAME_SEQUENCER_RATE = 4194304 / 512; //(cpu_cycles/s)/(frame seqeuncer cycles/s)

static const uint8_t DUTY_PATTERNS[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

void tick_envelopes(){
    uint8_t nrx2_1 = read_byte(C1_VOLUME_REGISTER);
    int period_1 = nrx2_1 & 0x07;
    if(period_1 > 0){
        if(apu.ch1_envelope_timer > 0){
            apu.ch1_envelope_timer--;
            if(apu.ch1_envelope_timer == 0){
                apu.ch1_envelope_timer = period_1;
                bool adding = (nrx2_1 & 0x08) != 0;
                if(adding && apu.ch1_envelope_volume < 15){
                    apu.ch1_envelope_volume++;
                }else if (!adding && apu.ch1_envelope_volume > 0){
                    apu.ch1_envelope_volume--;
                }
            }
        }
    }

    uint8_t nrx2_2 = read_byte(C2_VOLUME_REGISTER);
    int period_2 = nrx2_2 & 0x07;
    if(period_2 > 0){
        if(apu.ch2_envelope_timer > 0){
            apu.ch2_envelope_timer--;
            if(apu.ch2_envelope_timer == 0){
                apu.ch2_envelope_timer = period_2;
                bool adding = (nrx2_2 & 0x08) != 0;
                if(adding && apu.ch2_envelope_volume < 15){
                    apu.ch2_envelope_volume++;
                } else if(!adding && apu.ch2_envelope_volume > 0){
                    apu.ch2_envelope_volume--;
                }
            }
        }
    }

    uint8_t nrx2_4 = read_byte(C4_VOLUME_REGISTER);
    int period_4 = nrx2_4 & 0x07;
    if (period_4 > 0) {
        if (apu.ch4_envelope_timer > 0) {
            apu.ch4_envelope_timer--;
            if (apu.ch4_envelope_timer == 0) {
                apu.ch4_envelope_timer = period_4;
                bool adding = (nrx2_4 & 0x08) != 0;
                if (adding && apu.ch4_envelope_volume < 15) {
                    apu.ch4_envelope_volume++;
                } else if (!adding && apu.ch4_envelope_volume > 0) {
                    apu.ch4_envelope_volume--;
                }
            }
        }
    }
}
void tick_sweep(){
    if(!apu.ch1_sweep_enabled){
        return;
    }

    if(apu.ch1_sweep_timer > 0){
        apu.ch1_sweep_timer--;
    }

    if(apu.ch1_sweep_timer == 0){
        uint8_t sweep_reg = read_byte(C1_SWEEP_REGISTER);
        int period = (sweep_reg >> 4) & 0x07;
        if(period > 0){
            apu.ch1_sweep_timer = period;

            int shift = sweep_reg & 0x07;
            if(shift > 0){
                bool negate = (sweep_reg & 0x08) != 0;
                
                uint16_t new_freq_diff = apu.ch1_sweep_shadow_freq >> shift;
                uint16_t new_freq;
                if(negate){
                    new_freq = apu.ch1_sweep_shadow_freq - new_freq_diff;
                } 
				else{
                    new_freq = apu.ch1_sweep_shadow_freq + new_freq_diff;
                }

                if(new_freq > 2047){
                    memory[AUDIO_MASTER_REGISTER] &= ~0x01;
                    apu.ch1_sweep_enabled = false;
                } 
				else{
                    apu.ch1_sweep_shadow_freq = new_freq;
                    
                    memory[C1_LOW_REGISTER] = new_freq & 0xFF;
                    memory[C1_HIGH_REGISTER] = (memory[C1_HIGH_REGISTER] & 0xF8) | ((new_freq >> 8) & 0x07);

                    uint16_t overflow_check = apu.ch1_sweep_shadow_freq >> shift;
                    if(negate){
                        overflow_check = apu.ch1_sweep_shadow_freq - overflow_check;
                    } 
					else{
                        overflow_check = apu.ch1_sweep_shadow_freq + overflow_check;
                    }
                    if(overflow_check > 2047){
                        memory[AUDIO_MASTER_REGISTER] &= ~0x01;
                    }
                }
            }
        }
    }
}

void tick_length_counters(){
    if(apu.audio_length_2 > 0 && (read_byte(C1_HIGH_REGISTER) & 0x40)){
        apu.audio_length_2--;
        if(apu.audio_length_2 == 0){
            memory[AUDIO_MASTER_REGISTER] &= ~0x01;
        }
    }
	if(apu.audio_length_2 > 0 && (read_byte(C2_HIGH_REGISTER) & 0x40)){
        apu.audio_length_2--; 
        if(apu.audio_length_2 == 0){
            memory[AUDIO_MASTER_REGISTER] &= ~0x01;
        }
    }
	if(apu.audio_length_3 > 0 && (read_byte(C3_HIGH_REGISTER) & 0x40)){
    	apu.audio_length_3--;
	    if(apu.audio_length_3 == 0){
        	memory[AUDIO_MASTER_REGISTER] &= ~0x04; // Disable bit 2
    	}
	}
	if(apu.audio_length_4 > 0 && (read_byte(C4_CONTROL_REGISTER) & 0x40)){
        apu.audio_length_4--;
        if(apu.audio_length_4 == 0){
            memory[AUDIO_MASTER_REGISTER] &= ~0x01;
        }
    }
}

void update_audio(int cycles){
	apu.frame_sequence_counter += cycles;
	if(apu.frame_sequence_counter >= FRAME_SEQUENCER_RATE){
		apu.frame_sequence_counter -= FRAME_SEQUENCER_RATE;
		if(apu.frame_sequence_step % 2 == 0) {
            tick_length_counters();
        }
		if (apu.frame_sequence_step == 2 || apu.frame_sequence_step == 6) {
            tick_sweep();
        }
		if (apu.frame_sequence_step == 7) {
            tick_envelopes();
        }
		apu.frame_sequence_step = (apu.frame_sequence_step + 1) % 8;
	}
}

void audio_callback(void* userdata, uint8_t* stream, int len){
    int16_t* buffer = (int16_t*)stream;
    int num_samples = len / sizeof(int16_t);

    for(int i = 0; i < num_samples; i++){
		if(apu.ch1_triggered){
		    apu.ch1_triggered = false;
		    memory[AUDIO_MASTER_REGISTER] |= 0x01;
		    apu.audio_cycles_1 = 0;
		    apu.ch1_duty_position = 0;
		    uint8_t length_data = read_byte(C1_TIMER_REGISTER) & 0x3F;
		    apu.audio_length_1 = 64 - length_data;
		    uint8_t nrx2 = read_byte(C1_VOLUME_REGISTER);
		    apu.ch1_envelope_volume = nrx2 >> 4;
		    apu.ch1_envelope_timer = nrx2 & 0x07;
		    uint8_t sweep_reg = read_byte(C1_SWEEP_REGISTER);
		    apu.ch1_sweep_shadow_freq = (read_byte(C1_HIGH_REGISTER) & 0x07) << 8 | read_byte(C1_LOW_REGISTER);
		    int period = (sweep_reg >> 4) & 0x07;
		    int shift = sweep_reg & 0x07;
		    apu.ch1_sweep_timer = (period == 0) ? 8 : period;
		    apu.ch1_sweep_enabled = (period > 0 || shift > 0);
		    if(shift > 0){
        		uint16_t new_freq_diff = apu.ch1_sweep_shadow_freq >> shift;
		        if(apu.ch1_sweep_shadow_freq + new_freq_diff > 2047){
        		    memory[AUDIO_MASTER_REGISTER] &= ~0x01; // Disable channel on immediate overflow
		        }
    		}
		}	
		if(apu.ch2_triggered){
		    apu.audio_cycles_2 = 0;
		    apu.ch2_duty_position = 0;
		    uint8_t length_data = read_byte(C2_TIMER_REGISTER) & 0x3F;
		    apu.audio_length_2 = 64 - length_data;
		    memory[AUDIO_MASTER_REGISTER] |= 0x02;
		    uint8_t nrx2 = read_byte(C2_VOLUME_REGISTER);
            apu.ch2_envelope_volume = nrx2 >> 4;
            apu.ch2_envelope_timer = nrx2 & 0x07;
			apu.ch2_triggered = false;
		}
		if(apu.ch3_triggered){
    		apu.audio_cycles_3 = 0;
	    	apu.ch3_wave_position = 0;

		    uint8_t length_data = read_byte(C3_TIMER_REGISTER);
    		apu.audio_length_3 = 256 - length_data;

		    memory[AUDIO_MASTER_REGISTER] |= 0x04;

    		apu.ch3_triggered = false;
		}
		if(apu.ch4_triggered){
    		apu.audio_cycles_4 = 0;
		    uint8_t length_data = read_byte(C4_TIMER_REGISTER) & 0x3F;
    		apu.audio_length_4 = 64 - length_data;

	    	memory[AUDIO_MASTER_REGISTER] |= 0x08;
			uint8_t nrx2 = read_byte(C4_VOLUME_REGISTER);
            apu.ch4_envelope_volume = nrx2 >> 4;
            apu.ch4_envelope_timer = nrx2 & 0x07;
	    	apu.ch4_triggered = false;
}

		uint16_t freq_data_1 = (read_byte(C1_HIGH_REGISTER) & 0x07) << 8 | read_byte(C1_LOW_REGISTER);
		uint16_t freq_data_2 = (read_byte(C2_HIGH_REGISTER) & 0x07) << 8 | read_byte(C2_LOW_REGISTER);
		uint16_t freq_data_3 = (read_byte(C3_HIGH_REGISTER) & 0x07) << 8 | read_byte(C3_LOW_REGISTER);
		uint16_t freq_data_4 = read_byte(C4_CONTROL_REGISTER);

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

		int16_t amplitude_1 = apu.ch1_envelope_volume * 20;
		int16_t amplitude_2 = apu.ch2_envelope_volume * 20; 
		
		uint16_t wave_addr = 0xFF30 + (apu.ch3_wave_position / 2);
		uint8_t wave_byte = read_byte(wave_addr);

		uint8_t sample_4bit = (apu.ch3_wave_position % 2 == 0) ? (wave_byte >> 4) : (wave_byte & 0x0F);

        uint8_t vol_shift_code = (read_byte(C3_OUTPUT_REGISTER) >> 5) & 0x03;
		int16_t amplitude_3 = 0;
		if(vol_shift_code > 0){
			amplitude_3 = sample_4bit >> (vol_shift_code - 1);
		}


		int16_t amplitude_4 = apu.ch4_envelope_volume * 20;

       int curr = 0;
        uint8_t master_status = read_byte(AUDIO_MASTER_REGISTER);

        if(master_status & 0x80){
            if((master_status & 0x01) && (read_byte(C1_VOLUME_REGISTER) & 0xF8) != 0){
                curr += wave_state_1 ? amplitude_1 : -amplitude_1;
            }
            if ((master_status & 0x02) && (read_byte(C2_VOLUME_REGISTER) & 0xF8) != 0){
                curr += wave_state_2 ? amplitude_2 : -amplitude_2;
            }
            if((master_status & 0x04) && (read_byte(C3_DAC_REGISTER) & 0x80) != 0){
                curr += amplitude_3;
            }
            if((master_status & 0x08) && (read_byte(C4_VOLUME_REGISTER) & 0xF8) != 0){
                curr += apu.ch4_wave_state ? amplitude_4 : -amplitude_4;
            }
        } 

		buffer[i] = curr;

        apu.audio_cycles_1 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_2 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_3 -= CPU_CYCLES_PER_SAMPLE;
		apu.audio_cycles_4 -= CPU_CYCLES_PER_SAMPLE;
    	
	}
}
