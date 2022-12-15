
#include "TechTechTechnologies.hpp"
#include "PngModule.hpp"

struct QMod : PngModule {
    /* +ENUMS */
    #include "QMod_enums.hpp"
    /* -ENUMS */
    
    /* +TRIGGER_VARS */
    #include "QMod_vars.hpp"
    /* -TRIGGER_VARS */

    float phase[4] = {0,0,0,0};

    QMod() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        /* +ENUMS */
        #include "QMod_paramconfig.hpp"
        /* -ENUMS */
        
    }

    void process(const ProcessArgs& args) override {
        float deltaTime = args.sampleTime;


        /*  +INPUT_PROCESSING */
        #include "QMod_inputs.hpp"
        /*  -INPUT_PROCESSING */

        float i_clk[4];
        float q_clk[4];

        //TODO: Set clock frequency default to quarter sample rate
        for(int i = 0; i < 4; ++i)
        {
            float pitch = input_freq[i];
            double dp = 2*M_PI*261.626f * powf(2.0f, pitch) * deltaTime;
            dp = std::min(dp, M_PI);

            phase[i] += dp;
            if(phase[i] > 2*M_PI)
            {
                phase[i] -= 2*M_PI;
            }
        }

        //I carrier
        if(inputs[INPUT_AUX_I].active)
        {
            for(int i = 0; i < 4; ++i)
            {
                i_clk[i] = input_aux_i;
            }
        }
        else
        {
            for(int i = 0; i < 4; ++i)
            {
                i_clk[i] = sin(phase[i]);
            }
        }

        //Q carrier
        if(inputs[INPUT_AUX_Q].active)
        {
            for(int i = 0; i < 4; ++i)
            {
                q_clk[i] = input_aux_q;
            }
        }
        else
        {
            for(int i = 0; i < 4; ++i)
            {
                q_clk[i] = cos(phase[i]);
            }
        }
        //Modulation

        for(int i = 0; i < 4; ++i)    
        {
            input_q_in[i] *= param_in_gain[i];
            input_i_in[i] *= param_in_gain[i];
            output_mod_out[i] =   i_clk[i]*input_i_in[i]
                                + q_clk[i]*input_q_in[i];
        }

        //Demodulation

        for(int i = 0; i < 4; ++i)
        {
            if(!inputs[INPUT_MOD_IN+i].active)
            {
                input_mod_in[i] = output_mod_out[i];
            }
            input_mod_in[i] *= param_in_gain[i];
            output_i_out[i] = input_mod_in[i] * i_clk[i];
            output_q_out[i] = input_mod_in[i] * q_clk[i];
        }

        /*  +OUTPUT_PROCESSING */
        #include "QMod_outputs.hpp"
        /*  -OUTPUT_PROCESSING */



    }



    // For more advanced Module features, read Rack's engine.hpp header file
    // - dataToJson, dataFromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};



struct QModWidget : PngModuleWidget {

    void appendContextMenu(Menu* menu) override {
            QMod* module = dynamic_cast<QMod*>(this->module);

            menu->addChild(new MenuEntry);

            panel_select_menu(menu, module);

        }


    QModWidget(QMod *module) {
		setModule(module);
        box.size = Vec(18.0 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        set_panels(
            {
            {"Default", "res/QMod.png"},
            {"Fancy", "res/qmod_a.png"},
            });



        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        /* +CONTROL INSTANTIATION */
        #include "QMod_panel.hpp"
        /* -CONTROL INSTANTIATION */
    }
};

#include "QMod_instance.hpp"

