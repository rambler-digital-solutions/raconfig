//
// Copyright 2018 Rambler Digital Solutions
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "config.hpp"
#include <boost/program_options/options_description.hpp>
#include <iostream>

std::istream& operator >> (std::istream& is, color& c)
{
    std::string str;
    is >> str;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "red")
        c = color::red;
    else if (str == "green")
        c = color::green;
    else if (str == "blue")
        c = color::blue;
    else
        throw std::runtime_error{"Unknown color"};
    return is;
}

std::ostream& operator << (std::ostream& os, color c)
{
    switch (c) {
    case color::red:
        return os << "RED";
    case color::green:
        return os << "GREEN";
    case color::blue:
        return os << "BLUE";
    default:
        return os << "?";
    }
}

void actions::version(const char* ver)
{
    std::cout << "***********\n"
              << "* VERSION *\n"
              << "***********\n";
    raconfig::default_actions::version(ver);
}

void actions::help(boost::program_options::options_description const& desc)
{
    std::cout << "********\n"
              << "* HELP *\n"
              << "********\n";
    raconfig::default_actions::help(desc);
}

void actions::show_config_begin()
{
    std::cout << "***********\n"
              << "* OPTIONS *\n"
              << "***********\n";
    raconfig::default_actions::show_config_begin();
}

int main(int argc, char* argv[])
{
    // 1) parse cmd line options (may throw config_error exception)
    try {
        config::instance().parse_cmd_line(argc, argv);
        std::cout << "Config was successfully parsed" << std::endl;
    } catch (raconfig::config_error const& e) {
        std::cerr << "config_error: " << e.what() << std::endl;
        return 1;
    }
    // 2) access config options (no exceptions guarantee)
    config::instance().get<option::text>();
    config::instance().get<option::number>();
    config::instance().get<option::flag>();
    config::instance().get<option::power2>();
    return 0;
}
