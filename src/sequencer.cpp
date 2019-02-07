#include <cstdio>
#include <jack/jack.h>

#include "sequencer.hpp"
#include "config.hpp"
#include "jack.hpp"

int jack_callback (jack_nframes_t nframes, void *arg)
{

    Sequencer *sequencer = (Sequencer *) arg;

	int i;

    if (!sequencer->playing) return 0;

	for(i=0; i < nframes; i++)
	{
        sequencer->elapsed_samples += 1;
        if (sequencer->elapsed_samples >= sequencer->period) {
            sequencer->elapsed_samples = sequencer->elapsed_samples - sequencer->period;
            sequencer->play_current();
        }
	}

	return 0;

}

Sequencer::Sequencer(Jack jack, Osc *osc_server)
{

    osc = osc_server;

    playing = false;

    cursor = 0;
    elapsed_samples = 0;

    sample_rate = jack_get_sample_rate(jack.jack_client);

    set_bpm(Config::DEFAULT_BPM);

    jack.set_callback(jack_callback, this);
    // fprintf (stderr,"%i",sample_rate);

    const char* address = "/test";
    const char* type = "i";
    std::map<int, double> values;
    bool enabled = true;
    bool is_note = false;
    values[1] = 12;
    values[91] = 12;

    sequence_add(address, type, values, enabled, is_note);


}

Sequencer::~Sequencer()
{

}

void Sequencer::set_period(double p)
{

    period = p * sample_rate;

}

void Sequencer::set_bpm(float b)
{

    if (b > Config::MAX_BPM) {
        bpm = Config::MAX_BPM;
    } else if (b < Config::MIN_BPM) {
        bpm = Config::MIN_BPM;
    } else {
        bpm = b;
    }

    set_period(60. / bpm / Config::PPQN);

}

void Sequencer::play_current()
{

    // fprintf (stderr,"%i", cursor);
    cursor += 1;
    for (auto& item: sequences) {
        item.second.play(cursor);
    }
    // loop through sequences...

}


void Sequencer::play() {

    if (playing) {
        trig();
    } else {
        playing = true;
    }

}

void Sequencer::stop() {

    playing = false;

}

void Sequencer::trig() {

    cursor = 0;
    elapsed_samples = 0;

    if (!playing) {
        play();
    }

}

void Sequencer::sequence_add(const char* address, const char* type, std::map<int, double> values, bool enabled, bool is_note)
{

    sequences[address] = Sequence(osc, address, type, values, enabled, is_note);

}

void Sequencer::sequence_remove(const char* address)
{

    std::map<const char*, Sequence>::iterator it = sequences.find(address);
    sequences.erase(it);

}

void Sequencer::sequence_enable(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].enable();
    }

}

void Sequencer::sequence_disable(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].disable();
    }

}

void Sequencer::sequence_toggle(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].toggle();
    }

}