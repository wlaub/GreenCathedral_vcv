#include "utils.hpp"

//PathMemory

void PathMemory::update(const char* path)
{
    char* tpath = strdup(path);
    dir = dirname(tpath);
    free(tpath);
    
    tpath = strdup(path);
    file = basename(tpath);
    free(tpath);
}

json_t* PathMemory::to_json()
{
    json_t* result = json_object();
    json_object_set_new(result, "dir", json_string(dir.c_str()));
    json_object_set_new(result, "file", json_string(file.c_str()));
    return result;
}

void PathMemory::from_json(json_t* rootJ)
{
    json_t* temp;
    temp = json_object_get(rootJ, "dir");
    if(temp) dir = json_string_value(temp);

    temp = json_object_get(rootJ, "file");
    if(temp) file = json_string_value(temp);
}

std::string PathMemory::get_path()
{
    if (dir.empty() && file.empty())
    {
        return "";
    }
    std::string result = dir + "/" + file;
    
    return result;
}

char* PathMemory::file_dialog(osdialog_file_action action, const char* defdir, const char* deffile, const char* dialog_filter_spec)
{
    osdialog_filters* dialog_filter = 0;
    if(dialog_filter_spec)
    {
        dialog_filter = osdialog_filters_parse(dialog_filter_spec);
    }

    char* result = osdialog_file(action, 
        dir.empty() ? defdir:dir.c_str(), 
        file.empty() ? deffile:file.c_str(),
        dialog_filter
        );

    if(dialog_filter)
    {
        osdialog_filters_free(dialog_filter);
    }

    if(result)
        update(result);

    return result;
}


