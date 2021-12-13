#include <iostream>
#include <string>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <mutex>
#include <sys/mman.h>
#include <condition_variable>
#include <thread>


using namespace std;

queue<string> files;
vector<int> pref;

mutex mutex_files;
mutex mutex_stdout;

string search_str = "FILE";

int fd = 1;

condition_variable cv;


struct Psearch
{
    string pattern = "";
    string dir = "";
    int threads_amount = 1;
    bool recursive = true;
};

struct Psearch args_init(int argc, char ** argv) {
    struct Psearch args;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-n")) {
            args.recursive = false;
            continue;
        }
        if (argv[i][0] == '-' and argv[i][1] == 't') {
            sscanf(argv[i], "-t%d", &args.threads_amount);
            continue;
        }
        if (argv[i][0] == '/') {
            args.dir = argv[i];
            continue;
        }
        args.pattern = argv[i];   
    }
    
    if (args.dir == "") {
        char _dir[4096];
        getcwd(_dir, 4096);
        args.dir = _dir;
    }

    return args;
}

vector <int> pref_init(string pattern) {
	int n = pattern.length();
	vector<int> ans (n);
	for (int i = 1; i < n; i++) {
		int j = ans[i-1];
		while (j > 0 && pattern[i] != pattern[j])
			j = ans[j-1];
		if (pattern[i] == pattern[j]) j++;
		ans[i] = j;
	}
	return ans;
}



void files_from_dir (string dir_name, bool rec) {

    DIR* dir = opendir(dir_name.c_str());
	if (dir == nullptr) return;
	for (dirent* de = readdir(dir); de != NULL; de = readdir(dir)) {
		if (strcmp(de->d_name, ".") == 0 or strcmp(de->d_name, "..") == 0) continue;
        
        string ss = dir_name + "/" + de->d_name;
        
        if( de-> d_type == DT_REG){
            mutex_files.lock();
            files.push(ss);
            cv.notify_one();
            mutex_files.unlock();       
        }
        
        if (de->d_type == DT_DIR and rec == true) files_from_dir(ss, rec);
	}
	closedir(dir);

}

void queue_filler (string dir, bool recursive ) {
    files_from_dir(dir, recursive);
    files.push("/");
    cv.notify_one();
    return;
}

void sercher(string path) {
    string s = search_str;
    int n = s.size();

    int pos = 0;
	size_t begin = 0;
	int line = 1;
	char * buf;
	int file;

    file = open(path.c_str(), O_RDONLY);
    if(file < 0){
		mutex_stdout.lock();
		printf("can not open %s \n", path.c_str());
		mutex_stdout.unlock();
        return;
	}

    struct stat bufff;
    fstat(file, &bufff);
    size_t siz = bufff.st_size;
    buf = (char*)mmap(NULL, siz, PROT_READ, MAP_PRIVATE, file, 0);

	for(size_t i = 0; i < siz; ++i){
		
        if( buf[i] == '\n') line ++, begin = i + 1, pos = 0;

		while(pos > 0 && s[pos] != buf[i]) pos = pref[pos-1];

		if (s[pos] == buf[i]) ++pos;

		if(pos == n) {
            while(i < siz and buf[i] !='\n') ++i;

            mutex_stdout.lock();

            char Intstr[10];
            sprintf(Intstr,"%d", line);
            string s = "[found] line " +string(Intstr) + " from '"+path.c_str()+"':\n";
            write(fd, s.c_str(), s.length());
            write(fd, ">>", 2);
            write(fd, &buf[begin], i - begin);
            write(fd, "<<\n", 3);
            mutex_stdout.unlock();
            
            line ++, begin = i + 1, pos = 0;
		}
	
	}

    munmap(buf, siz);
	close(file);
    return;
}

void file_manager (){
    string s = "";
    while(true){
        unique_lock<mutex> ul(mutex_files);
        cv.wait(ul, []{return !files.empty();});   
        
        s = files.front();
        if (s == "/") {
            cv.notify_one();
            return;
        }
        files.pop();
        if (files.size() > 0) cv.notify_one();
        
        ul.unlock();
        sercher(s);  
    }
    return;
}



int main(int argc, char ** argv) {
    struct Psearch args = args_init(argc, argv);
    search_str = args.pattern;

    pref = pref_init(args.pattern);
    
    fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    dup2(fd, 1);


    thread a(queue_filler, args.dir, args.recursive);
    
    vector<thread> searchers(args.threads_amount);
    for(int i = 0; i < args.threads_amount; ++i){
        searchers[i] = thread(file_manager);
    }

    a.join();
    for(int i = 0; i < args.threads_amount; ++i) searchers[i].join();
    
    dup2(1,1);
    close(fd);

	return 0;
}