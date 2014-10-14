#ifndef main_h
#define main_h

#include <vector>
#include <string>

void print_error(std::string error_message);

class Options{
public:

    std::string key_passwords_location_windows;
    std::string key_passwords_location_linux;

    Options();

    bool load();
};

int main(int argc,char* args[]);

std::vector<std::string> get_key_passwords(const Options& options);

void create_asset_list(std::string directory);

void update_source_file_list();

void rename_file(std::string target,std::string replacement);

void replace_in_file(std::string file,std::string target,std::string replacement);

#endif
