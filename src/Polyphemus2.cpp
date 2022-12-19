#include "GreenCathedral.hpp"
#include "PngModule.hpp"

//The cutoff frequency gain
#define Hc .707
//The maximum pole order
#define Nmax 7

#define MAX_CHANNELS 16


struct PoleFilter {

    double mem = 0;

    double get_y(double x, double r, double k)
    {
        double y = x*k+r*mem;
        mem = y;
        return y;
    }

};

struct Polyphemus2 : PngModule {
	enum ParamIds {
		ORDER_PARAM,
		ENVP_PARAM,
		GAIN_PARAM,
		KNEE_PARAM,
		VOCTP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENV_INPUT,
		IN0_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		VOCT0_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT0_OUTPUT,
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    struct PoleFilter filters[3][MAX_CHANNELS][Nmax];

    float rvals[MAX_CHANNELS] = {0};
    float kvals[MAX_CHANNELS] = {0};

	Polyphemus2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(ORDER_PARAM, 1.f, Nmax , 1.f, "Pole Order");
        configParam(ENVP_PARAM, 0.f, 2.f, 1.f, "Envelope Gain");
        configParam(GAIN_PARAM, 0.f, 19.f, 1.f, "Frequency gain above knee");
        configParam(KNEE_PARAM, 0.f, 10.f, 5.f, "Envelope knee level", " V");
        configParam(VOCTP_PARAM, -54.f, 54.f, 0.f, "Cutoff frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);

        configInput(VOCT0_INPUT, "Cutoff Frequency (v/oct)");
        configInput(ENV_INPUT, "Envelope");
        for(int j = 0; j < 3; ++ j)
        {
            configBypass(IN0_INPUT+j, OUT0_OUTPUT+j);
            configInput(IN0_INPUT+j, "Filter");
            configOutput(OUT0_OUTPUT+j, "Filter");
        }
	}

    double get_k(double r, double N)
    {
        return 1-r;
        return pow(1-r, floor(N));
    }

    double get_gain(double w, double r)
    {
        return 1/sqrt(r*r-2*r*cos(w)+1);
    }


    double get_r(double wc, double N)
    {
        //Kc must be 0-1 gain^2 at cutoff frequency 
        double kc = 1/pow(Hc, 2.f/floor(N));
        double a = 1-kc; //a = c
        double b = 2*(kc-cos(wc));
        if(b > 2*a)
        {
            return (-b + sqrt(b*b-4*a*a))/(2*a);
        }
        else
        {
            return 0;
        }
    }


	void process(const ProcessArgs &args) override {

        bool pane_active = false;
        int pane_in_channels = 0;
        for(int j = 0; j < 3; ++j)
        {
            pane_active |= inputs[IN0_INPUT+j].active;
            pane_in_channels = std::max(pane_in_channels, inputs[IN0_INPUT+j].getChannels());
        }

        if(pane_active)
        {


            int voct_channels = std::min(inputs[VOCT0_INPUT].getChannels(), MAX_CHANNELS);
            int env_channels = std::min(inputs[ENV_INPUT].getChannels(), MAX_CHANNELS);
            int pane_channels = std::max(voct_channels, env_channels);
            pane_channels = std::max(pane_channels, 1);

            //Don't need to evaluate channels that don't have a corresponding input signal
            int clamp_channels = std::min(pane_in_channels, pane_channels); 
            float N = params[ORDER_PARAM].getValue();

            float knee = params[KNEE_PARAM].getValue()/10.f;
            for(int c = 0; c < clamp_channels; ++c)
            {
                int ec = std::min(c,env_channels-1);
                int vc = std::min(c,voct_channels-1);
                float t = 1.f;
                if(inputs[ENV_INPUT].active)
                {
                    t = inputs[ENV_INPUT].getVoltage(ec)/10.f;
                    t *= params[ENVP_PARAM].getValue();

                    if(t <= knee) 
                    {
                        t /= knee;
                    }
                    else
                    {
                        t = 1+(t-knee)/(1-knee)*params[GAIN_PARAM].getValue();
                    }
                }

                float pitch = params[VOCTP_PARAM].getValue()/12.f;
                pitch += inputs[VOCT0_INPUT].getVoltage(vc);
                float freq =  dsp::FREQ_C4 * std::pow(2.f, pitch) * t;
                float wc = 6.28*freq*args.sampleTime;
                wc = clamp(wc, 0.f, 3.14f);

                rvals[c] = get_r(wc, N);
                kvals[c] = get_k(rvals[c], N);
            }

//            printf("%f: %f, %f\n", wc, r, k);
            for(int j = 0; j < 3; ++j)
            {
                int channels = inputs[IN0_INPUT+j].getChannels();
                if(channels > MAX_CHANNELS)
                {
                    channels = MAX_CHANNELS;
                }
                for(int c = 0; c < channels; ++c)
                {
                    float y = 0;
                    if(inputs[IN0_INPUT+j].active)
                    {
                        float x = inputs[IN0_INPUT+j].getVoltage(c);
                        int pc = std::min(c,clamp_channels-1);
                        double r = rvals[pc];
                        double k = kvals[pc];
                        for(int n = 0; n < Nmax; ++n)
                        {
                            x = filters[j][c][n].get_y(x,r,k);
                            if(n == (floor(N) -1))
                            {
                                y = x;
                            }
                        }
                    }
                    outputs[OUT0_OUTPUT+j].setVoltage(y, c);
                }
                outputs[OUT0_OUTPUT+j].setChannels(channels);
            }
        }

	}
};


#define GRIDX(x) 15.24*(x+0.5)+5.135
#define GRIDY(y) 15.24*(y)+3.28
#define GRID(x,y) Vec(GRIDX(x), GRIDY(y))

struct OrderKnob : RoundBlackKnob {
    OrderKnob() {
        snap = true;
        minAngle = -M_PI*6/7;
        maxAngle = M_PI*6/7;
    }
};


struct Polyphemus2Widget : PngModuleWidget {

    void draw(const DrawArgs& args)
    {
//        PngModuleWidget::draw(args);

        nvgSave(args.vg);
        nvgBeginPath(args.vg);

        float rmin = 7.62;
        float cx = box.size.x/2;


        nvgArc(args.vg, cx, 50, 100, 0, M_PI, NVG_CCW);
//        nvgArcTo(args.vg, 0, 50, 50, 50, 100);
//        nvgCircle(args.vg, 25, 25, 100);

        nvgStrokeColor(args.vg, nvgRGB(0,0,0));
        nvgStrokeWidth(args.vg, 2);
        nvgStroke(args.vg);

        nvgClosePath(args.vg);
        nvgRestore(args.vg);
               

    } 

	Polyphemus2Widget(Polyphemus2 *module) {
		setModule(module);

        init_panels("Polyphemus2");

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


       addParam(createParamCentered<RoundBlackKnob>(
            mm2px(GRID(3,1.5)), module, Polyphemus2::GAIN_PARAM));
       addParam(createParamCentered<RoundBlackKnob>(
            mm2px(GRID(1.5,1)), module, Polyphemus2::KNEE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            mm2px(GRID(3,3)), module, Polyphemus2::ENVP_PARAM));

        addParam(createParamCentered<RoundBlackKnob>(
            mm2px(GRID(0,3)), module, Polyphemus2::VOCTP_PARAM));
        addParam(createParamCentered<OrderKnob>(
            mm2px(GRID(0,1.5)), module, Polyphemus2::ORDER_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(GRID(0.5,4)), module, Polyphemus2::VOCT0_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(GRID(2.5,4)), module, Polyphemus2::ENV_INPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(GRID(0.5,5)), module, Polyphemus2::IN0_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(GRID(0.5,6)), module, Polyphemus2::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(GRID(0.5,7)), module, Polyphemus2::IN2_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(GRID(2.5,5)), module, Polyphemus2::OUT0_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(GRID(2.5,6)), module, Polyphemus2::OUT1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(GRID(2.5,7)), module, Polyphemus2::OUT2_OUTPUT));
    }

};


Model *modelPolyphemus2 = createModel<Polyphemus2, Polyphemus2Widget>("Polyphemus2");
