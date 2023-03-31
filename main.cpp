#include <iostream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <sys/vfs.h>
#include <linux/magic.h>

using namespace std;
// Types found here
// https://docs.rust-embedded.org/rust-sysfs-gpio/libc/
// DOS
// FAT
// FAT32
// EXT2
// EXT3
// EXT4
// NTFS
string prompt = "cwushell";
/*
 * Handles 'exit' command
*/
void exitHandler(vector<string> args){
    if(args.size() == 1){
        exit(0);
    }
    else if(args.size() == 2){
        int exitCode = stoi(args[1]);
        exit(exitCode);
    }else{
        cout << "Too Many Arguments" << endl;
    }
}
/*
 * Handles 'prompt' command
*/
void promptHandler(vector<string> args){
    if(args.size() == 1){
        prompt = "cwushell";
    }else if (args.size() == 2){
        prompt = args[1];
    }else{
        cout << "Too Many Arguments" << endl;
    }
}
/*
 * Helper function for 'fileinfo'
 *
 * Uses stat.h to get the file type, only checks for Directoty, Regular File, Symbolic Link, and Local Socket
*/
string getType(int fd){
    struct stat buf;
    int ret = fstat(fd, &buf);
    if(ret < 0){
        cout << "Bad File descriptor" << endl;
    }
    string output = "";
    if(S_ISDIR(buf.st_mode)) output = "Directory";
    if(S_ISREG(buf.st_mode)) output = "Regular File";
    if(S_ISLNK(buf.st_mode)) output = "Symbolic Link";
    if(S_ISSOCK(buf.st_mode)) output = "Local Socket";

    return output;
}
/*
 * Handles arguments for 'fileinfo' command 
*/
void handleArg(string arg, string filename){
    int fd = open(filename.c_str(), O_RDONLY); // file descriptor

    if ( fd < 0 ) {
        // no such file or directory
        cout << "No such file or directory" << endl;
        return;
    }
    struct stat buf;
    int ret;
 
    ret = fstat(fd, &buf);
    if (ret < 0) {
        cout << "Bad File descriptor" << endl;
        return;
    }
    int inode = buf.st_ino;
    string lastModified = ctime(&buf.st_ctime);
    string type = getType(fd);

    if(arg == "-i"){
        cout << "Inode: " <<inode << endl;
    }else if (arg == "-t"){
        cout << "Type: " << getType(fd) << endl;
    }else if (arg == "-m"){
        cout << "Last Modified: " << lastModified << flush;
    }else if (arg == "-it" || arg == "-ti"){
        cout << "Inode: " <<inode << endl;
        cout << "Type: " << getType(fd) << endl;
    }else if (arg == "-im" || arg == "-mi"){
        cout << "Inode: " <<inode << endl;
        cout << "Last Modified: " << lastModified << flush;
    }else if (arg == "-tm" || arg == "-mt"){
        cout << "Type: " << getType(fd) << endl;
        cout << "Last Modified: " << lastModified << flush;
    }else if (arg == "-itm" || arg == "-imt" || arg == "-tmi" || arg == "-tim" || arg == "-mti" || arg == "-mit"){
        cout << "Inode: " <<inode << endl;
        cout << "Type: " << getType(fd) << endl;
        cout << "Last Modified: " << lastModified << flush;
    }
    else{
        cout << "invalid flag '" << arg << "'" << endl;
    }
}
/*
 * Handles 'fileinfo' command
 * Uses stat.h to query information about given file
 * 
 * More info about stat.h can be found here {{ https://www.ibm.com/docs/en/i/7.3?topic=ssw_ibm_i_73/apis/stat.html }} 
*/
void fileinfo(vector<string> args){
    string fileName = args[args.size()-1];
    if(args.size() < 2){
        cout << "fileinfo: missing operand" << endl;
        return;
    }
    int fd = open(args[args.size()-1].c_str(), O_RDONLY); // file descriptor

    if ( fd < 0 ) {
        // no such file or directory
        cout << "No such file or directory" << endl;
        return;
    }
    struct stat buf;
    int ret;
 
    ret = fstat(fd, &buf);
    if (ret < 0) {
        cout << "Bad File descriptor" << endl;
        return;
    }
    int inode = buf.st_ino;
    //string type = buf.st_size;
    string lastModified = ctime(&buf.st_ctime);
    if(args.size() == 2){
        cout << "Inode: " << inode << endl;
        cout << "Type: " << getType(fd) << endl;
        cout << "Last Modified: " << lastModified << flush;
    }else if (args.size() <= 5){
        for(int i = 1; i < args.size() - 1; i++){
            handleArg(args[i], fileName);
        }
        
    }else{
        cout << "Too Many Arguments" << endl;
    }
    
}
/*
 * Hanldes 'osifo' command
*/
void osinfo(vector<string> args){
    // -T print the type of file system
    // -b print the total numer of blocks in the file system
    // -s print the maximum number of characters what can be used in a filename
    const char* path = "."; // change this to the path you want to retrieve file system information for

    struct statfs buf;
    if (statfs(path, &buf) == -1)
    {
        std::cerr << "Error retrieving file system information." << std::endl;
        return;
    }
    int type = buf.f_type;
    if(type == 0){
        cout << "File System: NTFS" << endl;
    }else if(type == 4){
        cout << "File System: FAT" << endl;
    }else if (type == 11){
        cout << "File System: FAT32" << endl;
    }else if (type == 17){
        cout << "File System: DOS" << endl;
    }else if (type == 61265){
        cout << "File System: EXT2" << endl;
    }else if (type == 61266){
        cout << "File System: EXT3" << endl;
    }else if (type == 61267){
        cout << "File System: EXT4" << endl;
    }
    
}
/*
 * Handles user input
*/
void handleUserInput(vector<string> args){
    if(args[0] == "exit"){
        exitHandler(args);
    }else if(args[0] == "prompt"){
        promptHandler(args);
    }else if(args[0] == "fileinfo"){
        fileinfo(args);
    }else if(args[0] == "osinfo"){
        osinfo(args);
    }else{
        string command = "";
        for(string s : args)
            command += (s + " ");
        system(command.c_str());
    }
}
/*
 * Tokenizes given string into vector of strings
*/
vector<string> tokenize(string userInput){
    vector<string> args;
    string temp;
    for(int i = 0; i < userInput.length(); i++){
        if(userInput[i]==' '){
            if(temp != ""){
                if(find(args.begin(), args.end(), temp) == args.end()){
                    args.push_back(temp);
                }
                temp = "";
            }
        }else{
            temp += userInput[i];
        }
    }

    if(find(args.begin(), args.end(), temp) == args.end()){
        args.push_back(temp);
    }
    
    return args;
}
void start(){
    while(true){
        // Take in user input
        
        string userInput;
        cout << prompt << "> " << flush;
        getline (cin, userInput);
        // tokenize user input
        vector<string> args = tokenize(userInput);
        handleUserInput(args);
    }
}
int main(){
    start();
}

