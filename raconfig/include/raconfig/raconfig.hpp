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

#ifndef RACONFIG_HPP
#define RACONFIG_HPP

#include <boost/program_options/value_semantic.hpp>
#include <tuple>
#include "raconfig_range.hpp"

namespace raconfig
{

class config_error: public std::runtime_error
{
public:
    explicit config_error(std::string const& what);
    explicit config_error(const char* what);
};

namespace detail
{

#if __cplusplus < 201402L

template<size_t I, class T, class H, class ...Ts>
struct find_type
{
    static constexpr size_t index = find_type<I + 1, T, Ts...>::index;
};

template<size_t I, class T, class ...Ts>
struct find_type<I, T, T, Ts...>
{
    static constexpr size_t index = I;
};

template<class T, class ...Ts>
T const& get(std::tuple<Ts...> const& t) noexcept
{
    return std::get<find_type<0, T, Ts...>::index>(t);
}

template<class T, class ...Ts>
T& get(std::tuple<Ts...>& t) noexcept
{
    return std::get<find_type<0, T, Ts...>::index>(t);
}

#else

template<class T, class ...Ts>
T const& get(std::tuple<Ts...> const& t) noexcept { return std::get<T>(t); }

template<class T, class ...Ts>
T& get(std::tuple<Ts...>& t) noexcept { return std::get<T>(t); }

#endif

template<class T>
struct type_proxy
{
    static std::string to_string(T const& v)
    {
        return boost::lexical_cast<std::string>(v);
    }

    static void show_option(default_actions& actions, const char *name, T const& v)
    {
        return actions.show_config(name, to_string(v));
    }
};

template<class T>
std::string to_string(T const& v)
{
    return type_proxy<T>::to_string(v);
}

template<class T, class Allocator>
struct type_proxy<std::vector<T, Allocator>>:
    type_proxy_range<std::vector<T, Allocator>>
{};

template<class T>
class option_value: public option_value_backend<T>
{
public:
    option_value(T const& value)
        : option_value_backend<T>{value}
    {}

    option_value(T&& value)
        : option_value_backend<T>{std::move(value)}
    {}

    T const& operator ()(get_user_type) const noexcept { return **this; }
    void operator ()(transform_backend) { /* no transformation */ }
};

template<class T>
T deduce_value_backend_type(option_value_backend<T>&&) noexcept;

template<class Option>
using value_backend_type = decltype(deduce_value_backend_type(std::declval<Option>()));

class options_parser
{
public:
    options_parser(const char *header = "");

    options_parser(options_parser const&) = delete;
    options_parser& operator = (options_parser const&) = delete;
    options_parser(options_parser&&) = delete;
    options_parser& operator = (options_parser&&) = delete;

    void add(const char *name, const char *description, boost::program_options::value_semantic const *s = nullptr);
    void parse_command_line(int argc, const char* const argv[]);
    void parse_config_file(const char *path);
    void notify();

    bool has(const char *name) const;
    bool get(const char *name, std::string& value) const;

    operator boost::program_options::options_description const& () const noexcept;

    ~options_parser();

private:
    struct impl;
    impl *impl_;
};

struct name{};
struct cmd_name{};
struct cfg_name{};
struct description{};
struct check_value{};

template<class Option>
void show_option(default_actions& actions, Option const& option)
{
    auto& v = option(get_user_type{});
    return type_proxy<typename std::remove_const<
        typename std::remove_reference<decltype(v)>::type
            >::type>::show_option(actions, option(name{}), v);
}

template<class T>
constexpr bool skip_option_check(T&&) { return true; }

void throw_option_check_failed(const char *name, const char *value);

} // namespace detail

#if __cplusplus < 201703L
#define RACONFIG_FOLD(expr) { int _[] = {((expr), 0)...}; (void)_; }
#else
#define RACONFIG_FOLD(expr) (..., (expr))
#endif

#if __cplusplus < 201402L
namespace detail
{
template<class T>
T deduce_value_type(option_value<T>&&) noexcept;
}
#define RACONFIG_VALUE_TYPE(T) decltype(detail::deduce_value_type(std::declval<T>()))
#else
#define RACONFIG_VALUE_TYPE(T) auto
#endif

template<class Actions, class ...Ts>
class config final
{
    static_assert(std::is_base_of<default_actions, Actions>::value,
                  "Actions should derive from default_actions");

    using this_type = config<Actions, Ts...>;

public:
    static this_type& instance()
    {
        static this_type inst;
        return inst;
    }

    template<class T>
    RACONFIG_VALUE_TYPE(T) const& get() const noexcept
    {
        return detail::get<T>(options_)(detail::get_user_type{});
    }

    void parse_cmd_line(int argc, const char* const argv[])
    {
        try {
            parse_cmd_line_impl(argc, argv);
        } catch (config_error const& e) {
            throw;
        } catch (std::exception const& e) {
            std::throw_with_nested(config_error{e.what()});
        } catch (...) {
            std::throw_with_nested(config_error{"unknown exception"});
        }
        for (auto cb: callbacks_)
            cb();
    }

    void parse_file(const char *path)
    {
        const char* args[] = {"", "--config", path};
        parse_cmd_line(3, args);
    }

    void add_callback(void (*cb)())
    {
        callbacks_.push_back(cb);
    }

    struct callback
    {
        explicit callback(void (*cb)())
        {
            this_type::instance().add_callback(cb);
        }
    };

private:
    config() = default;
    config(config const&) = delete;
    config& operator = (config const&) = delete;
    config(config&&) = delete;
    config& operator = (config&&) = delete;

    void parse_cmd_line_impl(int argc, const char* const argv[])
    {
        std::tuple<Ts...> tmp;
        detail::options_parser p{"Allowed options"};
        p.add("help", "Show this message and exit");
#ifdef RACONFIG_VERSION_STRING
        p.add("version", "Show version and exit");
#endif
        p.add("show-config", "Show final configuration and exit");
        p.add("config", "Load options from file, command line options override ones from file",
              boost::program_options::value<std::string>());
        RACONFIG_FOLD(!detail::get<Ts>(tmp)(detail::cmd_name{}) ? (void)0
                      : p.add(detail::get<Ts>(tmp)(detail::cmd_name{}),
                              detail::get<Ts>(tmp)(detail::description{}),
                              boost::program_options::value(&*detail::get<Ts>(tmp))));
        p.parse_command_line(argc, argv);

        if (p.has("help"))
            Actions{}.help(p);
#ifdef RACONFIG_VERSION_STRING
        if (p.has("version"))
            Actions{}.version(RACONFIG_VERSION_STRING);
#endif
        std::string config;
        if (p.get("config", config)) {
            detail::options_parser p;
            RACONFIG_FOLD(!detail::get<Ts>(tmp)(detail::cfg_name{}) ? (void)0
                          : p.add(detail::get<Ts>(tmp)(detail::cfg_name{}),
                                  detail::get<Ts>(tmp)(detail::description{}),
                                  boost::program_options::value(&*detail::get<Ts>(tmp))));
            p.parse_config_file(config.c_str());
            p.notify();
        }

        // notify command line options after config
        p.notify();
        RACONFIG_FOLD(detail::get<Ts>(tmp)(detail::transform_backend{}));
        RACONFIG_FOLD(detail::get<Ts>(tmp)(detail::check_value{}));
        options_ = std::move(tmp);
        if (p.has("show-config")) {
            Actions actions;
            actions.show_config_begin();
            RACONFIG_FOLD(detail::show_option(actions, detail::get<Ts>(options_)));
            actions.show_config_end();
        }
    }

    std::tuple<Ts...> options_;
    std::vector<void(*)()> callbacks_;
};

} // namespace raconfig

#ifndef RACONFIG_LIB
#define RACONFIG_INLINE inline
#include "raconfig.ipp"
#endif

#define RACONFIG_T(...) __VA_ARGS__
#define RACONFIG_V(...) __VA_ARGS__
#define RACONFIG_NO_NAME nullptr

#define RACONFIG_OPTION_CHECKED(tag, type, default_value, pred, cmd_name_, cfg_name_, description_) \
    struct tag final: raconfig::detail::option_value<type> \
    { \
        using raconfig::detail::option_value<type>::operator (); \
        const char* operator ()(raconfig::detail::name) const noexcept { return #tag; } \
        const char* operator ()(raconfig::detail::cmd_name) const noexcept { return (cmd_name_); } \
        const char* operator ()(raconfig::detail::cfg_name) const noexcept { return (cfg_name_); } \
        const char* operator ()(raconfig::detail::description) const noexcept { return (description_); } \
        void operator ()(raconfig::detail::check_value) const \
        { \
            auto& v = (*this)(raconfig::detail::get_user_type{}); \
            if (!(pred)(v)) \
                raconfig::detail::throw_option_check_failed(#tag, \
                        raconfig::detail::to_string(v).c_str());  \
        } \
        tag(): raconfig::detail::option_value<type>{default_value} {} \
    };

#define RACONFIG_OPTION(tag, type, default_value, cmd_name_, cfg_name_, description_) \
    RACONFIG_OPTION_CHECKED(tag, RACONFIG_T(type), RACONFIG_V(default_value), \
            raconfig::detail::skip_option_check, cmd_name_, cfg_name_, description_)

#define RACONFIG_OPTION_EASY(tag, type, default_value, description_) \
    RACONFIG_OPTION(tag, RACONFIG_T(type), RACONFIG_V(default_value), #tag, #tag, description_)

#endif
