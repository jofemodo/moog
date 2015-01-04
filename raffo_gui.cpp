#include <gtkmm.h>
#include <lv2gui.hpp>
#include "raffo.peg"

using namespace sigc;
using namespace Gtk;

class RaffoSynthGUI : public LV2::GUI<RaffoSynthGUI> {
public:
  	
  	
  	
	RaffoSynthGUI(const std::string& URI) {
		Table* interfaz = manage(new Table(1, 4)); // tabla que contiene toda la interfaz
		Table* osciladores = manage(new Table(5, 4)); // subtablas
		Table* modificadores = manage(new Table(8, 3));
		Table* params = manage(new Table(4,1));
		
		interfaz->set_border_width(12);
		interfaz->set_spacings(6);
		
		osciladores->attach(*manage(new Label("Range")), 1, 2, 0, 1);
		osciladores->attach(*manage(new Label("Waveform")), 2, 3, 0, 1);
		osciladores->attach(*manage(new Label("Volume")), 3, 4, 0, 1);
		osciladores->attach(*manage(new Label("Osc 0")), 0, 1, 1, 2);
		osciladores->attach(*manage(new Label("Osc 1")), 0, 1, 2, 3);
		osciladores->attach(*manage(new Label("Osc 2")), 0, 1, 3, 4);
		osciladores->attach(*manage(new Label("Osc 3")), 0, 1, 4, 5);
		
		for (int i=0; i<4; i++) {
			range[i] = manage(new HScale(m_ports[m_range0 + i].min, m_ports[m_range0+i].max, 1));
			wave[i] = manage(new HScale(m_ports[m_wave0 + i].min, m_ports[m_wave0+i].max, 1));
			vol[i] = manage(new HScale(m_ports[m_vol0 + i].min, m_ports[m_vol0+i].max, 0.01));
			
			range[i]->set_size_request(100, 50);
			wave[i]->set_size_request(100, 50);
			vol[i]->set_size_request(100, 50);
			
			range[i]->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_range0 + i), mem_fun(*range[i], &HScale::get_value)));
		 	
			wave[i]->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_wave0 + i), mem_fun(*wave[i], &HScale::get_value)));
			
			vol[i]->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_vol0 + i), mem_fun(*vol[i], &HScale::get_value)));
			
			//range[i]->set_draw_value(false);
			
			osciladores->attach(*range[i], 1, 2, 1 + i, 2 + i);
			osciladores->attach(*wave[i], 2, 3, 1 + i, 2 + i);
			osciladores->attach(*vol[i], 3, 4, 1 + i, 2 + i);
			   	
		}
		
		modificadores->attach(*manage(new Label("Filter")), 0, 3, 0, 1);
		modificadores->attach(*manage(new Label("Cutoff")), 0, 1, 1, 2);
		modificadores->attach(*manage(new Label("Attack")), 0, 1, 3, 4);
		modificadores->attach(*manage(new Label("Decay")), 1, 2, 3, 4);
		modificadores->attach(*manage(new Label("Sustain")), 2, 3, 3, 4);
		modificadores->attach(*manage(new Label("Loudness")), 0, 3, 5, 6);
		modificadores->attach(*manage(new Label("Attack")), 0, 1, 6, 7);
		modificadores->attach(*manage(new Label("Decay")), 1, 2, 6, 7);
		modificadores->attach(*manage(new Label("Sustain")), 2, 3, 6, 7);
		
		filter_cutoff = manage(new HScale(m_ports[m_filter_cutoff].min, m_ports[m_filter_cutoff].max, 0.01));
		filter_attack = manage(new HScale(m_ports[m_filter_attack].min, m_ports[m_filter_attack].max, 0.01));
		filter_decay = manage(new HScale(m_ports[m_filter_decay].min, m_ports[m_filter_decay].max, 0.01));
		filter_sustain = manage(new HScale(m_ports[m_filter_sustain].min, m_ports[m_filter_sustain].max, 0.01));
		attack = manage(new HScale(m_ports[m_attack].min, m_ports[m_attack].max, 0.01));
		decay = manage(new HScale(m_ports[m_decay].min, m_ports[m_decay].max, 0.01));
		sustain = manage(new HScale(m_ports[m_sustain].min, m_ports[m_sustain].max, 0.01));
		
		
		
		filter_cutoff->set_size_request(100, -1);
		filter_attack->set_size_request(100, -1);
		filter_decay->set_size_request(100, -1);
		filter_sustain->set_size_request(100, -1);
		attack->set_size_request(100, -1);
		decay->set_size_request(100, -1);
		sustain->set_size_request(100, -1);
		
		filter_cutoff->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_filter_cutoff), mem_fun(*filter_cutoff, &HScale::get_value)));
		filter_attack->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_filter_attack), mem_fun(*filter_attack, &HScale::get_value)));
		filter_decay->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_filter_decay), mem_fun(*filter_decay, &HScale::get_value)));
		filter_sustain->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_filter_sustain), mem_fun(*filter_sustain, &HScale::get_value)));
		attack->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_attack), mem_fun(*attack, &HScale::get_value)));
		decay->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_decay), mem_fun(*decay, &HScale::get_value)));
		sustain->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_sustain), mem_fun(*sustain, &HScale::get_value)));
		
		modificadores->attach(*filter_cutoff, 0, 1, 2, 3);
		modificadores->attach(*filter_attack, 0, 1, 4, 5);
		modificadores->attach(*filter_decay, 1, 2, 4, 5);
		modificadores->attach(*filter_sustain, 2, 3, 4, 5);
		modificadores->attach(*attack, 0, 1, 7, 8);
		modificadores->attach(*decay, 1, 2, 7, 8);
		modificadores->attach(*sustain, 2, 3, 7, 8);
		
		
		glide = manage(new VScale(m_ports[m_glide].min, m_ports[m_glide].max, 0.01));
		glide->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_glide), mem_fun(*glide, &HScale::get_value)));
		glide->set_inverted(true);
		
		volume = manage(new VScale(m_ports[m_volume].min, m_ports[m_volume].max, 0.01));
		volume->signal_value_changed().connect(compose(bind<0>(mem_fun(*this, &RaffoSynthGUI::write_control), m_volume), mem_fun(*volume, &HScale::get_value)));
		volume->set_inverted(true);
		
		Frame* f_glide = new Frame("Glide");
		f_glide->add(*glide);
		Frame* f_osc = new Frame("Oscillators");
		f_osc->add(*osciladores);
		Frame* f_mod = new Frame("Modifiers");
		f_mod->add(*modificadores);
		Frame* f_vol = new Frame("Volume");
		f_vol->add(*volume);
		
		Gdk::Color wood;
		wood.set_rgb(2000, 1000, 500);
		
		Gdk::Color black;
		black.set_rgb(60000, 60000, 60000);
		
		
		/*
		Gdk::Color text;
		text.set_red(60000);
		text.set_green(60000);
		text.set_blue(60000);
		*/
		f_glide->modify_bg(STATE_NORMAL, black);
		f_osc->modify_bg(STATE_NORMAL, black);
		f_mod->modify_bg(STATE_NORMAL, black);
		f_vol->modify_bg(STATE_NORMAL, black);
		
		/*Image* bg_img = new Image("Wood-Texture.jpg");
		Gdk::Pixbuf* bg_img = Gdk::Pixbuf::create_from_file("Wood-Texture.jpg");
		interfaz->attach(manage(*bg_img));
		*/

		interfaz->attach(*f_glide, 0, 1, 0, 1);
		interfaz->attach(*f_osc, 1, 2, 0, 1);
		interfaz->attach(*f_mod, 2, 3, 0, 1);
		interfaz->attach(*f_vol, 3, 4, 0, 1);
		
		EventBox* fondo = new EventBox();
		fondo->modify_bg(STATE_NORMAL, wood);
		
		//fondo->add_pixlabel("Wood-Texture.jpg", "bg_image", 0);
		fondo->add(*interfaz);		
		add(*fondo);
		
		// tocar midi con el teclado
//		grab_focus();
//		signal_key_pressed_event().connect(mem_fun(*this, gtk_main_quit),);
		

	}
    
  
	void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
		switch (port) {
			case (m_wave0): {
				wave[0]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_wave1):{
				wave[1]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_wave2):{
				wave[2]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_wave3):{
				wave[3]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_range0): {
				range[0]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_range1): {
				range[1]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_range2): {
				range[2]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_range3): {
				range[3]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_vol0): {
				vol[0]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_vol1): {
				vol[1]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_vol2): {
				vol[2]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_vol3): {
				vol[3]->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_attack): {
				attack->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_decay): {
				decay->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_sustain): {
				sustain->set_value(*static_cast<const float*>(buffer));
				break;
			}
			/*case (m_release): {
				vol[3]->set_value(*static_cast<const float*>(buffer));
				break;
			}*/
			case (m_filter_cutoff): {
				filter_cutoff->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_filter_attack): {
				filter_attack->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_filter_decay): {
				filter_decay->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_filter_sustain): {
				filter_sustain->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_volume): {
				volume->set_value(*static_cast<const float*>(buffer));
				break;
			}
			case (m_glide): {
				glide->set_value(*static_cast<const float*>(buffer));
				break;
			}
		}
	}

	protected:

		HScale* range[4];

		HScale* wave[4];

		HScale* vol[4];

		HScale* filter_cutoff;
		HScale* filter_attack;
		HScale* filter_decay;
		HScale* filter_sustain;

		HScale* attack;
		HScale* decay;
		HScale* sustain;
		
		VScale* glide;
		VScale* volume;
		
		//static char* format_value(GtkScale *scale, int value){
         //   return str("-->%d<--", value);}

		
	};


	static int _ = RaffoSynthGUI::register_class("http://example.org/raffo/gui");