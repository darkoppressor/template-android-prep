/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

#include "file_io.h"

#include <fstream>
#include <iostream>

using namespace std;

string File_IO::load_file(string path){
    string file_data="";

    ifstream file(path.c_str());

    if(file.is_open()){
        file>>file_data;
    }

    file.close();
    file.clear();

    return file_data;
}

bool File_IO::save_file(string path,string data){
    ofstream file(path.c_str());

    if(file.is_open()){
        file<<data;

        file.close();
        file.clear();
    }
    else{
        file.close();
        file.clear();

        cout<<"Error opening file '"<<path<<"' for saving\n";

        return false;
    }

    return true;
}

bool File_IO::directory_exists(string path){
    return boost::filesystem::exists(path);
}

bool File_IO::file_exists(string path){
    return boost::filesystem::exists(path);
}

bool File_IO::is_directory(string path){
    return boost::filesystem::is_directory(path);
}

bool File_IO::is_regular_file(string path){
    return boost::filesystem::is_regular_file(path);
}

void File_IO::create_directory(string path){
    boost::filesystem::create_directory(path);
}

void File_IO::remove_file(string path){
    boost::filesystem::remove(path);
}

void File_IO::remove_directory(string path){
    boost::filesystem::remove_all(path);
}

string File_IO::get_file_name(string path){
    boost::filesystem::path boost_path(path);
    return boost_path.filename().string();
}

File_IO_Directory_Iterator::File_IO_Directory_Iterator(string get_directory){
    it=boost::filesystem::directory_iterator(get_directory);
}

bool File_IO_Directory_Iterator::evaluate(){
    if(it!=boost::filesystem::directory_iterator()){
        return true;
    }
    else{
        return false;
    }
}

void File_IO_Directory_Iterator::iterate(){
    it++;
}

bool File_IO_Directory_Iterator::is_directory(){
    if(boost::filesystem::is_directory(it->path())){
        return true;
    }
    else{
        return false;
    }
}

bool File_IO_Directory_Iterator::is_regular_file(){
    if(boost::filesystem::is_regular_file(it->path())){
        return true;
    }
    else{
        return false;
    }
}

string File_IO_Directory_Iterator::get_full_path(){
    return it->path().string();
}

string File_IO_Directory_Iterator::get_file_name(){
    return it->path().filename().string();
}

string File_IO_Directory_Iterator::get_file_extension(){
    return it->path().extension().string();
}
