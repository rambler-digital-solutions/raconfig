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

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <raconfig/raconfig.hpp>
#include <raconfig/raconfig_set.hpp>

enum class color
{
    red,
    green,
    blue
};

std::istream& operator >> (std::istream& is, color& c);
std::ostream& operator << (std::ostream& os, color c);

namespace option
{

RACONFIG_OPTION(text, std::string, "default text",
    "text,t",           // --text=abc, -tabc
    "common.text",      // [common]
                        // text = abc
    "Some text")
RACONFIG_OPTION(number, unsigned short, 80,
    "number,n",         // --number=42, -n42
    "common.number",    // [common]
                        // number = 42
    "Unsigned short number")
RACONFIG_OPTION(flag, bool, false,
    "flag,f",           // --flag, -f
    "common.flag",      // [common]
                        // flag = 1
    "Boolean flag")
RACONFIG_OPTION_CHECKED(power2,
    RACONFIG_T(std::set<unsigned, std::greater<unsigned>>),
    RACONFIG_V({32, 64, 128}),
    [](std::set<unsigned, std::greater<unsigned>> const& v) {
        return std::all_of(v.begin(), v.end(), [](unsigned x) {
            return x > 0 && (x & (x - 1)) == 0;
        });
    },
    "power2",           // --power2=4 --power2=8 --power2=4
    "power2.item",      // [power2]
                        // item = 4
                        // item = 8
    "Power of 2 numbers")
RACONFIG_OPTION(color, ::color, ::color::red, // require stream >> and << operators
    "color",
    "common.color",
    "RGB color (red|green|blue)")

} // namespace option

struct actions: raconfig::default_actions
{
    void help(boost::program_options::options_description const& desc) override;
    void version(const char* ver) override;
    void show_config_begin() override;
};

using config = raconfig::config<actions,
    option::text,
    option::number,
    option::flag,
    option::power2,
    option::color>;

#endif
