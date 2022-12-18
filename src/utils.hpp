#include <string>

#include <libgen.h>
#include <osdialog.h>

#include "rack.hpp"

//For handling dialog paths
struct PathMemory {
    std::string dir;
    std::string file;

    void update(const char* path);
    json_t* to_json();
    void from_json(json_t* rootJ);
    std::string get_path();

    char* file_dialog(osdialog_file_action action, osdialog_filters* filters);
    char* file_dialog(osdialog_file_action action, const char* defdir, const char* deffile, const char* dialog_filter_spec);
};


