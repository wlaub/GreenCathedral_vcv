#include "PngModule.hpp"
#include "TechTechTechnologies.hpp"

MyPanel::MyPanel(PanelInfo config) {
    label = std::get<0>(config);
    path = std::get<1>(config);

    if(path.find("svg") != std::string::npos)
    {
        type = ImageType::SVG;

        svg_handle = nsvgParseFromFile(
                asset::plugin(pluginInstance, path).c_str(), 
                "px", SVG_DPI);

        width = svg_handle->width;
    }
    else
    {
        type = ImageType::PNG;
    }
}

float MyPanel::get_width()
{
    return width;
}

void MyPanel::draw(const ModuleWidget::DrawArgs& args, float w, float h)
{
    if(type == ImageType::SVG)
    {
        nvgScissor(args.vg, 0,0, svg_handle->width, svg_handle->height);
        svgDraw(args.vg, svg_handle);
    }
    else
    {

        if(png_handle == 0)
        {
            png_handle = nvgCreateImage(
                args.vg,
                asset::plugin(pluginInstance, path).c_str(),
                0
            );
        }
    
        nvgSave(args.vg);
        nvgBeginPath(args.vg);
        NVGpaint png_paint = nvgImagePattern(args.vg, 0, 0, w,h, 0, png_handle, 1.0f);
        nvgRect(args.vg, 0, 0, w,h);
        nvgFillPaint(args.vg, png_paint);
        nvgFill(args.vg);
        nvgClosePath(args.vg);
        nvgRestore(args.vg);
        
 
    }

}

void MyPanelCache::add_panel(PanelInfo config)
{

    panel_options.push_back(new struct MyPanel(config));
    MyPanel* new_panel = panel_options.back();

    panel_map.emplace(new_panel->label, new_panel);
    panel_map.emplace(new_panel->path, new_panel);

    if(width == 0 && new_panel->type == ImageType::SVG)
    {
        width = new_panel->get_width();
    }


}

void MyPanelCache::find_default_panel(const char* default_label)
{/* If the default panel hasn't been set, try to guess the default panel
    from the panels that have been loaded.
    */

    if(default_label == 0)
    {
        default_label = "Default";
    }

    if(default_panel == 0)
    {
        try {
            default_panel = panel_map.at(default_label);
        } catch (const std::out_of_range& e) {
            default_panel = panel_options[0];
        }
    }
}

void MyPanelCache::set_panels(const std::vector<PanelInfo> panels)
{
    for (const PanelInfo& config : panels)
    {
        add_panel(config);
    }

    find_default_panel();
}


struct PanelMenu : MenuItem {

    PngModuleWidget* widget;
    MyPanel* current;
    int mode; //0 for current, 1 for default

    Menu* createChildMenu() override {
    
        Menu* menu = new Menu;
        struct PanelItem : MenuItem {
            PngModule* module;
            MyPanel* panel;
            int mode;
            void onAction(const event::Action& e) override {
                if(mode == 0)
                {
                    module->current_panel = panel;
                }
                else
                {
                    module->panel_cache->default_panel = panel;
                }
            }
        };

        for(auto& option : widget->panel_cache->panel_options)
        {
            PanelItem* item = createMenuItem<PanelItem>(option->label);
            item->mode = mode;
            item->module = (PngModule*)widget->module;
            item->panel = option;
            item->rightText = CHECKMARK(current==option);
            menu->addChild(item);
        }

        return menu;

    }

};

void PngModuleWidget::panel_select_menu(Menu* menu, PngModule* module)
{

    PanelMenu* panel_menu = createMenuItem<PanelMenu>("Panel", RIGHT_ARROW);
    panel_menu->widget = this;
    panel_menu->current = current_panel;
    panel_menu->mode = 0;
    menu->addChild(panel_menu);

    panel_menu = createMenuItem<PanelMenu>("Default Panel", RIGHT_ARROW);
    panel_menu->widget = this;
    panel_menu->current = panel_cache->default_panel;
    panel_menu->mode = 1;
    menu->addChild(panel_menu);


}

PanelCacheMap PngModuleWidget::panel_cache_map;

void PngModuleWidget::setModel(plugin::Model* model)
{
    printf("Hello!\n");
	assert(!this->model);
	this->model = model;    

}

void PngModuleWidget::_init_instance_panels()
{
    
    current_panel = panel_cache->default_panel;
    box.size.x = std::round(panel_cache->width / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;

    if(!module)
    {
        return;
    }
    
    PngModule* mod = dynamic_cast<PngModule*>(this->module);

    mod->panel_cache = panel_cache;
    mod->current_panel = current_panel;

}

void PngModuleWidget::load_panels_from_json()
{/* Load the panel configuration from the json file at res/panels/<slug>.json
    */
    
    std::string json_path = asset::plugin(pluginInstance, "res/panels/") + slug + ".json";

    FILE* fp = std::fopen(json_path.c_str(), "r");

    json_error_t error;
    json_t* rootJ = json_loadf(fp, 0, &error);

    if(!rootJ)
    {
        std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
        return;
    }
    
    json_t* panels = json_object_get(rootJ, "panels");
    const char* default_label = json_string_value(json_object_get(rootJ, "default"));
    const char* user_default_label = json_string_value(json_object_get(rootJ, "user_default"));

    if(user_default_label != 0)
    {
        default_label = user_default_label;
    }

    size_t index;
    json_t* config;
    json_array_foreach(panels, index, config){
        panel_cache->add_panel({
            json_string_value(json_array_get(config, 0)),
            json_string_value(json_array_get(config, 1))
            });
    }

    panel_cache->find_default_panel(default_label);

    std::fclose(fp);

}


void PngModuleWidget::init_panels()
{/* Load the panel configuration from the json file at res/panels/<slug>.json
    Create the panel cache
    */

    if(panel_cache == 0 )
    {
        try {
            panel_cache = PngModuleWidget::panel_cache_map.at(slug);
        } catch (const std::out_of_range& e) {
            //This is where the panel cache is created and populated for the first time
            PngModuleWidget::panel_cache_map.emplace(slug, new struct MyPanelCache);
            panel_cache = PngModuleWidget::panel_cache_map.at(slug);

            load_panels_from_json();

            if(panel_cache->width == 0)
            {
                panel_cache->width = 12.0f * RACK_GRID_WIDTH;
                printf("Warning: Module %s doesn't have an svg panel. Assuming 12 HP\n", slug.c_str());
            }


        }
    }

    _init_instance_panels();

}

void PngModuleWidget::set_panels(const std::vector<PanelInfo> panels)
{
    if(panel_cache == 0 )
    {
        try {
            panel_cache = PngModuleWidget::panel_cache_map.at(slug);
        } catch (const std::out_of_range& e) {
            //This is where the panel cache is created and populated for the first time
            PngModuleWidget::panel_cache_map.emplace(slug, new struct MyPanelCache);
            panel_cache = PngModuleWidget::panel_cache_map.at(slug);
            panel_cache->set_panels(panels) ;

            if(panel_cache->width == 0)
            {
                panel_cache->width = 12.0f * RACK_GRID_WIDTH;
                printf("Warning: Module %s doesn't have an svg panel. Assuming 12 HP\n", slug.c_str());
            }


        }
    }

    _init_instance_panels();

}

void PngModuleWidget::draw(const DrawArgs& args)
{
   if(module){
        PngModule* mod = dynamic_cast<PngModule*>(this->module);
        if(mod->current_panel != current_panel)
        {
            current_panel = mod->current_panel;
        }
    }
    if(current_panel)
    {
        current_panel->draw(args, box.size.x, box.size.y);
    }

    ModuleWidget::draw(args);
}

