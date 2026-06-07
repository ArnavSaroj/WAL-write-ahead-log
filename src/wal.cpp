#include<fstream>
#include<stdexcept>
#include "../include/wal.h"
#include<iostream>

using namespace std;

bool wal::WAL::recover(){
    return true;
}

bool wal::WAL::flush(){
    file_.flush();
    return true;
}

wal::WAL::~WAL(){
    flush();

    if(file_.is_open()){
        file_.close();
    }
}

wal::WAL::WAL(const string&file_path):file_path_(file_path){

file_.open(file_path_,ios::binary|ios::in|ios::out);

if(!file_.is_open()){
    cout<<"file doestn exists creating new file"<<endl;
    ofstream create_file(file_path_,  std::ios::binary);

    create_file.close();

    file_.open(file_path_,ios::binary|ios::in|ios::out);

}

if(!file_.is_open()){
    throw runtime_error("failed to open wal files");

}
recover();

}
int main(){
    wal::WAL wal("test.wal");
}

