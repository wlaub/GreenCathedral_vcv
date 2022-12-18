#include "rack.hpp"

#pragma once

using namespace rack;

//Params

struct GCTrimerKnob : SvgKnob {
    GCTrimerKnob();
};

//Lights

template <typename TBase = GrayModuleLightWidget>
struct TOrangeLight : TBase {
    TOrangeLight() {
        this->addBaseColor(nvgRGBAf(1, .33, 0, 1));
    }
};
using OrangeLight = TOrangeLight<>;

static const float KNOB_SENSITIVITY = 0.0015f;



