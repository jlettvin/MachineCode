/* MachineCode.cpp Copyright(c) Jonathan D. Lettvin, All Rights Reserved.
 * Author: Jonathan D. Lettvin
 * Original: 20150925
 * History: 20151005 converted to github project
 * License: GPL 3.0 General
 *
 * This code is designed to violate many principles of "good design practice".
 * byte composition is used to inject machine code as self-modifying code.
 * computed goto will be used.
 *
 * Data will be made available as global fields accessible to all code.
 * Protections and stack frame conventions are explicitly avoided.
 * Overhead accompanying conventions is eliminated wherever possible.
 *
 * Since this code is prototype, the looping mechanism has too much overhead.
 * This will be eliminated as the prototype matures to stage 2 of 6.
 *
 * At each stage of development, a complete rewrite of the code is allowed:
 * Stage 1: test implementability of an idea
 * Stage 2: make a useful program using the idea
 * Stage 3: make the language available for other developers to try out
 * Stage 4: convert to alpha for internal general use
 * Stage 5: convert to beta for friendly external users
 * Stage 6: convert to product for general distribution
 *
 * Typically, languages impose rules for "correct use" and offer protections.
 * These affect performance in significant ways.
 *
 * This language exposes the developer to extreme danger.
 * No protections are offered, and no performance reduction is imposed.
 * Spaghetti code is encouraged.  Self-modifying code is encouraged.
 * GOTO is encouraged.
 * Write code for the machine, not for the language author.
 */
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

typedef void (*fptr)();

const int stay[] = { SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGTERM, 0 };
const int quit[] = { SIGINT, 0 };
static int sigs[SIGSYS+1];

map<string,fptr> mnemonic;

int heard = 0;

class Machine {
    public:
        Machine();
        ~Machine();
        void BYTE(const unsigned char c);
        void WORD(const unsigned short s);
        void LONG(const unsigned long l);
        void QUAD(const unsigned long long q);
        void PTR(const void* p);
        void STR(const string& s);
        const fptr CODE();
        void operator()();
        ostream &print(ostream& stream);
        friend ostream& operator<< (ostream& stream, const Machine& me);
    protected:
    private:
        vector<unsigned char> vc;
        unsigned char* code;
};

Machine::Machine():code(0) {}
Machine::~Machine() { if(this->code != 0) free(this->code); }
void Machine::BYTE(const unsigned char c) { this->vc.push_back(c); }
void Machine::WORD(const unsigned short s) { }
void Machine::LONG(const unsigned long l) { }
void Machine::QUAD(const unsigned long long q) { }
void Machine::PTR(const void* p) { }
void Machine::STR(const string& s) { }
const fptr Machine::CODE() {
    const size_t len = this->vc.size();
    if(this->code) {
        free(this->code);
        this->code = (unsigned char*)malloc(len);
    }
    for(size_t i = 0; i < len; ++i) {
        this->code[i] = this->vc.size();
    }
    return (fptr)this->code;
}
void Machine::operator()() {
    vector<unsigned char>::iterator i;
    vector<unsigned char>::iterator a=this->vc.begin();
    vector<unsigned char>::iterator z=this->vc.end();
    this->CODE()();
}
ostream& Machine::print(ostream& stream) {
    vector<unsigned char>::iterator i;
    vector<unsigned char>::iterator a=this->vc.begin();
    vector<unsigned char>::iterator z=this->vc.end();
    for(i = a; i != z; i++) stream << hex << *i;
    return stream;
}
ostream& operator <<(ostream& stream, Machine& machine) {
    return machine.print(stream);
}

void signalHandler(int signum) {
    static const void *table[2] = { &&done, &&more };
    static const string how[2] = { "Fatal", "Allow" };
    cout << how[signum] << " signal(" << signum << ") received." << endl;
    goto *(table[sigs[signum]]);
done: exit(signum);
more: signal(heard = signum, signalHandler);
}

int main(int argc, char **argv) {

  for(int i = 0; stay[i]; i++) { signal(stay[i], signalHandler); sigs[i] = 1; }
  for(int i = 0; quit[i]; i++) { signal(quit[i], signalHandler);              }

  Machine RET;
  RET.BYTE(0xc3);

  mnemonic["RET"] = (fptr)"\xc3";
  mnemonic["NOPRET"] = (fptr)"\x90\xc3";
  mnemonic["STOP"] = (fptr)0;

  vector<string> P;
  vector<string>::iterator Pi;

  P.push_back("NOPRET");
  P.push_back("RET");
  P.push_back("STOP");

  cout << "vvvvv" << endl;
  try {
    for(Pi = P.begin(); Pi != P.end() && mnemonic[*Pi]; Pi++) {
        cout << *Pi << endl;
        (mnemonic[*Pi])();
    }
  }
  catch (const int     e) { cout << "int     exception: " << e << endl; }
  catch (const char    e) { cout << "char    exception: " << e << endl; }
  catch (const string &e) { cout << "string  exception: " << e << endl; }
  catch (...)             { cout << "default exception"        << endl; }
  cout << "^^^^^" << endl;
  exit(0);
}
