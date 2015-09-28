/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

#include "main.h"
#include "string_stuff.h"
#include "file_io.h"

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

void print_error(string error_message){
    cout<<"Error: "<<error_message<<"\n";
}

Options::Options(){
    key_passwords_location_windows="";
    key_passwords_location_linux="";
}

bool Options::load(){
    cout<<"Reading android-prep-options\n";

    ifstream file("android-prep-options");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            boost::algorithm::trim(line);

            if(boost::algorithm::starts_with(line,"windows:")){
                boost::algorithm::erase_first(line,"windows:");

                key_passwords_location_windows=line;
            }
            else if(boost::algorithm::starts_with(line,"linux:")){
                boost::algorithm::erase_first(line,"linux:");

                key_passwords_location_linux=line;
            }
        }
    }
    else{
        print_error("Failed to open android-prep-options for reading options");

        file.close();
        file.clear();

        return false;
    }

    file.close();
    file.clear();

    if(key_passwords_location_windows.length()==0 || key_passwords_location_linux.length()==0){
        print_error("Failed to load options");

        return false;
    }

    return true;
}

int main(int argc,char* args[]){
    File_IO file_io;
    String_Stuff string_stuff;
    Options options;

    //Can this even happen?
    if(argc<=0){
        print_error("Did not receive the program name");

        return 1;
    }
    else if(argc!=2){
        string program_name=args[0];

        cout<<program_name<<" - prepare the android/ directory of a Cheese Engine project for building\n";
        cout<<"Usage: "<<program_name<<" PROJECT-DIRECTORY\n";

        return 0;
    }

    string project_directory=args[1];

    if(project_directory.length()==0){
        print_error("The PROJECT-DIRECTORY argument has a length of 0");

        return 1;
    }

    if(boost::algorithm::ends_with(project_directory,"/") || boost::algorithm::ends_with(project_directory,"\\")){
        project_directory.erase(project_directory.begin()+project_directory.length()-1);
    }

    if(!boost::filesystem::is_directory(project_directory)){
        print_error("No such directory: "+project_directory);

        return 1;
    }

    if(!options.load()){
        return 1;
    }

    vector<string> key_passwords=get_key_passwords(options);

    if(key_passwords.size()!=2){
        print_error("Incorrect number of key passwords");

        return 1;
    }

    string android_directory=project_directory+"/development/android";

    cout<<"Populating assets/ directory\n";

    file_io.remove_directory(android_directory+"/assets");
    file_io.create_directory(android_directory+"/assets");
    file_io.copy_file(project_directory+"/save_location.cfg",android_directory+"/assets/save_location.cfg");

    //Android always needs the save location to be set to home, so we will make sure
    //it is set to that here
    if(!replace_in_file(android_directory+"/assets/save_location.cfg","local","home")){
        return 1;
    }

    file_io.create_directory(android_directory+"/assets/data");
    for(boost::filesystem::recursive_directory_iterator dir(project_directory+"/data"),end;dir!=end;dir++){
        string new_location=dir->path().string();
        correct_slashes(&new_location);

        string remove_string=project_directory+"/data/";
        correct_slashes(&remove_string);

        boost::algorithm::erase_first(new_location,remove_string);

        boost::filesystem::copy(dir->path(),android_directory+"/assets/data/"+new_location);
    }

    cout<<"Creating asset lists\n";

    if(!create_asset_lists(android_directory+"/assets/data")){
        return 1;
    }

    cout<<"Copying prebuilt shared libraries\n";

    vector<string> abis;
    abis.push_back("armeabi");
    abis.push_back("armeabi-v7a");
    abis.push_back("x86");

    for(size_t i=0;i<abis.size();i++){
        string abi=abis[i];

        file_io.remove_directory(android_directory+"/jni/"+abi);

        file_io.create_directory(android_directory+"/jni/"+abi);

        string library_location="/home/tails/build-server";
        #ifdef GAME_OS_WINDOWS
            library_location="C:/Development/c++";
        #endif

        file_io.copy_file(library_location+"/cheese-engine/development/android/obj/local/"+abi+"/libSDL2.a",android_directory+"/jni/"+abi+"/libSDL2.a");
        file_io.copy_file(library_location+"/cheese-engine/development/android/obj/local/"+abi+"/libSDL2_image.a",android_directory+"/jni/"+abi+"/libSDL2_image.a");
        file_io.copy_file(library_location+"/cheese-engine/development/android/obj/local/"+abi+"/libSDL2_mixer.a",android_directory+"/jni/"+abi+"/libSDL2_mixer.a");
        file_io.copy_file(library_location+"/cheese-engine/development/android/obj/local/"+abi+"/libRakNet.a",android_directory+"/jni/"+abi+"/libRakNet.a");
        file_io.copy_file(library_location+"/cheese-engine/development/android/obj/local/"+abi+"/libCheese-Engine.a",android_directory+"/jni/"+abi+"/libCheese-Engine.a");
    }

    cout<<"Creating symlinks to development libraries\n";

    file_io.remove_file(android_directory+"/jni/include/SDL2");
    file_io.remove_file(android_directory+"/jni/include/SDL2_image");
    file_io.remove_file(android_directory+"/jni/include/SDL2_mixer");
    file_io.remove_file(android_directory+"/jni/include/RakNet");
    file_io.remove_file(android_directory+"/jni/include/boost");
    file_io.remove_file(android_directory+"/jni/include/cheese-engine");
    file_io.remove_file(android_directory+"/jni/include");

    file_io.create_directory(android_directory+"/jni/include");

    #ifdef GAME_OS_WINDOWS
        string windows_android_dir=android_directory;
        boost::algorithm::replace_all(windows_android_dir,"/","\\");
        string mklink="mklink /D "+windows_android_dir+"\\jni\\include\\SDL2 C:\\Development\\c++\\android\\SDL2";
        system(mklink.c_str());
        mklink="mklink /D "+windows_android_dir+"\\jni\\include\\SDL2_image C:\\Development\\c++\\android\\SDL2_image";
        system(mklink.c_str());
        mklink="mklink /D "+windows_android_dir+"\\jni\\include\\SDL2_mixer C:\\Development\\c++\\android\\SDL2_mixer";
        system(mklink.c_str());
        mklink="mklink /D "+windows_android_dir+"\\jni\\include\\RakNet C:\\Development\\c++\\android\\raknet\\raknet";
        system(mklink.c_str());
        mklink="mklink /D "+windows_android_dir+"\\jni\\include\\boost C:\\Development\\c++\\boost";
        system(mklink.c_str());
        mklink="mklink /D "+windows_android_dir+"\\jni\\include\\cheese-engine C:\\Development\\c++\\cheese-engine";
        system(mklink.c_str());
    #endif

    #ifdef GAME_OS_LINUX
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2",android_directory+"/jni/include/SDL2");
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2_image",android_directory+"/jni/include/SDL2_image");
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2_mixer",android_directory+"/jni/include/SDL2_mixer");
        boost::filesystem::create_symlink("/home/tails/build-server/android/raknet/raknet",android_directory+"/jni/include/RakNet");
        boost::filesystem::create_symlink("/home/tails/build-server/linux-x86_64/boost",android_directory+"/jni/include/boost");
        boost::filesystem::create_symlink("/home/tails/build-server/cheese-engine",android_directory+"/jni/include/cheese-engine");
    #endif

    cout<<"Preparing .properties files\n";

    file_io.remove_file(android_directory+"/ant.properties");
    file_io.remove_file(android_directory+"/local.properties");

    string properties_platform="windows";

    #ifdef GAME_OS_LINUX
        properties_platform="linux";
    #endif

    file_io.copy_file(android_directory+"/properties/"+properties_platform+"/ant.properties",android_directory+"/ant.properties");
    file_io.copy_file(android_directory+"/properties/"+properties_platform+"/local.properties",android_directory+"/local.properties");

    if(!replace_in_file(android_directory+"/ant.properties","STORE_PASSWORD",key_passwords[0],true)){
        return 1;
    }
    if(!replace_in_file(android_directory+"/ant.properties","ALIAS_PASSWORD",key_passwords[1],true)){
        return 1;
    }

    return 0;
}

void correct_slashes(string* str_input){
    boost::algorithm::replace_all(*str_input,"\\","/");
}

vector<string> get_key_passwords(const Options& options){
    vector<string> key_passwords;

    string keypass_file=options.key_passwords_location_windows;

    #ifdef GAME_OS_LINUX
        keypass_file=options.key_passwords_location_linux;
    #endif

    cout<<"Reading key passwords from "<<keypass_file<<"\n";

    ifstream file(keypass_file.c_str());

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            boost::algorithm::trim(line);

            if(line.length()>0){
                key_passwords.push_back(line);
            }
        }
    }
    else{
        print_error("Failed to open "+keypass_file+" for reading key passwords");
    }

    file.close();
    file.clear();

    return key_passwords;
}

bool create_asset_lists(string directory){
    if(!create_asset_list(directory)){
        return false;
    }

    for(File_IO_Directory_Iterator it(directory);it.evaluate();it.iterate()){
        if(it.is_directory()){
            string file_name=it.get_file_name();

            boost::algorithm::trim(file_name);

            if(!create_asset_lists(directory+"/"+file_name)){
                return false;
            }
        }
    }

    return true;
}

bool create_asset_list(string directory){
    string assets="";

    File_IO file_io;

    for(File_IO_Directory_Iterator it(directory);it.evaluate();it.iterate()){
        if(it.is_regular_file()){
            string file_name=it.get_file_name();

            boost::algorithm::trim(file_name);

            if(file_name.length()>0){
                assets+=file_name+"\n";
            }
        }
    }

    return file_io.save_file(directory+"/asset_list",assets);
}

bool replace_in_file(string filename,string target,string replacement,bool hide_replacement){
    if(!boost::filesystem::exists(filename)){
        print_error("No such file: "+filename);

        return false;
    }

    string terminal_replacement=replacement;
    if(hide_replacement){
        terminal_replacement="REDACTED";
    }

    cout<<"Renaming all occurrences of "<<target<<" to "<<terminal_replacement<<" in "<<filename<<"\n";

    vector<string> file_data;

    ifstream file(filename.c_str());

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open "+filename+" for updating (input phase)");

        file.close();
        file.clear();

        return false;
    }

    file.close();
    file.clear();

    for(int i=0;i<file_data.size();i++){
        boost::algorithm::replace_all(file_data[i],target,replacement);
    }

    ofstream file_save(filename.c_str());

    if(file_save.is_open()){
        for(int i=0;i<file_data.size();i++){
            file_save<<file_data[i];

            if(i<file_data.size()-1){
                file_save<<"\n";
            }
        }
    }
    else{
        print_error("Failed to open "+filename+" for updating (output phase)");

        file_save.close();
        file_save.clear();

        return false;
    }

    file_save.close();
    file_save.clear();

    return true;
}
