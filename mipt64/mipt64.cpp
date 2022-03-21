#ifndef BZ
#pragma GCC optimize("O3")
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC optimize("no-stack-protector")
//#pragma GCC target ("tune=native")
//#pragma GCC target ("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx")
#pragma GCC optimize("conserve-stack")
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
#include <cmath>
using namespace std;

unsigned int memory[524288] = {0};
unsigned long long r[32] = {0};

int flag = 0;
enum compare
{
    EQ = 0,
    L = 1,
    G = 2
};

unsigned int pc = 0;

// length of input
const int _len = 100;

// домик для меток
unordered_map<string, int> labels;

// домик для команд
const int _commands_amount = 30;
string commands[_commands_amount] = {""};
unordered_map<string, int> commands_num;

enum format
{
    RR = 0,
    RM = 1,
    B = 2
};

enum code
{
    HALT = 0,
    SVC = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    MOD = 6,
    AND = 7,
    OR = 8,
    XOR = 9,
    NAND = 10,
    SHL = 11,
    SHR = 12,
    ADDD = 13,
    SUBD = 14,
    MULD = 15,
    DIVD = 16,
    ITOD = 17,
    DTOI = 18,
    BL = 19,
    CMP = 20,
    CMPD = 21,
    CNE = 22,
    CEQ = 23,
    CLE = 24,
    CLT = 25,
    CGE = 26,
    CGT = 27,
    LD = 28,
    ST = 29,
};

void commands_init()
{
    commands[0] = "halt";
    commands[1] = "svc";
    commands[2] = "add";
    commands[3] = "sub";
    commands[4] = "mul";
    commands[5] = "div";
    commands[6] = "mod";
    commands[7] = "and";
    commands[8] = "or";
    commands[9] = "xor";
    commands[10] = "nand";
    commands[11] = "shl";
    commands[12] = "shr";
    commands[13] = "addd";
    commands[14] = "subd";
    commands[15] = "muld";
    commands[16] = "divd";
    commands[17] = "itod";
    commands[18] = "dtoi";
    commands[19] = "bl";
    commands[20] = "cmp";
    commands[21] = "cmpd";
    commands[22] = "cne";
    commands[23] = "ceq";
    commands[24] = "cle";
    commands[25] = "clt";
    commands[26] = "cge";
    commands[27] = "cgt";
    commands[28] = "ld";
    commands[29] = "st";

    for (int i = 0; i < _commands_amount; i++)
    {
        commands_num.emplace(commands[i], i);
    }
}

unsigned int reg_num(string str)
{
    unsigned int x;
    if (str == "rz")
    {
        x = 27;
    }
    else if (str == "fp")
    {
        x = 28;
    }
    else if (str == "sp")
    {
        x = 29;
    }
    else if (str == "lr")
    {
        x = 30;
    }
    else if (str == "pc")
    {
        x = 31;
    }
    else
    {
        sscanf(str.c_str(), "r%ud", &x);
    }
    return x;
}

long long double_uint64(double d)
{
    union
    {
        double d;
        long long i;
    } u;
    u.d = d;
    return u.i;
}

double uint64_double(unsigned long long d)
{
    union
    {
        double d;
        unsigned long long i;
    } u;
    u.i = d;
    return u.d;
}
// делим вход на разные слова
int splitter(string str, string words[])
{

    int _start = 0;
    int _counter = 0;


    for (int i = 0; i < (int)str.size(); i++)
    {
        if (str[i] == ';')
        {
            str = str.substr(0, i);
            break;
        }
        if (str[i] == ',')
        {
            str[i] = ' ';
        }
        if (str[i] == '\t')
        {
            str[i] = ' ';
        }
    }

    int _flag = 1;
    for (int i = 0; i < (int)str.size(); i++)
    {
        if (str[i] == ' ' && _flag == 0)
        {
            words[_counter] = str.substr(_start, i - _start);
            _flag = 1;
            _counter++;
            if (_counter == 10)
            {
                return 10;
            }
            continue;
        }

        if (str[i] != ' ' && _flag == 1)
        {
            _start = i;
            _flag = 0;
            continue;
        }
    }
    if (_flag == 0)
    {
        words[_counter] = str.substr(_start, str.size() - _start);
        _counter++;
    }
    return _counter;
}

// поиск меток
int labs_init(int k, string word[])
{
    if (k == 0)
    {
        return 0;
    }

    if (word[0][word[0].size() - 1] == ':')
    {
        word[0] = word[0].substr(0, word[0].size() - 1);

        labels.emplace(word[0], pc);
        if (k > 1)
        {
            pc++;
        }
    }
    else
    {
        pc++;
    }

    return 0;
}

// создание слов
int translate(int k, string words[])
{
    int start = 0;
    if (words[0][words[0].size() - 1] == ':')
    {
        if (k == 1)
        {
            return 0;
        }
        start++;
    }


    if (words[start] == "end")
    {
        r[31] = labels[words[1+start]];
        return 0;
    }
    if (k == 0)
    {
        return 0;
    }

    if (words[start] == "word")
    {
        int a;
        sscanf(words[1].c_str(), "%d", &a);
        memory[pc] = a;
        pc++;
        return 0;
    }
    if (words[start] == "double")
    {
        double a;
        sscanf(words[1].c_str(), "%lf", &a);
        long long _i = double_uint64(a);
        memory[pc + 1] = _i;
        _i >>= 32;
        memory[pc] = _i;
        pc += 2;
        return 0;
    }
    if (words[start] == "dword")
    {
        long long a;
        sscanf(words[1].c_str(), "%lld", &a);
        memory[pc + 1] = a;
        a >>= 32;
        memory[pc] = a;
        pc += 2;
        return 0;
    }
    if (words[start] == "bytes")
    {
        int k;
        sscanf(words[1].c_str(), "%d", &k);
        k = (k - 1) / 4 + 1;
        for (int i = 0; i < k; i++)
            memory[pc + i] = 0;
        pc += k;
        return 0;
    }

    int type = RR;
    int num = commands_num.at(words[start]);
    if (num >= 28)
        type = RM;
    if (num == 19)
        type = B;

    unsigned int ans = 0;
    int R1 = reg_num(words[start + 1]);
    int R2 = reg_num(words[start + 2]);
    ans = num << 26;
    ans += R1 << 21;
    int RI, imm, bits;

    if (num >= 22 && num <= 27 && k == 3 + start) {
        ans+=reg_num("rz")<<16;
        ans+=labels[words[start+2]];
        type = -2;
    }
    


    switch (type)
    {
    case RR:
        if (labels.find(words[start+3]) != labels.end()) {
            R2 = reg_num("rz");
            ans += R2 << 16;
            ans += labels[words[start+3]];
            memory[pc] = ans;
            pc++;
            return 0;
        } 
        ans += R2 << 16;
        if (R2 == reg_num("rz"))
        {
            sscanf(words[start + 3].c_str(), "%d", &imm);
            imm = imm & 0x0000ffff;
            ans += imm;
            break;
        }
        RI = reg_num(words[start + 3]);
        ans += RI << 11;
        sscanf(words[start + 4].c_str(), "%d", &bits);
        sscanf(words[start + 5].c_str(), "%d", &imm);
        ans += bits << 8;
        ans += imm;
        break;
    case RM:
        if (R2 == reg_num("pc") || R2 == reg_num("rz") || R2 == reg_num("sp"))
        {
            sscanf(words[start + 3].c_str(), "%d", &imm);
            imm = imm & 0x0000ffff;
            ans += R2<<16;
            ans += imm;
            memory[pc] = ans;
            pc++;
            return 0;
        }
        ans+= R2<<16;
        RI = reg_num(words[start + 3]);
        ans += RI << 11;
        if (RI == reg_num("rz"))
        {
            sscanf(words[start + 4].c_str(), "%d", &imm);
            ans += imm;
            memory[pc] = ans;
            pc++;
            return 0;
        }
        sscanf(words[start + 4].c_str(), "%d", &bits);
        sscanf(words[start + 5].c_str(), "%d", &imm);
        ans += bits << 8;
        ans += imm;
        break;
    case B:
        if (k == 2+start) {
            ans = num << 26;
            ans += reg_num("rz")<<21;
            ans += labels[words[start+1]];
        } else {
        sscanf(words[start + 3].c_str(), "%d", &imm);
        imm = imm & 0x001fffff;
        ans += imm;
        }
        break;
    }
    memory[pc] = ans;
    pc++;
    return 0;
}

//32bit to args
int word_args(unsigned int word, int ar[])
{
    ar[0] = (word & 0xfc000000) >> 26;
    int type = RR;
    if (ar[0] >= 28)
        type = RM;
    if (ar[0] == 19)
        type = B;
    ar[1] = (word & 0x03e00000) >> 21;
    ar[2] = (word & 0x1f0000) >> 16;

    switch (type)
    {
    case RR:
        if (ar[2] == reg_num("rz"))
        {
            ar[3] = word & 0xffff;
            if (word & 0x00008000)
            {
                ar[3] |= 0xffffffffffff0000;
            }
            return 4;
        }
        ar[3] = (word & 0xf800) >> 11;
        ar[4] = (word & 0x300) >> 8;
        ar[5] = word & 0xff;
        if (word & 0x80)
        {
            ar[5] |= 0xffffffffffffff00;
        }
        return 6;
        break;
    case RM:
        if (ar[2] == reg_num("pc") || ar[2] == reg_num("rz") || ar[2] == reg_num("sp"))
        {
            ar[3] = word & 0xffff;
            if (ar[2] != reg_num("sp") && word & 0x8000)
                ar[3] |= 0xffffffffffff0000;
            return 4;
        }
        ar[3] = (word & 0xf800) >> 11;
        if (ar[3] == reg_num("rz"))
        {
            ar[4] = word & 0x7ff;
            if (ar[4] & 0x400)
                ar[4] |= 0xffffffffffff8000;
            return 5;
        }
        ar[4] = (word & 0x300) >> 8;
        ar[5] = word & 0xff;
        if (ar[4] & 0x80)
            ar[4] |= 0xffffffffffffff00;
        return 6;
        break;
    case B:
        if (ar[1] == reg_num("pc") || ar[1] == reg_num("rz"))
        {
            ar[2] = word & 0x1fffff;
            if (ar[1] == reg_num("pc") && ar[1] & 0x100000)
                ar[1] |= 0xfffffffffff00000;
            return 3;
        }
        ar[3] = (word & 0xe000) >> 13;
        ar[4] = word & 0x7ff;
        if (ar[4] & 0x400)
            ar[4] |= 0xfffffffffffff800;
        return 5;
        break;

    default:
        break;
    }
    return -1;
}

void args_print(int k, int ar[])
{
    cout << commands[ar[0]] << " ";
    for (int j = 1; j < k; j++)
    {
        printf("%d ", ar[j]);
    }
    cout << endl;
}

void debuggggg()
{

    int ar[7];
    int k = word_args(memory[r[31]], ar);
    args_print(k, ar);

    for (int i = 0; i < 32; i++)
    {   
        if (i>20 && i <26)
            continue;
        if (i % 4 == 0)
        {
            printf("\n");
        }
        printf("r[%d]: %lld; ", i, r[i]);
    }
    printf("\n");
    printf("flag: %d\n\n", flag);
    printf("stack[ r[29] - 1]: %d\n", memory[r[29] - 1]);
    printf("stack[ r[29] ]: %d\n", memory[r[29]]);
    printf("stack[ r[29] + 1]: %d\n", memory[r[29] + 1]);
    printf("stack[ r[29] + 2]: %d\n", memory[r[29] + 2]);
    printf("stack[ r[29] + 3]: %d\n", memory[r[29] + 3]);
    printf("stack[ r[29] + 4]: %d\n\n", memory[r[29] + 4]);
    getchar();
}

int exec()
{
    int _int32;
    long long _int64;
    double _doub;
    int ar[7];
    int count = word_args(memory[r[31]], ar);
    switch (ar[0])
    {
    case HALT:
        return -1;
        break;
    case SVC:
        switch (ar[3])
        {
        case 0:
            return -1;
            break;
        case 100:
            scanf("%lld", &r[ar[1]]);
            break;
        case 101:
            scanf("%lf", &_doub);
            r[ar[1]] = double_uint64(_doub);
            break;
        case 102:
            printf("%lld", (long long)r[ar[1]]);
            break;
        case 103:
            printf("%lf", uint64_double(r[ar[1]]));
            break;
        case 104:
            r[ar[1]] = getchar();
            break;
        case 105:
            printf("%c", (char)r[ar[1]]);
            break;
        default:
            break;
        }
        r[31]++;
        break;
    case ADD:
        if (count == 4)
        {
            r[ar[1]] = ar[3];
        }
        else
        {
            r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4]) + ar[5];
        }
        r[31]++;
        break;
    case SUB:
        if (count == 4)
        {
            ar[r[1]] = ar[r[3]];
        }
        else
        {
            r[ar[1]] = r[ar[2]] - ((r[ar[3]] << ar[4]) + ar[5]);
        }
        r[31]++;
        break;
    case MUL:
        r[ar[1]] = r[ar[2]] * (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case DIV:
        if (r[ar[3]] << ar[4] + ar[5] == 0)
        {
            printf("zero divisiob\n");
            return -1;
        }
        r[ar[1]] = r[ar[2]] / (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case MOD:
        if (r[ar[3]] << ar[4] + ar[5] == 0)
        {
            printf("zero divisiob\n");
            return -1;
        }
        r[ar[1]] = r[ar[2]] % (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case AND:
        r[ar[1]] = r[ar[2]] & (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case OR:
        r[ar[1]] = r[ar[2]] | (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case XOR:
        r[ar[1]] = r[ar[2]] ^ (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case NAND:
        r[ar[1]] = ~(r[ar[2]] ^ (r[ar[3]] << ar[4] + ar[5]));
        r[31]++;
        break;
    case SHL:
        r[ar[1]] = r[ar[2]] << (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case SHR:
        r[ar[1]] = r[ar[2]] >> (r[ar[3]] << ar[4] + ar[5]);
        r[31]++;
        break;
    case ADDD:
        r[ar[1]] = double_uint64(uint64_double(r[ar[1]]) + (uint64_double(r[ar[2]]) * pow(2, uint64_double(ar[3])) + uint64_double(ar[4])));
        r[31]++;
        break;
    case SUBD:
        r[ar[1]] = double_uint64(uint64_double(r[ar[1]]) - (uint64_double(r[ar[2]]) * pow(2, uint64_double(ar[3])) + uint64_double(ar[4])));
        r[31]++;
        break;
    case MULD:
        r[ar[1]] = double_uint64(uint64_double(r[ar[1]]) * (uint64_double(r[ar[2]]) * pow(2, uint64_double(ar[3])) + uint64_double(ar[4])));
        r[31]++;
        break;
    case DIVD:
        r[ar[1]] = double_uint64(uint64_double(r[ar[1]]) / (uint64_double(r[ar[2]]) * pow(2, uint64_double(ar[3])) + uint64_double(ar[4])));
        r[31]++;
        break;
    case ITOD:
        r[ar[1]] = (double)((r[ar[2]] + r[ar[3]] << ar[4] + ar[5]) % 0x100000000);
        r[31]++;
        break;
    case DTOI:
        r[ar[1]] = (long long)uint64_double(uint64_double(r[ar[1]]) + (uint64_double(r[ar[2]]) * pow(2, uint64_double(ar[3])) + uint64_double(ar[4])));
        r[31]++;
        break;
    case BL:
        r[30] = r[31];
        if (count == 3)
        {
            r[31] = r[ar[1]] + ar[2];
        }
        else
        {

            r[31] = r[ar[1]] + (r[ar[2]] << ar[3]) + ar[4];
        }
        break;
    case CMP:
        int _f;
        if (count == 4)
        {
            _f = r[ar[1]] - r[ar[2]] - ar[3];
            if (_f < 0)
                flag = L;
            if (_f == 0)
                flag = EQ;
            if (_f > 0)
                flag = G;
        }
        else
        {
            _f = r[ar[1]] - (r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]));
            if (_f < 0)
                flag = L;
            if (_f == 0)
                flag = EQ;
            if (_f > 0)
                flag = G;
        }
        r[31]++;
        break;
    case CMPD:
        _doub = uint64_double(r[ar[1]]) - uint64_double(r[ar[2]]) - uint64_double(r[ar[3]]) * pow(2, uint64_double(ar[4])) - uint64_double(ar[5]);
        if (_doub < 0)
            flag = L;
        if (_doub == 0)
            flag = EQ;
        if (_doub > 0)
            flag = G;
        r[31]++;
        break;
    case CNE:
        if (flag != EQ)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case CEQ:
        if (flag == EQ)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case CLE:
        if (flag != G)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case CLT:
        if (flag == L)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case CGE:
        if (flag != L)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case CGT:
        if (flag == G)
        {
            if (count == 6)
            {
                r[ar[1]] = r[ar[2]] + (r[ar[3]] << ar[4] + ar[5]);
            }
            else
            {
                r[ar[1]] = ar[3];
            }
            if (ar[1] != reg_num("pc"))
            {
                r[31]++;
            }
        }
        else
        {
            r[31]++;
        }
        break;
    case LD:
        if (ar[2] == reg_num("pc") || ar[2] == reg_num("rz")) {
            r[ar[1]] = memory[r[ar[2]] + ar[3]+1];
            r[ar[1]]<<=32;
            r[ar[1]] += memory[r[ar[2]] + ar[3]];
            r[31]++;
        } else if (ar[2] == reg_num("sp")) {
            r[ar[1]] = memory[r[ar[2]]+1];
            r[ar[1]]<<=32;
            r[ar[1]] += memory[r[ar[2]]];
            r[ar[2]] += (ar[3]+3)/4;
            r[31]++;
        } else {
            int adress = 0;
            if (ar[3] == reg_num("rz")) {
                adress = r[ar[2]] + ar[4]; 
            } else {
                adress = r[ar[2]] + r[ar[3]]<<ar[4] + ar[5];
            }
            r[ar[1]] = memory[adress+1];
            r[ar[1]]<<=32;
            r[ar[1]] += memory[adress];
            r[31]++;
        }
        break;
    case ST:
        if (ar[2] == reg_num("pc") || ar[2] == reg_num("rz")) {
            memory[r[ar[2]] + ar[3]] = r[ar[1]];
            r[31]++;
        } else if (ar[2] == reg_num("sp")) {
            r[ar[2]] -= (ar[3]+3)/4;
            memory[r[ar[2]]] = r[ar[1]];
            memory[r[ar[2]]+1] = r[ar[1]]>>32;
            r[31]++;
        } else {
            int adress = 0;
            if (ar[3] == reg_num("rz")) {
                adress = r[ar[2]] + ar[4]; 
            } else {
                adress = r[ar[2]] + r[ar[3]]<<ar[4] + ar[5];
            }
            memory[adress] = r[ar[1]];
            memory[adress+1] = r[ar[1]]>>32;
            r[31]++;
        }
        break;
    default:
        break;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int _lab = 0;
    int _spl = 0;
    int _d = 0;
    int _tra = 0;
    if (argc > 1)
    {
        if (!strcmp(argv[1], "lab"))
            _lab = 1;
        if (!strcmp(argv[1], "spl"))
            _spl = 1;
        if (!strcmp(argv[1], "d"))
            _d = 1;
        if (!strcmp(argv[1], "tra"))
            _tra = 1;
    }
    //вывод разбиения строки
    int _splitter = 0;

    commands_init();

    r[29] = 524288 - 1;
    ifstream input("input.fasm");

    string str;
    string words[10];
    words[0] = "";
    words[1] = "";

    int _count;

    while (words[0] != "end" && words[1] != "end")
    {
        getline(input, str);
        _count = splitter(str, words);
        labs_init(_count, words);

        if (_spl)
        {
            for (int i = 0; i < _count; i++)
            {
                cout << "#" << words[i] << "# ";
            }
            cout << endl;
        }
    }
    input.close();

    if (_lab)
    {
        for (auto x : labels)
        {
            cout << x.first << " " << x.second << endl;
        }
    }

    input = ifstream("input.fasm");
    words[0] = "";
    words[1] = "";
    pc = 0;
    while (words[0] != "end" && words[1] != "end")
    {
        getline(input, str);
        _count = splitter(str, words);
        translate(_count, words);
    }
    input.close();

    if (_tra)
    {
        int ar[7];
        for (int i = 0; i < pc; i++)
        {
            int k = word_args(memory[i], ar);
            args_print(k, ar);
        }
    }

    while (1)
    {
        if (_d)
            debuggggg();

        if (exec() == -1)
        {
            return 0;
        }
    }
}