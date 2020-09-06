#include <filesystem>
#include <stack>
#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include <fstream>

using namespace std;
namespace fs = filesystem;

mutex stack_mutex, cout_mutex;
void search(stack<string> *files, string pattern) {
    auto id = this_thread::get_id();
    for(;;) {
        stack_mutex.lock();
        if(files->empty()) {
            stack_mutex.unlock();
            break;
        }
        string path = files->top();
        files->pop();
        stack_mutex.unlock();

        // actual search
        ifstream file(path);
        int line_num = 0;
        string line;
        while(!file.eof()) {
            getline(file, line);
            int pos = line.find(pattern);
            if(pos != string::npos) {
                cout_mutex.lock();
                if(line.size() < 1000)
                    cout << "line: " << line_num << "\tstarts at: " << pos << "\t" << path << '\n' << line << "\n\n";
                else 
                    cout << "line: " << line_num << "\tstarts at: " << pos << "\t" << path << "\t[Line is too big to display]" << "\n\n";
                cout_mutex.unlock();
            }
            line_num++;
        }
        file.close();
    }
}

int main(int argc, char* argv[]) {
    stack<string> files;
    int n = 1;
    bool recursive = true;
    string pattern;
    string dir;

    // parse args
    for(int i = 1; i < argc; i++) {
        string s = argv[i];
        if(s.substr(0,2) == "-t") {
            n = stoi(s.substr(2,s.size()-2));
        } else if(s == "-h" || s == "--help") {
            cout << "Usage: \n" \
                 << "-n\t\t - no recursion\n" \
                 << "-h, --help \t - display this messsage\n" \
                 << "-t# \t\t - number of threads\n" \
                 << "pattern\n" \
                 << "directory\n";
        } else if(s == "-n") 
            recursive = false;
        else if(pattern == "")
            pattern = s;
        else if(dir == "")
            dir = s;
        else {
            cout << "Wrong input, seek --help\n";
        }
    }

    // sanity check
    if(n < 1) {
        cout << "It is not possible to use " << n << " threads\n";
        return 1;
    }

    // directory check
    if(dir == "") 
        dir = fs::current_path();
    if(!fs::exists(dir)) {
        cout << "Directory " << dir << " does not exist!\n";
    }

    // add files to the stack
    if(recursive) {
        for (const auto p : fs::recursive_directory_iterator(dir)) {
            if(fs::is_regular_file(p.path())) {
                files.push(p.path());{
                // cout << p.path() << endl;
                }}}}
    else 
        for (const auto p : fs::directory_iterator(dir)) 
            if(fs::is_regular_file(p.path())) 
                files.push(p.path());

    // go
    thread *threads = new thread[n];
    // mutex m;
    for(int i = 0; i < n; i++) {
        threads[i] = thread(search, &files, pattern);
    }
    for(int i = 0; i < n; i++) {
        threads[i].join();
    }
}