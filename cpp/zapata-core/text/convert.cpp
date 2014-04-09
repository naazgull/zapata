#include <text/convert.h>

void zapata::tostr(string& s, int i){
	char oss[512];
	sprintf(oss,"%i", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(std::string& s, int i, std::ios_base& (&hex)(std::ios_base&)) {
	char oss[512];
	sprintf(oss,"%x", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, unsigned int i){
	char oss[512];
	sprintf(oss,"%u", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, size_t i){
	char oss[512];
	sprintf(oss,"%lu", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, long i){
	char oss[512];
	sprintf(oss,"%ld", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, long long i){
	char oss[512];
	sprintf(oss,"%lld", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, float i){
	char oss[512];
	sprintf(oss,"%f", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, double i){
	char oss[512];
	sprintf(oss,"%lf", i);
	s.insert(s.length(), oss);
}

void zapata::tostr(string& s, char i){
	char oss[512];
	sprintf(oss,"%c", i);
	s.insert(s.length(), oss);
}

void zapata::fromstr(string& s, int* i){
	sscanf(s.data(),"%i", i);
}

void zapata::fromstr(string& s, unsigned int* i){
	sscanf(s.data(),"%u", i);
}

void zapata::fromstr(string& s, size_t* i){
	sscanf(s.data(),"%lu", i);
}

void zapata::fromstr(string& s, long* i){
	sscanf(s.data(),"%ld", i);
}

void zapata::fromstr(string& s, long long* i){
	sscanf(s.data(),"%lld", i);
}

void zapata::fromstr(string& s, float* i){
	sscanf(s.data(),"%f", i);
}

void zapata::fromstr(string& s, double* i){
	sscanf(s.data(),"%lf", i);
}

void zapata::fromstr(string& s, char* i){
	sscanf(s.data(),"%c", i);
}

void zapata::fromstr(string& s, bool* i){
	*i = s == string("true");
}
