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

#ifndef RACONFIG_IPP
#define RACONFIG_IPP

#include <boost/program_options/parsers.hpp>
#include <iostream>

#ifndef RACONFIG_INLINE
#define RACONFIG_INLINE
#endif

namespace raconfig
{

RACONFIG_INLINE config_error::config_error(std::string const& what)
    : std::runtime_error{what}
{}

RACONFIG_INLINE config_error::config_error(const char* what)
    : std::runtime_error{what}
{}

namespace detail
{

RACONFIG_INLINE void parse_command_line(int argc, const char* const argv[],
        po::options_description const& desc,
        po::variables_map& vm)
{
    po::store(po::parse_command_line(argc, argv, desc), vm);
}

RACONFIG_INLINE void parse_config_file(const char *path,
        po::options_description const& desc,
        po::variables_map& vm)
{
    po::store(po::parse_config_file<char>(path, desc, false), vm);
}

RACONFIG_INLINE void throw_option_check_failed(const char *name, const char *value)
{
    std::string what = "the argument ('";
    what.append(value).append("') check for option '");
    what.append(name).append("' failed");
    throw config_error{what};
}

} // namespace detail

RACONFIG_INLINE void default_actions::help(detail::po::options_description const& desc)
{
    std::cout << desc << '\n';
    std::exit(EXIT_SUCCESS);
}

RACONFIG_INLINE void default_actions::version(const char* ver)
{
    std::cout << ver << '\n';
    std::exit(EXIT_SUCCESS);
}

RACONFIG_INLINE void default_actions::show_config_begin() {}

RACONFIG_INLINE void default_actions::show_config(const char* name, std::string value)
{
    std::cout << name << " = " << value << '\n';
}

RACONFIG_INLINE void default_actions::show_config(const char* name, std::vector<std::string> value)
{
    if (value.empty()) {
        std::cout << name << "[]\n";
    } else {
        size_t i = 0;
        for (auto const& v: value)
            std::cout << name << '[' << i++ << "] = " << v << '\n';
    }
}

RACONFIG_INLINE void default_actions::show_config_end()
{
    std::exit(EXIT_SUCCESS);
}

} // namespace raconfig

#endif
