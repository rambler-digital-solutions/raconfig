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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
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
namespace po = boost::program_options;

struct options_parser::impl
{
    impl(const char *header)
        : desc{header}
        , init{&desc}
    {}

    po::options_description desc;
    po::options_description_easy_init init;
    po::variables_map vm;
};

RACONFIG_INLINE options_parser::options_parser(const char *header)
{
    impl_ = new impl{header};
}

RACONFIG_INLINE void options_parser::add(const char *name, const char *description, po::value_semantic const *s)
{
    if (s == nullptr)
        impl_->init(name, description);
    else
        impl_->init(name, s, description);
}

RACONFIG_INLINE void options_parser::parse_command_line(int argc, const char* const argv[])
{
    impl_->vm.clear();
    po::store(po::parse_command_line(argc, argv, impl_->desc), impl_->vm);
}

RACONFIG_INLINE void options_parser::parse_config_file(const char *path)
{
    impl_->vm.clear();
    po::store(po::parse_config_file<char>(path, impl_->desc, false), impl_->vm);
}

RACONFIG_INLINE void options_parser::notify()
{
    po::notify(impl_->vm);
}

RACONFIG_INLINE bool options_parser::has(const char *name) const
{
    return impl_->vm.count(name) > 0;
}

RACONFIG_INLINE bool options_parser::get(const char *name, std::string& value) const
{
    po::variable_value const& vv = impl_->vm[name];
    if (!vv.empty()) {
        value = vv.as<std::string>();
        return true;
    }
    return false;
}

RACONFIG_INLINE options_parser::operator po::options_description const& () const noexcept
{
    return impl_->desc;
}

RACONFIG_INLINE options_parser::~options_parser()
{
    delete impl_;
}

RACONFIG_INLINE void throw_option_check_failed(const char *name, const char *value)
{
    std::string what = "the argument ('";
    what.append(value).append("') check for option '");
    what.append(name).append("' failed");
    throw config_error{what};
}

} // namespace detail

RACONFIG_INLINE void default_actions::help(boost::program_options::options_description const& desc)
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
