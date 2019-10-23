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

std::unordered_map<std::string,std::string> message_map;
std::unordered_map<int,field*> field_map;

bool verbose(false);

void split(const std::string& line, std::vector<std::string>& v, char delim) {
       std::size_t current,previous=0;
        current = line.find(delim);
        while (current != std::string::npos) {
                v.push_back(line.substr(previous, current-previous));
                previous = current+1;
                current = line.find(delim, previous);
        }
        v.push_back(line.substr(previous, current-previous));
}

void parse_dictionary(const std::string dict) {
        std::ifstream ifs(dict);
        std::string line;
        field* current_field = NULL;

        while (std::getline(ifs, line)) {
                //std::cout << line << "\n";

                std::size_t beg = line.find(MSG_TAG);
                std::size_t end;
                if (beg != std::string::npos) {
                        beg += MSG_TAG.length();
                        end = line.find("\"", beg);
                        std::string message_name = line.substr(beg, end-beg);
                        std::cout << "message:" << line.substr(beg,end-beg) << "\n";
                        beg = line.find(MSG_TYPE, end);
       beg += MSG_TYPE.length();
                        end = line.find("\"", beg);
                        std::string message_type = line.substr(beg, end-beg);
                        std::cout << "type:" << line.substr(beg,end-beg) << "\n";
                        message_map[message_type] = message_name;
                        continue;
                }

                beg = line.find(FIELD_TAG);
                if (beg != std::string::npos) {
                        beg += FIELD_TAG.length();
                        end = line.find("\"", beg);
                        int field_num = std::stoi(line.substr(beg,end-beg));
                        std::cout << "field:" << field_num << "\n";
                        beg = line.find(FIELD_NAME, end);
                        beg += FIELD_NAME.length();
                        end = line.find("\"", beg);
                        std::string field_name = line.substr(beg, end-beg);
                        std::cout << "field name:" << field_name << "\n";
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
                        std::cout << "value: " << value << ", description: " << description << "\n";
                        if (current_field != NULL) {
                                current_field->values[value] = description;

                        }
                        continue;
                }
        }
}


char* get_option(char** beg, char** end, const std::string& opt) {
        char** it = std::find(beg, end, opt);
        if (it != end && ++it != end) {
                return *it;
        }
        return NULL;
}

int main(int argc, char* argv[]) {
        char* dictionary = get_option(argv, argv+argc, "-d");
        if (dictionary) {
                parse_dictionary(std::string(dictionary));

        }
}
                                                                                        
