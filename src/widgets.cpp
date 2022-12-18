#include "GreenCathedral.hpp"
#include "widgets.hpp"

using namespace rack;

//Params

GCTrimerKnob::GCTrimerKnob() {
    setSvg(SVG::load(asset::plugin(pluginInstance, "res/Components/RoundTinyBlackKnob.svg")));
}

//Lights


