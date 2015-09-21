/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

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

void correct_slashes(std::string* str_input);

std::vector<std::string> get_key_passwords(const Options& options);

bool create_asset_lists(std::string directory);

bool create_asset_list(std::string directory);

bool replace_in_file(std::string filename,std::string target,std::string replacement,bool hide_replacement=false);

#endif
