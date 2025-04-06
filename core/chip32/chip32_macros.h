/*
The MIT License

Copyright (c) 2022 Anthony Rabine

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include <iostream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>

#include <fstream>

#include "chip32_assembler.h"

namespace Chip32
{

struct Macro {
    int param_count;
    std::vector<std::string> body;
};

class ScriptProcessor {
public:
    void process(const std::string &text) {

        resultAsm.clear();

        std::stringstream data_stream(text);
        std::string line;

        while(std::getline(data_stream, line))
        {
            line = trim(line);
            parseLine(line);
        }
    }

    // Return the expanded assembly file
    // without any macro directives
    std::string GetResult() const {
        return resultAsm;
    }

    void generate_assembly() {
        
        std::stringstream out;
    
        for (const std::string& line : text_section) {
            out << line << "\n";
        }
    
        for (const std::string& line : data_section) {
            out << line << "\n";
        }
        resultAsm = out.str();
    }

private:
    std::stack<int> conditionStack;
    std::stack<int> loopStack;
    std::string resultAsm;
    Macro* current_macro = nullptr;


    std::unordered_map<std::string, Macro> macros;
    std::vector<std::string> text_section;
    std::vector<std::string> data_section;
    bool in_data_section = false;
    bool in_macro_section = false;


    int labelCounter = 0;
    int printCounter = 0;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t");
        return str.substr(first, (last - first + 1));
    }


    std::string expand_macro(const std::string& line, const std::vector<std::string>& args) {
        std::string expanded = line;
        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "%" + std::to_string(i + 1);
            expanded = std::regex_replace(expanded, std::regex(placeholder), args[i]);
        }
        return expanded;
    }

    void createLabel()
    {
        int label = labelCounter++;
        loopStack.push(label);
        std::cout << "L" << label << "_START:" << std::endl;
    }

    void parseLine(std::string& line) {
        line = std::regex_replace(line, std::regex("^ +| +$"), ""); // Trim spaces
        if (line.empty() || line[0] == ';') return;

        if ((line.find("%macro") == 0)) {

            if (!in_macro_section) {
                std::cerr << "Error: Macros must be located in macro section\n";
                exit(1);
            }

            std::istringstream ss(line);
            std::string _, name;
            int param_count;
            ss >> _ >> name >> param_count;
            macros[name] = { param_count, {} };
            current_macro = &macros[name];
        } else if (line.find("%endmacro") == 0) {
            current_macro = nullptr;
        } else if (current_macro) {
            current_macro->body.push_back(line);
        } else if (line.find("%section_macro") == 0) {
            in_macro_section = true;

        } else if (line.find("%section_text") == 0) {
     
            in_macro_section = false;
            in_data_section = false;
        } else if (line.find("%section_data") == 0) {
    
            in_macro_section = false;
            in_data_section = true;
        } else {
            std::vector<std::string> args = Assembler::Split(line);

            if (args.size() > 0) {
                std::string name = args[0];
                args.erase(args.begin());
                
                if ((name.find("DC") == 0 || name.find("DV") == 0) && !in_data_section) {
                    std::cerr << "Error: Data declarations (DC/DV) must be inside section .data\n";
                    exit(1);
                }

                if (macros.find(name) != macros.end()) {
                    Macro& macro = macros[name];
                    
                    if (args.size() != macro.param_count) {
                        std::cerr << "Error: Macro " << name << " expects " << macro.param_count << " arguments, got " << args.size() << "\n";
                        return;
                    }
                    for (const std::string& body_line : macro.body) {
                        text_section.push_back(expand_macro(body_line, args));
                    }
                } else {
                    if (in_data_section) {
                        data_section.push_back(line);
                    } else {
                        text_section.push_back(line);
                    }
                }
            }
            else
            {
                std::cerr << "Macro problem with this line\n";
                exit(1);
            }
        }

    }

};

} // namespace Chip32
