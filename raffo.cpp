#include <math.h>
#include "raffo.peg"
#include <lv2plugin.hpp>
#include <lv2_event_helpers.h>
#include <lv2_uri_map.h>
#include <lv2types.hpp>
#include <stdlib.h>

#include <iostream>

using namespace std;

static const unsigned char INVALID_KEY = 255;

static inline float key2hz(unsigned char key) {
  return 8.1758 * pow(1.0594, key);
}

class RaffoSynth : public LV2::Plugin<RaffoSynth, LV2::URIMap<true> > //LV2::Synth<RaffoVoice, RaffoSynth> 
{
  
protected:
  /*float*& p(uint32_t port) {
    return reinterpret_cast<float*&>(Parent::m_ports[port]);
  }*/
  
	double sample_rate;
	unsigned char key;
	uint32_t period;
	uint32_t counter;
	uint32_t subcounter;
	int envelope_count;
	float modwheel;
	float pitch;
	float pitch_width; // esto lo ponemos aca o en la gui?
	
	uint32_t midi_type;

public:
	typedef LV2::Plugin<RaffoSynth, LV2::URIMap<true> > Parent;
	
	RaffoSynth(double rate): 
		Parent(m_n_ports),
		sample_rate(rate),
		key(INVALID_KEY),
		period(500),
		counter(0),
		pitch(1),
		pitch_width(12)
		{
		 midi_type = Parent::uri_to_id(LV2_EVENT_URI, "http://lv2plug.in/ns/ext/midi#MidiEvent"); 
		}
		 
		 
  void render(uint32_t from, uint32_t to) {
    if (key == INVALID_KEY) return;
    
    // buffer en 0
    for (uint32_t i = from; i < to; ++i) p(m_output)[i] = 0;
    
    // osciladores
	int envelope_subcount;
	for (int osc = 0; osc < 4; osc++) {    
		subcounter = counter;
		envelope_subcount = envelope_count;
		float vol = pow(*p(m_volume) * *p(m_vol0 + osc) / 100., 2); // el volumen es el cuadrado de la amplitud
		int subperiod = period / pow(2, *p(m_range0 + osc)); // periodo efectivo del oscilador
		
		// valores precalculados para el envelope
		// la función de envelope es:
			// f(t) = s - (1-s)/(2*d) * (t-a-d-|t-a-d|) + (1/(2*a) + (1-s)/(2*d)) * (t-a-|t-a|)
			/*
              /\
             /  \
          	/    \_______________  -> s = sustain level
           /  
          /
          |-a-|-d-|--------------|
			*/
		float a = *p(m_attack)*100 + .1;
		float d = *p(m_decay)*100 + .1;
		float s = pow(*p(m_sustain),2);
		float c1 = (1.-s)/(2.*d);
		float c2 = 1./(2.*a);

		
		switch ((int)*p(m_wave0 + osc)) {
			case (0): { //triangular
				for (uint32_t i = from; i < to; ++i && subcounter++ && envelope_subcount++) 
					p(m_output)[i] += vol * (4. * (fabs(((subcounter + subperiod/4) % subperiod) /
					                  (float)subperiod - .5)-.25)) * 
					                  (s - c1 * (envelope_subcount - a - d - fabs(envelope_subcount - a - d)) + 
					                  (c2 + c1) * (envelope_subcount - a - fabs(envelope_subcount - a))) ;
				// zapato: la onda triangular esta hecha para que empiece continua, pero cuando se corta popea
				break;
			}
			case (1): { //sierra
				for (uint32_t i = from; i < to; ++i && subcounter++ && envelope_subcount++) 
					p(m_output)[i] += vol * (2. * ((int)(subcounter * pitch) % subperiod) / (float)subperiod - 1) * 
					                  (s - c1 * (envelope_subcount - a - d - fabs(envelope_subcount - a - d)) + 
					                  (c2 + c1) * (envelope_subcount - a - fabs(envelope_subcount - a))) ;
				break;
			}
			case (2): { //cuadrada
				for (uint32_t i = from; i < to; ++i && subcounter++ && envelope_subcount++) 
					p(m_output)[i] += vol * (2. * (((subcounter%subperiod) / (float)subperiod - .5) < 0)-1) * 
					                  (s - c1 * (envelope_subcount - a - d - fabs(envelope_subcount - a - d)) + 
					                  (c2 + c1) * (envelope_subcount - a - fabs(envelope_subcount - a))) ;
				break;
			}
			case (3): { //pulso
				for (uint32_t i = from; i < to; ++i && subcounter++ && envelope_subcount++) 
					p(m_output)[i] += vol * (2. * (((subcounter%subperiod) / (float)subperiod - .2) < 0)-1) * 
					                  (s - c1 * (envelope_subcount - a - d - fabs(envelope_subcount - a - d)) + 
					                  (c2 + c1) * (envelope_subcount - a - fabs(envelope_subcount - a))) ;
				break;
			}
			
		}
    }
    counter = subcounter;
    envelope_count += to - from;
  }
  
	void handle_midi(uint32_t size, unsigned char* data) {
	  if (size == 3) {
		  switch (data[0]) {
		    case (0x90): { // note on
			    if (key == INVALID_KEY) 
			    	envelope_count = 0;
			    key = data[1];
			    period = sample_rate * 4 / key2hz(key);
			    counter=0;
			    break;
		    }
		    case (0x80): { // note off
			    if (key == data[1])
			      key = INVALID_KEY;
			    break;
		    }
		    case (0xE0): { // pitch bend
		      subcounter = subcounter % period;
		      /* Calculamos el factor de pitch (numero por el que multiplicar 
		         la frecuencia fundamental). data[2] es el byte mas significativo, 
		         data[1] el menos. El primer bit de ambos es 0, por eso << 7. 
		         pitch_width es el numero maximo de semitonos de amplitud del pitch.
		      * Mas informacion: http://sites.uci.edu/camp2014/2014/04/30/managing-midi-pitchbend-messages/
		      */
		      pitch = pow(2.,(((data[2] << 7) ^ data[1]) / 8191 - 1) 
		               * pitch_width / 12); 
		    }	
      }
    }
  } /*handle_midi*/
  
  void run(uint32_t sample_count) {

    LV2_Event_Iterator iter;
    lv2_event_begin(&iter, reinterpret_cast<LV2_Event_Buffer*&>(Parent::m_ports[m_midi]));

    uint8_t* event_data;
    uint32_t samples_done = 0;

    while (samples_done < sample_count) {
      uint32_t to = sample_count;
      LV2_Event* ev = 0;
      if (lv2_event_is_valid(&iter)) {
        ev = lv2_event_get(&iter, &event_data);
        to = ev->frames;
        lv2_event_increment(&iter);
      }
      if (to > samples_done) {
        render(samples_done, to);
        samples_done = to;
      }

      if (ev) {
        if (ev->type == midi_type)
          static_cast<RaffoSynth*>(this)->handle_midi(ev->size, event_data);
      }
    }
  } /*run*/
  
};

static int _ = RaffoSynth::register_class(m_uri);

