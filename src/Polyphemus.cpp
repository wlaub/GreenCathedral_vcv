#include "TechTechTechnologies.hpp"
#define N 3
#define MIN(A,B) ((A<B)? A : B)
#define MAX(A,B) ((A>B)? A : B)
#define CLIP(A, B, C) MIN(MAX(A,B),C)

typedef struct
{
    float data[2] = {0};
    float r; //The filter radius
    float p; //The filter angle
    float a; //iir coefficient
    float b; //iir coefficient
    int head = 0;
} biquad;



struct Polyphemus : Module {
	enum ParamIds {
        NORM_PARAM,
        NORMCV_PARAM,
        STAB_PARAM,
        STABCV_PARAM,
        GAIN_PARAM,
        RADIUS_PARAM = GAIN_PARAM+N+1,
        ANGLE_PARAM = RADIUS_PARAM+N+1,
        RADIUSCV_PARAM = ANGLE_PARAM+N+1,
        ANGLECV_PARAM = RADIUSCV_PARAM+N+1,
		NUM_PARAMS = ANGLECV_PARAM+N+1
	};
	enum InputIds {
        NORM_INPUT,
        STAB_INPUT,
		SIGNAL_INPUT,
        RADIUS_INPUT = SIGNAL_INPUT+N+1,
        ANGLE_INPUT = RADIUS_INPUT+N+1,
		NUM_INPUTS = ANGLE_INPUT+N+1
	};
	enum OutputIds {
        SIGNAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    int ready = 0;

    Label* testLabel;

    biquad filters[N];

	Polyphemus() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

/* IIR coefficients
A = 2*r*cos(theta)
B = r^2
*/

void Polyphemus::step() {
//  float deltaTime = 1.0 / engineGetSampleRate();

    if(ready == 0) return;

    float x, y;
    float r, a;
    float gain;

    float norm = CV_ATV_VALUE(NORM, 1, 0);
    float stab = CV_ATV_VALUE(STAB, 1, 0);
    float rglob = CV_ATV_VALUE(RADIUS, 1, N);
    float aglob = CV_ATV_VALUE(ANGLE, 3.14, N);

    norm = CLIP(0, norm, 1);

    float maxrad = 1 + stab*.001;

    gain = params[GAIN_PARAM].value;

    x = inputs[SIGNAL_INPUT].value*gain;

    float g = 1;

    for(int j = 0; j < N; ++j)
    { 

        //retrieve pole params from inputs
        //radius is -1 ~ 1, angle is 0 ~ 3.14
        //inputs are 0 ~ 10 w/ attenuverters
        r = params[RADIUS_PARAM+j].value*maxrad
          + params[RADIUSCV_PARAM+j].value*inputs[RADIUS_INPUT+j].value*maxrad/10;
        a = params[ANGLE_PARAM+j].value
          + params[ANGLECV_PARAM+j].value*inputs[ANGLE_INPUT+j].value*3.14/10;

        r += rglob;
        a += aglob;

        //clip to +/- 1
        r = CLIP(-maxrad, r, maxrad);
        a = CLIP(0, a, 6.28);

        filters[j].r = r;
        filters[j].p = a;
        //Set filter params from inputs

        float c = cos(a);

        filters[j].a = -2*r*c;
        filters[j].b = r*r;
    }

    //Compute the total inverse gain at each filter frequency 
    //and store the smallest value in g
    g = -1;
    for(int i = 0; i < N; ++i)
    {
        float p = filters[i].p;  //the angle of concern
        
        float tg = 1;
        for(int j = 0; j < N; ++j)
        {
            a = filters[j].a;
            float b = filters[j].b;
            float c = cos(p);
            float k = (a+2*b*c);
            tg *= (1-b)*(1-b)+2*(1-b)*c*k+k*k;

        }
        if(tg < g || g<0)
        {
            g = tg;
        }
    }
    if(g == 0)
    {
        g = 1E-6;
    }

    g = sqrt(g);

    g = (1-norm)+norm*g;

    x*=g;

    //apply filter to value
    for(int j = 0; j <N; ++j)
    {
        y = x;
        y -= filters[j].a*filters[j].data[filters[j].head];
        filters[j].head ^= 1;
        y -= filters[j].b*filters[j].data[filters[j].head];

        filters[j].data[filters[j].head] = y;
        x = y;

    }


/*
            char tstr[256];
//            sprintf(tstr, "%f, %f, %f", r, a, g);
            sprintf(tstr, "%f", norm);
            if(testLabel)
                testLabel->text = tstr;
*/

    float clip = 100;

    if(x > clip)
    {
        x = clip;
    }
    else if(x<-clip)
    {
        x = -clip;
    }

    outputs[SIGNAL_OUTPUT].value = x;
    //set output to value*gain

}


PolyphemusWidget::PolyphemusWidget() {
	Polyphemus *module = new Polyphemus();
	setModule(module);
	box.size = Vec(18* RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Polyphemus.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


    float xoff, yoff;

    xoff = 17.5;
    yoff = 380-302.5-25;

    addInput(createInput<PJ301MPort>(
        Vec(xoff, yoff), module, Polyphemus::SIGNAL_INPUT
        ));


    addParam(createParam<RoundBlackKnob>(
        Vec(xoff+28, yoff-6.5), module, Polyphemus::GAIN_PARAM,
        0, 2, 1
        ));



    xoff = 89;

    addOutput(createOutput<PJ301MPort>(
        Vec(xoff, yoff), module, Polyphemus::SIGNAL_OUTPUT
        ));



    for(int j = 0; j < N; ++j)
    {
        xoff = 17.5;
        yoff = 380-232-25+j*85;

        addInput(createInput<PJ301MPort>(
            Vec(xoff, yoff), module, Polyphemus::RADIUS_INPUT+j
            ));

        addParam(createParam<RoundTinyBlackKnob>(
            Vec(xoff+34, yoff+2.5), module, Polyphemus::RADIUSCV_PARAM+j,
            -1,1,0
            ));
 
        addParam(createParam<RoundBlackKnob>(
            Vec(xoff+62.5, yoff-14), module, Polyphemus::RADIUS_PARAM+j,
            -1,1,0
            ));


        addInput(createInput<PJ301MPort>(
            Vec(xoff, yoff+28), module, Polyphemus::ANGLE_INPUT+j
            ));

        addParam(createParam<RoundTinyBlackKnob>(
            Vec(xoff+34, yoff+2.5+28), module, Polyphemus::ANGLECV_PARAM+j,
            -1,1,0
            ));
 
        addParam(createParam<RoundBlackKnob>(
            Vec(xoff+62.5, yoff-14+43), module, Polyphemus::ANGLE_PARAM+j,
            0,3.14,0
            ));



    }

    xoff = 152.5;
    yoff = 380-302.5-25;

    CV_ATV_PARAM(xoff, yoff, Polyphemus::NORM, 0,1,0,0)

    yoff += 53;

    CV_ATV_PARAM(xoff, yoff, Polyphemus::STAB, -1,1,0,0)

    yoff += 53;

    CV_ATV_PARAM(xoff, yoff, Polyphemus::RADIUS, -1,1,0,N)

    yoff += 53;

    CV_ATV_PARAM(xoff, yoff, Polyphemus::ANGLE, 0,3.14,0,N)



/*
    addInput(createInput<PJ301MPort>(
        Vec(xoff, yoff), module, Polyphemus::RADIUS_INPUT+j
        ));

    addParam(createParam<RoundTinyBlackKnob>(
        Vec(xoff+34, yoff+9), module, Polyphemus::RADIUSCV_PARAM+j,
        -1,1,0
        ));

    addParam(createParam<RoundBlackKnob>(
        Vec(xoff+62.5, yoff+6.5), module, Polyphemus::RADIUS_PARAM+j,
        -1,1,0
        ));
*/


    auto* label = new Label();
    label->box.pos=Vec(0, 30);
    label->text = "";
    addChild(label); 
    module->testLabel = label;

    module->ready = 1;

}
