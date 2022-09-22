#ifndef NOO_MML_H
#define NOO_MML_H

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/sound.h"

namespace noo {

namespace audio {

class Sample;
struct Sample_Instance;

class NOOSKEWL_SHIM_EXPORT MML : public Sound {
public:
	static void static_start();
	static void static_stop();
	static void pause_all();
	static int mix(float *buf, int samples, bool sfx_paused);

	MML(std::string filename, bool load_from_filesystem = false);
	virtual ~MML();

	void play(float volume, bool loop); // Sound interface
	void play(bool loop); // plays at SFX volume. This is part of the Sound interface
	bool is_done(); // also Sound interface
	void stop(); // Sound interface
	void pause();
	void set_master_volume(float volume);
	float get_master_volume();
	std::string get_name(); // returns same thing passed to constructor
	bool is_playing();

	void set_pause_with_sfx(bool pause_with_sfx);

private:
	class Internal {
	public:
		struct Wav_Start {
			int sample;
			Uint32 play_start;
			Uint32 length;
			Sample_Instance *instance;
			float volume;
		};

		Internal(std::string filename, bool load_from_filesystem);
		~Internal();

		void set_mml(MML *mml);
		MML *get_mml();

		void set_master_volume(float master_volume);
		float get_master_volume();
	
		void set_pause_with_sfx(bool pause_with_sfx);

		class Track
		{
		public:
			enum Type {
				PULSE = 0,
				NOISE,
				SAWTOOTH,
				SINE,
				TRIANGLE,
				NOISE_ORIG,
			};

			// pad is # of samples of silence to pad the end with so all tracks are even
			Track(Type type, std::string text, std::vector< std::pair<int, float> > &volumes, std::vector< std::pair<int, float> > &volume_offsets, std::vector<int> &pitches, std::vector<int> &pitch_offsets, std::vector< std::vector<float> > &pitch_envelopes, std::vector< std::vector<float> > &pitch_offset_envelopes, std::vector< std::pair<int, float> > &dutycycles, int pad, std::vector<Sample *> wav_samples, std::vector<Wav_Start> wav_starts);
			~Track();

			void play(bool loop);
			void stop();
			void pause();
			int update(float *buf, int length);

			bool is_playing();
			bool is_done();

			void set_master_volume(float master_volume, float master_volume_samples);
			float get_master_volume();

			void set_pause_with_sfx(bool pause_with_sfx);
			bool pause_with_sfx();

		private:
			void reset(Uint32 buffer_fulfilled);

			void generate(float *buf, int samples, int t, const char *tok, int octave);

			void real_get_frequency(int index, std::vector< std::vector<float> > &v, float zero_freq, float default_frequency, float &ret_freq, float &ret_time, float &ret_len, float &last_freq, float &last_start, int &same_sections);
			void get_frequency(float start_freq, float &ret_freq, float &ret_time, float &ret_len);
			void get_frequency_offset(float &ret_freq, float &ret_time, float &ret_len);
			float real_get_volume(int &section, std::vector< std::pair<int, float> > &v);
			float get_volume();
			float get_dutycycle();
			void start_wavs(Uint32 buffer_offset, Uint32 on_or_after);
			void stop_wavs();
			void set_sample_volumes(float volume);

			std::string next_note(const char *text, int *pos);
			int notelength(const char *tok, const char *text, int *pos);

			Type type;
			std::string text;
			std::vector< std::pair<int, float> > volumes;
			std::vector< std::pair<int, float> > volume_offsets;
			std::vector<int> pitches;
			std::vector<int> pitch_offsets;
			std::vector< std::vector<float> > pitch_envelopes;
			std::vector< std::vector<float> > pitch_offset_envelopes;
			std::vector< std::pair<int, float> > dutycycles;

			int pad;
			int sample;
			bool reset_time;
			int curve_volume;
			int curve_pitch;
			int curve_duty;
			float dutycycle;
			int octave;
			int note_length;
			float volume;
			int tempo;
			int note;
			int volume_section;
			int volume_offset_section;
			int dutycycle_section;
			int pos;
			std::string tok;
			int length_in_samples;
			int note_fulfilled;
			bool done;
			bool padded;
			bool loop;
			bool playing;
			int t;
			float last_freq;
			float last_start;
			int same_sections;
			float last_freq_o;
			float last_start_o;
			int same_sections_o;
			float master_volume;
			float master_volume_samples;
			float mix_volume;
			float last_noise;
			float last_noise2;
			float remain;
			bool fading;
			float prev_time;
			float lead;
			std::vector<Sample *> wav_samples;
			std::vector<Wav_Start> wav_starts; // <sample index, sample to start at>
			int wav_sample;
			bool _pause_with_sfx;
		};

		std::vector<Track *> tracks;

		MML *mml;
	
		std::vector<Sample *> wav_samples;
	};

	static std::vector<Internal *> loaded_mml;

	Internal *internal;

	std::string name;
};

void NOOSKEWL_SHIM_EXPORT play_music(std::string name);
void NOOSKEWL_SHIM_EXPORT pause_music();
void NOOSKEWL_SHIM_EXPORT stop_music();

} // End namespace audio

} // End namespace noo

#endif // NOO_MML_H
