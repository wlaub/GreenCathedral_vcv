#include "rack.hpp"
#include <vector>

using namespace rack;

enum class ImageType {SVG, PNG};

typedef std::tuple<std::string, std::string> PanelInfo;

struct MyPanel {
    std::string path;
    std::string label;
    ImageType type;

    int png_handle = 0;
    NSVGimage* svg_handle = 0;

    float width = 12.f*RACK_GRID_WIDTH;

    MyPanel(PanelInfo config);

    void draw(const ModuleWidget::DrawArgs& args, float w, float h);

    float get_width();

};

struct MyPanelCache {

    float width = 0;
    MyPanel* default_panel = 0;
    std::vector<struct MyPanel*> panel_options;
    std::map<std::string, MyPanel*> panel_map;

    void set_panels(const std::vector<PanelInfo> panels);
    void add_panel(PanelInfo config);
    void find_default_panel(const char* default_label=0);

};

struct PngModule : Module {

    MyPanelCache* panel_cache;
    MyPanel* current_panel;
    bool show_panel_labels = false;

};

typedef std::map<std::string, MyPanelCache*> PanelCacheMap;

struct PngModuleWidget : ModuleWidget {

    /* 1536000013 58% 17 90 2.1-768 */
    /* TODO:
        save/load panel options from json
        make default panel a json option?
        make defaults do anything
        save/load panel selection
    */

    MyPanel* current_panel = 0;

    std::string slug = "";

    static PanelCacheMap panel_cache_map;
    MyPanelCache* panel_cache = 0;

    void load_panels_from_json();
    void init_panels();
    void set_panels(const std::vector<PanelInfo> panels);
    void _init_instance_panels();

    void setModel(plugin::Model* model);

    void draw(const DrawArgs& args) override;

    void panel_select_menu(Menu* menu, PngModule* module);

    json_t* config_to_json();
    void config_from_json(json_t* rootJ);


};


