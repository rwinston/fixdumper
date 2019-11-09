#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <cmath>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <cmath>
#include <numeric>
#include <set>

const std::string RED("\033[0;31m");  
const std::string RED_BOLD("\033[1;31m");    
const std::string YELLOW("\033[0;33m");
const std::string YELLOW_BOLD("\033[1;33m"); 
const std::string CYAN("\033[0;36m"); 
const std::string WHITE("\033[0;37m");
const std::string WHITE_BOLD("\033[1;37m");
const std::string GREEN("\033[0;32m");
const std::string GREEN_BOLD("\033[1;32m");  
const std::string RESET("\e[m");  

const std::string COLOR_MESSAGE=WHITE_BOLD;
const std::string COLOR_FIELD_NAME=GREEN;
const std::string COLOR_FIELD_VALUE=YELLOW;
const std::string COLOR_SENDER_TARGET=RED_BOLD;



struct field {
	int tag;
	std::string name;
	std::unordered_map<std::string,std::string> values;
};

const static std::string MSG_TAG("<message name=\"");
const static std::string MSG_TYPE("msgtype=\"");
const static std::string FIELD_TAG("<field number=\"");
const static std::string FIELD_NAME("name=\"");
const static std::string ENUM_TAG("<value enum=\"");
const static std::string DESC("description=\"");

const static char FS=0x01;
const static char TS='=';

const static std::string FIX_BEGIN("8=FIX");
const static std::string TAG_MSGTYPE("35");
const static std::string TAG_SENDERCOMPID("49");
const static std::string TAG_TARGETCOMPID("56");

const std::string SEPARATOR = "--------------------------------------------";

std::unordered_map<std::string,std::string> message_map;
std::unordered_map<int,field*> field_map;

bool verbose(false);
bool color(false);

void split(const std::string& line, std::vector<std::string>& v, char delim) {
	std::size_t current,previous=0;
	current = line.find(delim);
	while (current != std::string::npos) {
	        if (verbose) std::cout << "Split: cur=" << current << ", prev=" << previous << " (line=" << line << ")" << std::endl;
		v.push_back(line.substr(previous, current-previous));
		previous = current+1;
		if (verbose) std::cout << "Searching for " << (int)delim << " from index " << previous << std::endl;
		current = line.find(delim, previous);
		if (verbose) std::cout << "find() returned " << current << " (prev=" << previous << ")" << std::endl;
	}
	v.push_back(line.substr(previous, current-previous));
}

void parse_dictionary(const std::string dict) {
	std::ifstream ifs(dict);
	std::string line;
	field* current_field = NULL;	

	while (std::getline(ifs, line)) {
	        if (verbose) std::cout << "parsing line " << line << "\n";	
		std::size_t beg = line.find(MSG_TAG);
		std::size_t end;
		if (beg != std::string::npos) {
			beg += MSG_TAG.length();
			end = line.find("\"", beg);
			std::string message_name = line.substr(beg, end-beg);
			beg = line.find(MSG_TYPE, end);
			beg += MSG_TYPE.length();
			end = line.find("\"", beg);
			std::string message_type = line.substr(beg, end-beg);
			message_map[message_type] = message_name;
			continue;	
		}

		beg = line.find(FIELD_TAG);
		if (beg != std::string::npos) {
			beg += FIELD_TAG.length();
			end = line.find("\"", beg);
			int field_num = std::stoi(line.substr(beg,end-beg));
			beg = line.find(FIELD_NAME, end);
			beg += FIELD_NAME.length();
			end = line.find("\"", beg);
			std::string field_name = line.substr(beg, end-beg);
			field* f = new field();
			f->tag=field_num; f->name = field_name;
			field_map[field_num] = f;
			continue;
		}		

		beg = line.find(ENUM_TAG);
		if (beg != std::string::npos) {
			beg += ENUM_TAG.length();
			end = line.find("\"", beg);
			std::string value = line.substr(beg, end-beg);
			beg = line.find(DESC, end);
			beg += DESC.length();
			end = line.find("\"", beg);
			std::string description = line.substr(beg,end-beg);
			if (current_field != NULL) {
				current_field->values[value] = description;
			
			}
			continue;
		}
	}
	if(verbose) std::cout << "parsed dictionary\n"; 
}


void cleanup() {
	std::unordered_map<int, field*>::iterator it;
	for (it = field_map.begin(); it != field_map.end(); it++) {
		delete it->second;
	}
}



void parse_log() {	
	std::string line;
	while (std::getline(std::cin, line)) {
		if (verbose) std::cout << "Parsing FIX: " << line << std::endl;
		std::vector<std::string> tags;
		size_t pos = line.find(FIX_BEGIN);
		if (verbose) std::cout << "Found FIX tag at " << pos << std::endl;
		if (pos != std::string::npos && pos >= 0) {
			line = pos == 0 ? line : line.substr(pos, std::string::npos);
		}
		std::cout << SEPARATOR << "\n";
		split(line, tags, FS);
		if (verbose) std::cout << "Split tags" << std::endl;

		std::string msg,sender,target;
		for (int i=0;i<tags.size()-1;++i) {
		   std::vector<std::string> kv;
		   if (verbose) std::cout << "Parsing tag " << tags[i] << std::endl;
		   split(tags[i], kv, TS);
		   std::string k=kv[0], v = kv[1];
	           if (k==TAG_MSGTYPE) {
		   	msg=message_map[v];
		   }
		   else if (k==TAG_SENDERCOMPID) {
		   	sender = v;
		   }
		   else if (k==TAG_TARGETCOMPID) {
		   	target = v;
		   }
		}
		
		std::cout << (color?COLOR_MESSAGE:"") << msg << "\t" 
			<< (color?COLOR_SENDER_TARGET:"") << sender
			<< (color?RED:"") << " -> "
			<< (color?COLOR_SENDER_TARGET:"") << target << "\n";


		for (int i=0;i<tags.size()-1;++i) {
			std::vector<std::string> kv;
			split(tags[i], kv, TS); 
			if (verbose) std::cout << "Split tag: key:" << kv[0] << ",val: " << kv[1] << std::endl;
			std::string k=kv[0], v = kv[1];
			int field_tag = std::stoi(k);
			field* f = field_map[field_tag];
			if (verbose) std::cout << "Field map for tag: " << field_map[field_tag] << std::endl;
			
			std::string field_name = (f ? f->name : kv[0]);
			std::cout << std::left << std::setw(1) << (color?COLOR_FIELD_NAME:"") << field_name;
			std::cout << std::left << std::setw(1) << " | ";
			std::string val = v; 
			if (f && !(f->values.empty())) {
				val = f->values[v];
			}
			std::cout << (color?COLOR_FIELD_VALUE:"") << val << "\n";
		}
	}
	if (color) std::cout << RESET;
}

char* get_option(char** beg, char** end, const std::string& opt) {
	char** it = std::find(beg, end, opt);
	if (it != end && ++it != end) {
		return *it;	
	}
	return NULL;
}

bool has_option(char** beg, char** end, const std::string& opt) {
	char** it = std::find(beg, end, opt);
	return it!=end;	
}

int main(int argc, char* argv[]) {
	std::ios::sync_with_stdio(false);
	color = has_option(argv, argv+argc, "-c");
	verbose = has_option(argv, argv+argc, "-v");
	char* dictionary = get_option(argv, argv+argc, "-d");
	if (dictionary) {
		parse_dictionary(std::string(dictionary));
	}
	parse_log();	
	cleanup();
	return 0;
}
