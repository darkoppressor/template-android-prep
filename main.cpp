#include "main.h"
#include "string_stuff.h"
#include "file_io.h"

#include <iostream>
#include <vector>
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

    if(!options.load()){
        return 1;
    }

    vector<string> key_passwords=get_key_passwords(options);

    if(key_passwords.size()!=2){
        print_error("Incorrect number of key passwords in the key passwords file");

        return 2;
    }

    string build_scripts_platform="windows";

    #ifdef GAME_OS_LINUX
        build_scripts_platform="linux";
    #endif

    file_io.remove_directory("assets");
    file_io.create_directory("assets");
    boost::filesystem::copy_file("../../save_location.cfg","assets/save_location.cfg");

    file_io.create_directory("assets/data");
    for(boost::filesystem::recursive_directory_iterator dir("../../data"),end;dir!=end;dir++){
        string new_location=dir->path().string();

        boost::algorithm::erase_all(new_location,"../");
        boost::algorithm::erase_all(new_location,"data\\");

        boost::filesystem::copy(dir->path(),"assets/data/"+new_location);
    }

    create_asset_list("assets/data");
    create_asset_list("assets/data/images");
    create_asset_list("assets/data/music");
    create_asset_list("assets/data/sounds");

    file_io.remove_file("jni/SDL2");
    file_io.remove_file("jni/SDL2_image");
    file_io.remove_file("jni/SDL2_mixer");
    file_io.remove_file("jni/RakNet");
    file_io.remove_file("jni/boost");

    #ifdef GAME_OS_WINDOWS
        system("mklink /D jni\\SDL2 C:\\Development\\c++\\android\\SDL2");
        system("mklink /D jni\\SDL2_image C:\\Development\\c++\\android\\SDL2_image");
        system("mklink /D jni\\SDL2_mixer C:\\Development\\c++\\android\\SDL2_mixer");
        system("mklink /D jni\\RakNet C:\\Development\\c++\\android\\raknet\\raknet");
        system("mklink /D jni\\boost C:\\Development\\c++\\boost");
    #endif

    #ifdef GAME_OS_LINUX
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2","jni/SDL2");
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2_image","jni/SDL2_image");
        boost::filesystem::create_symlink("/home/tails/build-server/android/SDL2_mixer","jni/SDL2_mixer");
        boost::filesystem::create_symlink("/home/tails/build-server/android/raknet/raknet","jni/RakNet");
        boost::filesystem::create_symlink("/home/tails/build-server/linux-x86_64/boost","jni/boost");
    #endif

    update_source_file_list();

    file_io.remove_file("ant.properties");
    file_io.remove_file("local.properties");

    boost::filesystem::copy_file("build-scripts/"+build_scripts_platform+"/ant.properties","./ant.properties");
    boost::filesystem::copy_file("build-scripts/"+build_scripts_platform+"/local.properties","./local.properties");

    replace_in_file("ant.properties","STORE_PASSWORD",key_passwords[0]);
    replace_in_file("ant.properties","ALIAS_PASSWORD",key_passwords[1]);

    return 0;
}

vector<string> get_key_passwords(const Options& options){
    vector<string> key_passwords;

    string keypass_file=options.key_passwords_location_windows;

    #ifdef GAME_OS_LINUX
        keypass_file=options.key_passwords_location_linux;
    #endif

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

    file.close();
    file.clear();

    return key_passwords;
}

void create_asset_list(string directory){
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

    file_io.save_file(directory+"/asset_list",assets);
}

void update_source_file_list(){
    string source_files="";

    File_IO file_io;

    for(File_IO_Directory_Iterator it("../..");it.evaluate();it.iterate()){
        if(it.is_regular_file()){
            string file_name=it.get_file_name();

            boost::algorithm::trim(file_name);

            if(file_name.length()>0 && it.get_file_extension()==".cpp"){
                source_files+="../../../../"+file_name+" \\\n";
            }
        }
    }

    boost::algorithm::erase_last(source_files," \\");

    file_io.remove_file("jni/src/Android.mk");
    boost::filesystem::copy_file("jni/src/Android.mk.template","jni/src/Android.mk");

    replace_in_file("jni/src/Android.mk","SOURCE_FILE_LIST_GOES_HERE",source_files);
}

void rename_file(string target,string replacement){
    boost::filesystem::rename(target,replacement);
}

void replace_in_file(string file,string target,string replacement){
    string temp=file+".tmp";
    rename_file(file,temp);

    ifstream load(temp.c_str());
    ofstream save(file.c_str());

    if(load!=0 && save!=0){
        while(!load.eof()){
            string line="";

            getline(load,line);

            boost::algorithm::replace_first(line,target,replacement);

            save<<line<<"\n";
        }
    }

    load.close();
    load.clear();

    save.close();
    save.clear();

    boost::filesystem::remove(temp);
}
