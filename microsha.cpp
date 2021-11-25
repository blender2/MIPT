#include <iostream>
#include <vector>
#include <string>

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <time.h>
#include <signal.h>


// using std::vector;
using namespace std;


int _time = 0;

//writes invitation to enter
void invitation() {
    char currentdir[PATH_MAX];
    if (getcwd(currentdir, PATH_MAX) == nullptr) {
        perror("Unable to get dir :(\n");
        exit(1);
    }

    char _char = '!';
    if (strcmp(getenv("USER"), "root")) _char = '>';
    printf("%s%c", (char*)currentdir, _char);
}

void print_error (string msg, vector<vector<string>> & conveer) {
    cout<<msg<< endl;
    conveer.clear();
    conveer.push_back(vector<string> {""});
}

void read_line(string & line) {
    getline(cin, line);
}

bool contains_reg(string & s) {
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == ' ')
            return false;

        if (s[i] == '?' || s[i] == '*')
            return true;  
    }
    return false;
}

bool fits_reg(string str, string pattern) {
    if (str.empty()) {
        if (pattern[0] != '\0'){
			if(pattern[0] == '*') return fits_reg(str, pattern.substr(1));
			else return false;
        }
        return true;
    }

    if (str[0] == pattern[0] || pattern[0] == '?')
        return fits_reg(str.substr(1), pattern.substr(1));
    
    if (pattern[0] == '*') { 
        for (int i = 0; i < str.size() + 1; i++)
            if (fits_reg(str.substr(i), pattern.substr(1))) return true;
    }
    return false;
}

vector<string> files_from_dir(
                              string const dirname, 
                              bool         is_dir, 
                              bool         home, 
                              bool         dot, 
                              string       reg_vv)
{
	vector<string> ret;
	DIR* dir = opendir(dirname.c_str());
	if(dir == nullptr){ 
		perror("microsha");
		return ret;
		}
	
	for( dirent* de = readdir(dir); de != NULL; de = readdir(dir)){
		bool rv = fits_reg(de->d_name, reg_vv);
		if(!dot and de->d_name[0] =='.') continue; 
		if(is_dir and de-> d_type == DT_DIR and rv ){
			if(home) ret.push_back(de->d_name + string("/"));
			else ret.push_back(dirname + de->d_name + string("/"));
			}
		else if(rv and !is_dir){
			if(home) ret.push_back(de->d_name );
			else ret.push_back(dirname + de->d_name);
			}
		}
    closedir(dir);
	return ret;
}

//returns array fitting s
vector<string> regv(string s) {
    vector<string> res;

    bool home = true;
    bool dot = false;
    int st_pos = 0; 
    int i = 0;


    if (s[0] == '/') {
        st_pos = 1;
        i = 1;
        home = false;
        res.push_back("/");
    } else res.push_back("./");

    if (s[0] == '.') dot = true;

    while(s[i] != '\0'){
		for(;s[i] != '\0' and s[i] != '/';i++); //первое вхождение / или \0
		if(s[i] == '\0') break; //если конец строки выйти
		s[i] = '\0';
        i++; 
		vector<string> buffer;
		dot = false;
		if(s[st_pos] == '.') dot = true;

		for(size_t n = 0 ; n < res.size(); n++){
			vector<string> buffer1 = files_from_dir(res[n], true, home, dot, s.substr(st_pos));
			buffer.insert(buffer.end(), buffer1.begin(), buffer1.end());
			}
		home = false;
		res = buffer;
		s[i-1] = '/';
		st_pos = i;
	}


    if(st_pos != i){
		vector <string> buffer;
		dot = false;
		if(s[st_pos] == '.') dot = true;
		for(size_t n = 0 ; n < res.size(); n++){
			vector<string> buffer1 = files_from_dir(res[n], false, home, dot, s.substr(st_pos));
            buffer.insert(buffer.end(), buffer1.begin(), buffer1.end());
			}
		home = false;
		res = buffer;
	}

	if(res.size() == 0) res.push_back(s);
	return res;
    
}

//divedes line into words by DELIM and replaces regv
void split_line(string *_line, 
                vector<string> *tokens, 
                const char * DELIM) 
{
    string token = "";
    while(_line->compare(token) != 0) {
        token = _line->substr(0, _line->find_first_of(DELIM));
        *_line = _line->substr(_line->find_first_of(DELIM) + 1);

        if (token == "") continue;
        if (token == "time") {
            _time = 1;
            continue;
        }


        if (contains_reg(token)) {
            vector<string> L = regv(token);  
            tokens->insert(tokens->end(), L.begin(), L.end());          
            continue;
        }
        tokens->push_back(token);
    }

    if (_line->empty() && tokens->empty()) {
        tokens->push_back("");
    }
}

void init_conveer(vector<vector<string>> *conveer, string *line) {
    vector<string> conveer_separated;
    split_line(line, &conveer_separated, "|");

    for(int i = 0; i < conveer_separated.size(); i++) {
        vector<string> _str;
        split_line(&conveer_separated[i], &_str, " \n\t");
        conveer->push_back(_str);
    } 
}

struct redirection
{
    int type = -1; //0 -- stdin, 1 -- stdout
    int pos = -1; //0 -- first, 1 -- last
    string file;
};

//finds redirections and puts them into redirs
//redirs.size() <= 2
void init_redirections(
        vector<vector<string>>     & conveer, 
        vector<struct redirection> & redirs
        ) 
    {
  
    for (int i = 1; i < conveer.size() - 1; i++) {
            for (int j = 0; j < conveer[i].size(); j++)
                if (conveer[i][j] == "<" || conveer[i][j] == ">") {
                    print_error("Bad redirection", conveer);
                    return;
                }
    }

    struct redirection r;
    redirs.push_back(r);
    redirs.push_back(r);


    if (conveer.size() == 1) {
        for (int i = 0; i < conveer[0].size(); i++) {
            if (conveer[0][i] == "<" || conveer[0][i] == ">") {
                r.pos = 0;
                r.type = 0;
                if (conveer[0][i] == ">") r.type = 1;
                if (i < conveer[0].size() - 1) r.file = conveer[0][i+1];
                else {
                    print_error("Bad redirection", conveer);
                    return;
                }
                if (redirs[r.type].type == r.type) {
                    print_error("Bad redirection", conveer);
                    return;
                }
                redirs[r.type].type = r.type;
                redirs[r.type].pos = r.pos;
                redirs[r.type].file = r.file;
                continue;
            }
        }
        
        return;
    }

    int pos[2] = {0, (int)conveer.size() - 1};
    for (int k = 0; k < 2; k ++) {
        for (int i = 0; i < conveer[pos[k]].size(); i++) 
        {
            if (conveer[pos[k]][i] == "<" || conveer[pos[k]][i] == ">") {
                r.pos = pos[k];
                r.type = 0;
                if (conveer[pos[k]][i] == ">") r.type = 1;
                if (r.type != k) {
                    print_error("Bad redirection", conveer);
                    return;
                }
                if (i < conveer[pos[k]].size() - 1) r.file = conveer[pos[k]][i+1];
                else {
                    print_error("Bad redirection", conveer);
                    return;
                }
                redirs[r.type].type = r.type;
                redirs[r.type].pos = r.pos;
                redirs[r.type].file = r.file;
                break;
            }
        }
    }
    
}

//converts vector<string> to vector<char *> ignoring redirections
void c_str_array(
        vector<char *> & dest, 
        vector<string> & src
        ) 
{
    dest.clear();
    for (int i = 0, j = 0; i < src.size(); i ++) {
        if (src[i] == "<" || src[i] == ">") continue;
        if (i > 0) if (src[i-1] == "<" || src[i-1] == ">") continue;
        dest.push_back((char *) malloc(sizeof(char)*(src[i].size() + 1)));
        strcpy(dest[j], src[i].c_str());
        j++;
    }
    dest.push_back(NULL);
}

int _killer = 1;
void sigfunc(int val) {
    _killer = 0;
}

bool contains_redir_2(int i, vector<struct redirection> &redirs) {
    if (redirs.size() > 0)
        if (redirs[0].pos == i)
            return true;
    if (redirs.size() > 1)
        if (redirs[1].pos == i)
            return true;
    return false;        
}

int launch(vector<vector<string>> & args_ch, vector<struct redirection> &redirs)
{   
    pid_t pid, wpid;
    int status = 0;

    int pip[2][2], fd_in = 0;
    pip[0][0] = 0; //for n == 1
    int fd1, fd2;
    int n = args_ch.size();
    bool t = 0;
    vector<char *> c_args_ch;

    if (n == 1) {  
        pid = fork();
        if (pid == 0) {

            if (redirs[0].type == 0) {
            fd1 = open(redirs[0].file.c_str(), O_RDONLY);
            dup2(fd1, 0);
            }
            if (redirs[1].type == 1) {
                fd2 = open(redirs[1].file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(fd2, 1);
            }


            c_str_array(c_args_ch, args_ch[0]);
            execvp(c_args_ch[0], &c_args_ch[0]);

            if (redirs[0].type == 0) {
                close (fd1);
            }
            if (redirs[1].type == 1) {
                close(fd2);
            }

            exit(0);
        } 
        else 
        {
            int status;
            wpid = waitpid(pid, &status, 0);      
        }
        return 1;
    }

    // i == 0
    pipe(pip[t]);
    pid = fork();

    if (pid == 0) {
        // cout << "$child i == 0. pid = "<<pid << endl;

        if (redirs[0].type == 0) {
            fd1 = open(redirs[0].file.c_str(), O_RDONLY);
            dup2(fd1, 0);
        }


        close(pip[t][0]);
        dup2(pip[t][1], 1);

        
        c_str_array(c_args_ch, args_ch[0]);
        if (-1 == execvp(c_args_ch[0], &c_args_ch[0])) perror("microsha");
        exit(0);
    }


    for (int i = 1; i < args_ch.size() - 1; i++) {
      close(pip[t][1]); //хочу прочитать из pip[t]  
	  pipe(pip[!t]); //хочу сюда что то написать

	  pid = fork();
	  
      if (pid == 0) {
	  	dup2(pip[t][0], 0); //наконец то читаю

	  	close(pip[!t][0]); //хочу сюда написать
        dup2(pip[!t][1], 1); //пишу

	  	c_str_array(c_args_ch, args_ch[i]);
        if (-1 == execvp(c_args_ch[0], &c_args_ch[0])) perror("microsha");
	  	exit(0);
	  }
      close(pip[t][0]);
      t = !t; 
	}

    //i = n-1
    pid = fork();
    if (pid == 0) {
        
        // cout << "$child i ==  n - 1. pid = "<<pid << endl;

        if (redirs[1].type == 1) {
                fd2 = open(redirs[1].file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(fd2, 1);
        }

        close(pip[t][1]);
        dup2(pip[t][0], 0);
        c_str_array(c_args_ch, args_ch[n-1]);
        if (-1 == execvp(c_args_ch[0], &c_args_ch[0])) perror("microsha");
        
        exit(0);
    } else {
        close(pip[t][0]);
        close(pip[t][1]);

        int status;
        // cout << "$parent: begin to wait. pid = "<<pid << endl;
        wpid = waitpid(pid, &status, 0);
        // cout << "$parent: end to wait. pid = "<<pid << endl;
    }
    
    return 1;
}

int b_cd(vector<string> args) {
    if (args[1].empty()) {
    fprintf(stderr, "ожидается аргумент для \"cd\"\n");
    } else {
        if (chdir(args[1].c_str()) != 0) {
        perror("Microsha");
        }
    }
    return 1;
}
int b_exit(){
    return 0;
}

int execute (vector<vector<string>>& conveer, vector<struct redirection> &redirs) {
    
    if (conveer[0][0] == "" || conveer[0][0] == "\n") {
        // Была введена пустая команда.
        return 1;
    }
    
    if (conveer[0][0] == "cd") {
    return b_cd(conveer[0]);
    }
    
    
    if (conveer[0][0] == "exit") {
    return b_exit();
    }
    
    
    return launch(conveer, redirs);
}


void print_time(int timer) {
    struct rusage rus;
	if ( getrusage(RUSAGE_CHILDREN, &rus) != -1 ){
		printf("all: %lf \nsys : %lf\nuser: %lf\n", ((double)(clock() - timer))/ CLOCKS_PER_SEC,
		(double)rus.ru_stime.tv_sec + (double)rus.ru_stime.tv_usec/1000000.0,
		(double)rus.ru_utime.tv_sec+  (double)rus.ru_utime.tv_usec / 1000000.0);
	}
}

void _print_conveer(vector<vector<string>> & conveer) {
    for (int i = 0; i < conveer.size(); i++)
    {
        for (int j = 0; j < conveer[i].size(); j++)
        {
            cout<<conveer[i][j]<<"#";
        }
        cout << endl;
    }
    
}

void loop() {
    clock_t timer;
    string line;
    vector<vector<string>> conveer = {};
    vector<struct redirection> redirs;
    int args_amount = 0;
    int status = 1;

    do {
        
        invitation();
        read_line(line);
        init_conveer(&conveer, &line);
        init_redirections(conveer, redirs);
        // _print_conveer(conveer);

        if (_time) timer = clock();
        status = execute(conveer, redirs);
        if (_time) print_time(timer);
        
        redirs.clear();
        line.clear();
        conveer.clear();
        _time = false;

    } while (status);
}


int main() {
    
    signal(SIGINT, sigfunc);
    loop();
    
    signal(SIGINT, SIG_DFL);
}