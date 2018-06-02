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

#ifndef RACONFIG_RANGE_HPP
#define RACONFIG_RANGE_HPP

namespace raconfig
{

struct default_actions
{
    virtual void help(boost::program_options::options_description const& desc);
    virtual void version(const char *ver);
    virtual void show_config_begin();
    virtual void show_config(const char* name, std::string value);
    virtual void show_config(const char* name, std::vector<std::string> value);
    virtual void show_config_end();
};

namespace detail
{

template<class T>
class option_value;

template<class T>
std::string to_string(T const& v);

template<class Iter>
std::string to_string(Iter first, Iter last)
{
    std::string s = "{";
    if (first != last) {
        s += to_string(*first++);
        while (first != last) {
            s += ", ";
            s += to_string(*first++);
        }
    }
    return s += '}';
}

template<class Iter>
void show_option(default_actions& actions, const char *name, Iter first, Iter last)
{
    std::vector<std::string> vs;
    while (first != last)
        vs.push_back(to_string(*first++));
    actions.show_config(name, std::move(vs));
}

template<class T>
struct type_proxy_range
{
    static std::string to_string(T const& v)
    {
        return detail::to_string(std::begin(v), std::end(v));
    }

    static void show_option(default_actions& actions, const char *name, T const& v)
    {
        return detail::show_option(actions, name, std::begin(v), std::end(v));
    }
};

template<class T>
class option_value_backend
{
    static_assert(std::is_same<T, typename std::remove_cv<typename
            std::remove_reference<T>::type>::type>::value,
            "CVRef type is not allowed!");
public:
    option_value_backend(T const& value)
        : value_{value}
    {}

    option_value_backend(T&& value)
        : value_{std::move(value)}
    {}

    T const& operator *() const noexcept { return value_; }
    T& operator *() noexcept { return value_; }

private:
    T value_;
};

struct get_user_type{};
struct transform_backend{};

template<class T, class U>
class convertible_option_value: public option_value_backend<U>
{
public:
    convertible_option_value(T const& value)
        : option_value_backend<U>{U{std::begin(value), std::end(value)}}
        , value_{value}
    {}

    convertible_option_value(T&& value)
        : option_value_backend<U>{U{std::begin(value), std::end(value)}}
        , value_{std::move(value)}
    {}

    T const& operator ()(get_user_type) const noexcept { return value_; }

    void operator ()(transform_backend)
    {
        auto& from = **this;
        value_ = T{std::begin(from), std::end(from)};
    }

private:
    T value_;
};

} // namespace detail
} // namespace raconfig

#endif