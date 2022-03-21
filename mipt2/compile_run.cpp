#ifndef BZ
#pragma GCC optimize("O3")
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC optimize("no-stack-protector")
//#pragma GCC target ("tune=native")
//#pragma GCC target ("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx")
#pragma GCC optimize ("conserve-stack")
//#pragma GCC optimize ("no-stack-limit")
//#pragma clang optimize on
//#pragma clang loop unroll(enable)
#endif


// #include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <unordered_map>

using namespace std;

unsigned int memory[1024 * 1024] = {0};
unsigned int r[16] = {0};
FILE *out;


int flag = 0;
enum compare{
    EQ = 0,
    L = 1,
    G = 2
};


unsigned int pc = 0;

// length of input
const int _len = 100;

// домик для меток
unordered_map<string, int> labels;

unordered_map<string, double> _double;
unordered_map<string, int> _word;


// домик для команд
unordered_map<string, unsigned int> commands;
const int _commands_amount = 72;



enum format {
    RM = 0,
    RR = 1,
    RI = 2,
    J = 3
};

enum code {
    HALT = 0,
    SYSCALL = 1,
    ADD = 2,
    ADDI = 3,
    SUB = 4,
    SUBI = 5,
    MUL = 6,
    MULI = 7,
    DIV = 8,
    DIVI = 9,
    LC = 12,
    SHL = 13,
    SHLI = 14,
    SHR = 15,
    SHRI = 16,
    AND = 17,
    ANDI = 18,
    OR = 19,
    ORI = 20,
    XOR = 21,
    XORI = 22,
    NOT = 23,
    MOV = 24,
    ADDD = 32,
    SUBD = 33,
    MULD = 34,
    DIVD = 35,
    ITOD = 36,
    DTOI = 37,
    PUSH = 38,
    POP = 39,
    CALL = 40,
    CALLI = 41,
    RET = 42,
    CMP = 43,
    CMPI = 44,
    CMPD = 45,
    JMP = 46,
    JNE = 47,
    JEQ = 48,
    JLE = 49,
    JL = 50,
    JGE = 51,
    JG = 52,
    LOAD = 64,
    STORE = 65,
    LOAD2 = 66,
    STORE2 = 67,
    LOADR = 68,
    LOADR2 = 69,
    STORER = 70,
    STORER2 = 71
};

void commands_init() {
    commands.emplace("halt", RI * _commands_amount + 0);
    commands.emplace("syscall", RI * _commands_amount + 1);
    commands.emplace("add", RR * _commands_amount + 2);
    commands.emplace("addi", RI * _commands_amount + 3);
    commands.emplace("sub", RR * _commands_amount + 4);
    commands.emplace("subi", RI * _commands_amount + 5);
    commands.emplace("mul", RR * _commands_amount + 6);
    commands.emplace("muli", RI * _commands_amount + 7);
    commands.emplace("div", RR * _commands_amount + 8);
    commands.emplace("divi", RI * _commands_amount + 9);
    // ai
    commands.emplace("lc", RI * _commands_amount + 12);
    commands.emplace("shl", RR * _commands_amount + 13);
    commands.emplace("shli", RI * _commands_amount + 14);
    commands.emplace("shr", RR * _commands_amount + 15);
    commands.emplace("shri", RI * _commands_amount + 16);
    commands.emplace("and", RR * _commands_amount + 17);
    commands.emplace("andi", RI * _commands_amount + 18);
    commands.emplace("or", RR * _commands_amount + 19);
    commands.emplace("ori", RI * _commands_amount + 20);
    commands.emplace("xor", RR * _commands_amount + 21);
    commands.emplace("xori", RI * _commands_amount + 22);
    commands.emplace("not", RI * _commands_amount + 23);
    commands.emplace("mov", RR * _commands_amount + 24);
    commands.emplace("addd", RR * _commands_amount + 32);
    commands.emplace("subd", RR * _commands_amount + 33);
    commands.emplace("muld", RR * _commands_amount + 34);
    commands.emplace("divd", RR * _commands_amount + 35);
    commands.emplace("itod", RR * _commands_amount + 36);
    commands.emplace("dtoi", RR * _commands_amount + 37);
    commands.emplace("push", RI * _commands_amount + 38);
    commands.emplace("pop", RI * _commands_amount + 39);
    commands.emplace("call", RR * _commands_amount + 40);
    commands.emplace("calli", J * _commands_amount + 41);
    commands.emplace("ret", J * _commands_amount + 42);
    commands.emplace("cmp", RR * _commands_amount + 43);
    commands.emplace("cmpi", RI * _commands_amount + 44);
    commands.emplace("cmpd", RR * _commands_amount + 45);
    commands.emplace("jmp", J * _commands_amount + 46);
    commands.emplace("jne", J * _commands_amount + 47);
    commands.emplace("jeq", J * _commands_amount + 48);
    commands.emplace("jle", J * _commands_amount + 49);
    commands.emplace("jl", J * _commands_amount + 50);
    commands.emplace("jge", J * _commands_amount + 51);
    commands.emplace("jg", J * _commands_amount + 52);
    //ai^2
    commands.emplace("load", RM * _commands_amount + 64);
    commands.emplace("store", RM * _commands_amount + 65);
    commands.emplace("load2", RM * _commands_amount + 66);
    commands.emplace("store2", RM * _commands_amount + 67);
    commands.emplace("loadr", RR * _commands_amount + 68);
    commands.emplace("storer", RR * _commands_amount + 69);
    commands.emplace("loadr2", RR * _commands_amount + 70);
    commands.emplace("storer2", RR * _commands_amount + 71);
}

unsigned int reg_num(string str) {
    unsigned int x;
    sscanf(str.c_str(), "r%ud", &x);
    return x;
}

long long double_uint64 (double d) {
    union {
        double d;
        long long i;
    } u;
    u.d = d;
    return u.i;
}



// делим вход на разные слова
int splitter(string str, string words[]) {
    
    int _start = 0;
    int _counter = 0;

    for (int i = 0; i <(int)str.size(); i++) {
        if (str[i] == ';') {
            str = str.substr(0, i);
            break;
        }
        if (str[i] == ',') {
            str[i] = ' ';
        }
        if (str[i] == '\t') {
            str[i] = ' ';
        }
    }

    int _flag = 1;
    for (int i = 0; i < (int)str.size(); i++)
    {
        if (str[i] == ' ' && _flag == 0) {
            words[_counter] = str.substr(_start, i - _start);
            _flag = 1;
            _counter++;
            if (_counter == 5) {
                return 5;
            }
            continue;
        }
        
        if (str[i] != ' ' && _flag == 1) {
            _start = i;
            _flag = 0;
            continue;
        }

    }
    if (_flag == 0) {
        words[_counter] = str.substr(_start, str.size() - _start);
        _counter++;
    }
    return _counter;
}

// поиск меток
int labs_init(int k, string word[]) {
    if (k == 0) {
        return 0;
    }

    if (word[0][word[0].size() - 1] == ':') {
        word[0] = word[0].substr(0, word[0].size() - 1);

        if (word[1] == "word") {
            int x;
            sscanf(word[2].c_str(), "%d", &x);
            labels.emplace(word[0], x);
        } else if (word[1] == "double") {
            double x;
            sscanf(word[2].c_str(), "%lf", &x);
            _double.emplace(word[0], x);
        } else {
            labels.emplace(word[0], pc);
            if (k > 1) {
                pc++;
            }
        }


    } else {
        pc++;
    }
    
    return 0;
}

// создание слов
int translate(int k, string words[]) {
    int start = 0;
    int debug = 0;


    if (words[0] == "end") {
        r[15] = labels[words[1]];
        return 0;
    }
    if (k == 0) {
        return 0;
    }
    if (words[0][words[0].size() - 1] == ':') {
        if (k == 1) {
            return 0;
        }
        start++;
    }


    if (words[0] == "word") {
        int a;
        sscanf(words[0].c_str(), "%d", &a);
        memory[pc] = a;
        pc++;
        return 0;
    }
    if (words[0] == "double") {
        double a;
        sscanf(words[0].c_str(), "%lf", &a);
        long long _i = double_uint64(a);
        memory[pc + 1] = _i;
        _i >>= 32;
        memory[pc] = _i;
        pc +=2;
        return 0;
    }

    if (words[start] == "word" || words[start] == "double") {
        return 0;
    }

    int type = commands[words[start]];
    int num = type % _commands_amount;
    type /= _commands_amount;
    unsigned int ans = 0;

    unsigned int _r = 0;
    int x = 0;
    int t = 0;

    union {
        short sh_s;
        unsigned short sh_u;
    } sign_16;

    ans += (num << 24);

    switch (type) {
        case RM:
            _r = reg_num(words[start + 1]);
            _r = _r << 20;

            ans += _r;

            
            if (labels.find(words[start + 2]) == labels.end()) {
                sscanf(words[start + 2].c_str(), "%u", &_r);
            }
            else {
                _r = labels.at(words[start + 2]);
            }


            ans += _r;

            if (debug) {
                cout << words[start] << ", RM: " << endl;
                printf("\tans: %x\n", ans);
            }
            break;

        case RI:
            _r = reg_num(words[start + 1]);
            _r = _r << 20;

            ans += _r;

            if (labels.find(words[start + 2]) == labels.end()) {
                sscanf(words[start + 2].c_str(), "%d", &x);
            }
            else {
                x = labels.at(words[start + 2]);
            }


            if (x < 0) {
                t = 1;
            }
            x = x & (0x000fffff);
            x |= (t << 19);
            ans += x;

            if (debug) {
                cout << words[start] << ", RI: " << endl;
                printf("\tans: %x\n", ans);
            }
         break;

        case RR:
            _r = reg_num(words[start + 1]);
            _r = _r << 20;
            ans += _r;

            _r = reg_num(words[start + 2]);
            _r = _r << 16;
            ans += _r;

            sscanf(words[start + 3].c_str(), "%hd", &sign_16.sh_s);
            ans = ans + sign_16.sh_u;

            if (debug) {
                cout << words[start] << ", RR: " << endl;
                printf("\tans: %x\n", ans);
            }
            break;
        case J:
            if (words[start] == "ret") {
                unsigned int k;
                sscanf(words[start + 1].c_str(), "%u", &k);
                ans += k;
                break;
            }

            unsigned int _r;
            if (labels.find(words[start + 1]) == labels.end()) {
                sscanf(words[start + 1].c_str(), "%u", &_r);
            }
            else {
                _r = labels.at(words[start + 1]);
            }

            ans += _r;

            if (debug) {
                cout << words[start] << ", J: " << endl;
                printf("\tans: %x\n", ans);
            }           
            break;
    }

    memory[pc] = ans;
    pc ++;
    return 0;
}

void debuggggg() {
    printf("%x", memory[r[15]]);
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            printf("\n");
        }
        printf("r[%d]: %d; ", i, r[i]);
    }
    printf("\n");
    printf ("flag: %d\n\n", flag);
    printf("stack[ r[14] - 1]: %d\n", memory[r[14] - 1]);
    printf("stack[ r[14] ]: %d\n", memory[r[14]]);
    printf("stack[ r[14] + 1]: %d\n", memory[r[14] + 1]);
    printf("stack[ r[14] + 2]: %d\n", memory[r[14] + 2]);
    printf("stack[ r[14] + 3]: %d\n", memory[r[14] + 3]);
    printf("stack[ r[14] + 4]: %d\n\n", memory[r[14] + 4]);
    getchar();
}


double r_doub (int k) {
    union {
        double d;
        unsigned long int i;
    } d_i;
    d_i.i = r[k+1];
    d_i.i <<= 32;
    d_i.i += r[k];
    return d_i.d;
}

int int20_int32 (int t) {
    if ((t & 0xfff80000) != 0)
    {

        t = t | 0xfff80000;
    }
    return t;

}


void word_args(int type, unsigned int word, unsigned int ar[]) {
    switch (type) {
        case RM:
            ar[0] = (word & 0x00F00000) >> 20;
            ar[1] = (word & 0x000FFFFF);
            break;
        case RR:
            ar[0] = (word & 0x00F00000) >> 20;
            ar[1] = (word & 0x000F0000) >> 16;
            ar[2] = word & 0x0000FFFF;
            break;
        case RI:
            ar[0] = (word & 0x0f00000) >> 20;
            ar[1] = word & 0x00fffff;
            break;
        case J:
            ar[0] = word & 0x000fffff;
            break;

        default:
            break;
    }
}



int exec() {
    unsigned int mem = memory[r[15]];
    unsigned int num = mem >> 24;

    unsigned int args[3];

    union {
        unsigned int _uint;
        int _int;
    } uint32_int32;

    union {
        unsigned short _uint;
        short _int;
    } uint16_int16;

    union {
        unsigned long int _uint;
        double _doub;
    } uint64_doub;

    union {
        unsigned long int _uint;
        long int _int;
    } uint64_int64;

    switch (num) {
        case HALT:
            return -1;
            break;
        case SYSCALL:
            word_args(RI, mem, args);
            switch (args[1]) {
                case 0:
                    return -1;
                    break;
                case 100:
                    scanf("%d", &uint32_int32._int);
                    r[args[0]] = uint32_int32._uint;

                    break;
                case 101:
                    scanf("%lf", &uint64_doub._doub);
                    r[args[0]] = uint64_doub._uint;
                    uint64_doub._uint >>= 32;
                    r[args[0]+1] = uint64_doub._uint;
                    break;
                case 102:
                    fprintf(out, "%d", r[args[0]]);
                    break;
                case 103:
                    uint64_doub._uint = r[args[0] + 1];
                    uint64_doub._uint <<= 32;
                    uint64_doub._uint += r[args[0]];
                    fprintf(out, "%lg", uint64_doub._doub);
                    break;
                case 104:
                    char ch;
                    scanf("%c", &ch);
                    r[args[0]] = ch;
                    break;
                case 105:
                    fprintf(out, "%c", r[args[0]]);
                    break;
            }

            r[15]++;
            break;
        case ADD:
            word_args(RR, mem, args);
            r[args[0]] = r[args[1]] + args[2] + r[args[0]];
            r[15]++;
            break;
        case ADDI:
            word_args(RI, mem, args);
            r[args[0]] = (r[args[0]] + int20_int32(args[1]));
            r[15]++;
            break;
        case SUB:
            word_args(RR, mem, args);
            r[args[0]] = r[args[0]] - args[2] - r[args[1]];
            r[15]++;
            break;
        case SUBI:
            word_args(RI, mem, args);
            //printf("subi: %d\n", int20_int32(args[1]));
            r[args[0]] -= int20_int32(args[1]);
            r[15]++;
            break;
        case MUL: {
            word_args(RR, mem, args);
            uint32_int32._uint = r[args[0]];
            long int le = uint32_int32._int;

            uint32_int32._uint = r[args[1]];
            long int ri = uint32_int32._int;


            uint16_int16._uint = args[2];
            ri += uint16_int16._int;

            uint64_int64._int = le * ri;
            r[args[0]] = uint64_int64._uint;
            uint64_int64._uint >>= 32;
            r[args[0] + 1] = uint64_int64._uint;

            r[15]++;
            break;
        }
        case MULI:
        {
            word_args(RI, mem, args);
            uint32_int32._uint = r[args[0]];
            long int le = uint32_int32._int;

            uint64_int64._int = le * int20_int32(args[1]);
            r[args[0]] = uint64_int64._uint;
            uint64_int64._uint >>= 32;
            r[args[0] + 1] = uint64_int64._uint;

            r[15]++;
            break;
        }
        case DIV:
        {
            word_args(RR, mem, args);
            uint64_int64._uint = r[args[0] + 1];
            uint64_int64._uint <<=32;
            uint64_int64._uint += r[args[0]];
            long int a = uint64_int64._int;

            uint32_int32._uint = r[args[1]];
            long int b = uint32_int32._int;

            uint32_int32._int = a / b;
            r[args[0]] = uint32_int32._uint;

            uint32_int32._int = a % b;
            r[args[0] + 1] = uint32_int32._uint;

            r[15]++;
            break;
        }
        case DIVI:
        {
            word_args(RI, mem, args);
            uint64_int64._uint = r[args[0] + 1];
            uint64_int64._uint <<=32;
            uint64_int64._uint += r[args[0]];
            long int a = uint64_int64._int;

            long int b = int20_int32(args[1]);

            uint32_int32._int = a / b;
            r[args[0]] = uint32_int32._uint;

            uint32_int32._int = a % b;
            r[args[0] + 1] = uint32_int32._uint;

            r[15]++;
            break;

        }
        case LC:
            word_args(RI, mem, args);
            r[args[0]] = args[1];
            r[15]++;
            break;
        case SHL:
            word_args(RR, mem, args);
            r[args[0]] <<= r[args[1]];
            r[15]++;
            break;
        case SHLI:
            word_args(RI, mem, args);
            r[args[0]] <<= args[1];
            r[15]++;
            break;
        case SHR:
            word_args(RR, mem, args);
            if (r[args[1]] >= 32) {
                r[args[0]] = 0;
            }
            else {
                r[args[0]] >>= r[args[1]];
            }
            r[15]++;
            break;
        case SHRI:
            word_args(RI, mem, args);
            if (args[1] >= 32) {
                r[args[0]] = 0;
            }
            else {
                r[args[0]] >>= args[1];
            }
            r[15]++;
            break;
        case AND:
            word_args(RR, mem, args);
            r[args[0]] &= r[args[1]];
            r[15]++;
            break;
        case ANDI:
            word_args(RI, mem, args);
            r[args[0]] &= args[1];
            r[15]++;
            break;
        case OR:
            word_args(RR, mem, args);
            r[args[0]] |= r[args[1]];
            r[15]++;
            break;
        case ORI:
            word_args(RI, mem, args);
            r[args[0]] |= args[1];
            r[15]++;
            break;
        case XOR:
            word_args(RR, mem, args);
            r[args[0]] ^= r[args[1]];
            r[15]++;
            break;
        case XORI:
            word_args(RI, mem, args);
            r[args[0]] ^= args[1];
            r[15]++;
            break;
        case NOT:
            word_args(RI, mem, args);
            r[args[0]] = ~r[args[0]];
            r[15]++;
            break;
        case MOV:
            word_args(RR, mem, args);
            r[args[0]] = r[args[1]] + args[2];
            r[15]++;
            break;
        case ADDD:
        {
            word_args(RR, mem, args);
            double d1 = r_doub(args[0]);
            double d2 = r_doub(args[1]);
            d1 += d2;
            uint64_doub._doub = d1;
            r[args[0]] = uint64_doub._uint;
            uint64_doub._uint >>= 32;
            r[args[0]+1] = uint64_doub._uint;
            r[15]++;
            break;
        }
        case SUBD:
        {
            word_args(RR, mem, args);
            double d1 = r_doub(args[0]);
            double d2 = r_doub(args[1]);
            d1 -= d2;
            uint64_doub._doub = d1;
            r[args[0]] = uint64_doub._uint;
            uint64_doub._uint >>= 32;
            r[args[0]+1] = uint64_doub._uint;
            r[15]++;
            break;
        }
        case MULD:
        {
            word_args(RR, mem, args);
            double d1 = r_doub(args[0]);
            double d2 = r_doub(args[1]);
            d1 *= d2;
            uint64_doub._doub = d1;
            r[args[0]] = uint64_doub._uint;
            uint64_doub._uint >>= 32;
            r[args[0]+1] = uint64_doub._uint;
            r[15]++;
            break;
        }
        case DIVD:
        {
            word_args(RR, mem, args);
            double d1 = r_doub(args[0]);
            double d2 = r_doub(args[1]);
            d1 /= d2;
            uint64_doub._doub = d1;
            r[args[0]] = uint64_doub._uint;
            uint64_doub._uint >>= 32;
            r[args[0]+1] = uint64_doub._uint;
            r[15]++;
            break;
        }
        case ITOD:
        {
            word_args(RR, mem, args);
            uint32_int32._uint = r[args[1]];
            double d = (double)uint32_int32._int;
            uint64_doub._doub = d;
            r[args[0]] = uint64_doub._uint;
            uint64_doub._uint >>= 32;
            r[args[0]+1] = uint64_doub._uint;
            r[15]++;
            break;
        }
        case DTOI:
        {
            word_args(RR, mem, args);
            double d = r_doub(args[1]);

            int i = (int) d;
            r[args[0]] = i;
            r[15]++;
            break;
        }
        case PUSH:
            word_args(RI, mem, args);
            r[14]--;
            memory[r[14]] = r[args[0]] + args[1];
            r[15]++;
            break;
        case POP:
            word_args(RI, mem, args);

            r[args[0]] = memory[r[14]] + args[1];
            r[14]++;
            r[15]++;
            break;
        case CALL:
            {
            //r[arg[0]] = r[15] + 1
            //stack = r[15] + 1
            //r[15] = args[1]

            word_args(RR, mem, args);
            int _r1 = r[args[1]];

            r[14]--;
            memory[r[14]] = r[15] + 1;
            r[args[0]] = r[15] + 1;

            r[15] = _r1;
            return 0;
            }
        case CALLI:
            word_args(J, mem, args);
            r[14]--;
            memory[r[14]] = r[15] + 1;
            r[15] = args[0];
            break;
        case RET:
            word_args(J, mem, args);

            r[15] = memory[r[14]];
            r[14] += args[0] + 1;
            break;
        case CMP:
        {
            word_args(RR, mem, args);
            int _l, _r;
            uint32_int32._uint = r[args[0]];
            _l = uint32_int32._int;

            uint32_int32._uint = r[args[1]];
            _r = uint32_int32._int;

            if (_l == _r) {
                flag = EQ;
            }
            if (_l < _r) {
                flag = L;
            }
            if (_l > _r) {
                flag = G;
            }
            r[15]++;
            break;
        }
        case CMPI:
        {
            word_args(RI, mem, args);
            int _l;
            uint32_int32._uint = r[args[0]];
            int _arg = args[1];
            _l = uint32_int32._int;

            if (_l == _arg) {
                flag = EQ;
            }
            if (_l < _arg) {
                flag = L;
            }
            if (_l > _arg) {
                flag = G;
            }
            r[15]++;
            break;
        }
        case CMPD:
        {
            word_args(RI, mem, args);
            double _l, _r;
            _l = r_doub(r[args[0]]);
            _r =  r_doub(r[args[1]]);

            if (_l == _r) {
                flag = EQ;
            }
            if (_l < _r) {
                flag = L;
            }
            if (_l > _r) {
                flag = G;
            }
            r[15]++;
            break;
        }

        case JMP:
            word_args(J, mem, args);
            r[15] = args[0];
            break;
        case JNE:
            word_args(J, mem, args);
            r[15]++;
            if (flag != EQ) {
                r[15] = args[0];
            }
            break;
        case JEQ:
            word_args(J, mem, args);
            r[15]++;
            if (flag == EQ) {
                r[15] = args[0];
            }
            break;
        case JLE:
            word_args(J, mem, args);
            r[15]++;
            if (flag != G) {
                r[15] = args[0];
            }
            break;
        case JL:
            word_args(J, mem, args);
            r[15]++;
            if (flag == L) {
                r[15] = args[0];
            }
            break;
        case JGE:
            word_args(J, mem, args);
            r[15]++;
            if (flag != L) {
                r[15] = args[0];
            }
            break;
        case JG:
            word_args(J, mem, args);
            r[15]++;
            if (flag == G) {
                r[15] = args[0];
            }
            break;
        case LOAD:
            word_args(RM, mem, args);
            r[args[0]] = memory[args[1]];
            r[15]++;
            break;
        case STORE:
            word_args(RM, mem, args);
            memory[args[1]] = r[args[0]];
            r[15]++;
            break;
        case LOAD2:
            word_args(RM, mem, args);
            r[args[0]] = memory[args[1]];
            r[args[0]+1] = memory[args[1]+1];
            r[15]++;
            break;
        case STORE2:
            word_args(RM, mem, args);
            memory[args[1]] = r[args[0]];
            memory[args[1]+1] = r[args[0]+1];
            r[15]++;
            break;
        case LOADR:
            word_args(RR, mem, args);
            r[args[0]] = memory[r[args[1]] + args[2]];
            r[15]++;
            break;
        case LOADR2:
            word_args(RR, mem, args);
            r[args[0]] = memory[r[args[1]] + args[2]];
            r[args[0]+1] = memory[r[args[1]] + args[2] +1];
            r[15]++;
            break;
        case STORER:
            word_args(RR, mem, args);
            memory[r[args[1]] + args[2]] = r[args[0]];
            r[15]++;
            break;
        case STORER2:
            word_args(RR, mem, args);
            memory[r[args[1]] + args[2]] = r[args[0]];
            memory[r[args[1]] + args[2] + 1] = r[args[0] + 1];
            r[15]++;
            break;
        default:
            printf("Что ты наделал?(\n");
            break;
    }

    return 0;
}


int main(int argc, char * argv[]) {
    
    //вывод разбиения строки
    int _splitter = 0;

    commands_init();
    

    r[14] = 1048575;
    ifstream input("input.fasm");

    string str;
    string words[5];
    words[0] = "";

    
    int _count;

    while (words[0] != "end")
    {       
        getline(input, str);
        _count = splitter(str, words);
        labs_init(_count, words);

        if (_splitter) {
            cout << _count << ": ";
            for (int i = 0; i < _count; i++)
            {
                cout<<"#" <<words[i] <<"# ";
            }
            cout <<endl;
            getchar();
        }

    }
    input.close();



    input = ifstream("input.fasm");
    words[0] = "";
    pc = 0;
    while (words[0] != "end")
    {      
        getline(input, str);
        _count = splitter(str, words);
        translate(_count, words);
    }
    input.close();

    out = fopen("output.txt", "w");
    while (1) {
        if (argc > 1)
            debuggggg();

        if (exec() == -1) {
            fclose(out);
            return 0;
        }
    }
}