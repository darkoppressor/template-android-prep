#ifndef file_io_h
#define file_io_h

#include <boost/filesystem.hpp>

#include <string>

class File_IO{
public:
    std::string load_file(std::string path);
    bool save_file(std::string path,std::string data);

    bool directory_exists(std::string path);
    bool file_exists(std::string path);
    bool is_directory(std::string path);
    bool is_regular_file(std::string path);
    void create_directory(std::string path);
    void remove_file(std::string path);
    void remove_directory(std::string path);
    std::string get_file_name(std::string path);
};

class File_IO_Directory_Iterator{
public:

    boost::filesystem::directory_iterator it;

    File_IO_Directory_Iterator(std::string get_directory);

    bool evaluate();
    void iterate();
    bool is_directory();
    bool is_regular_file();
    std::string get_full_path();
    std::string get_file_name();
    std::string get_file_extension();
};

#endif
