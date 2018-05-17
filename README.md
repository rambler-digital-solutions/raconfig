# Declarative configuration wrapper on Boost.Program Options

## Motivation

**Boost.Program Options** is a very good library for getting options from command line, text file or even process environment. But it also requires to write imperative code to setup parsers and very often to wrap all of that stuff into configuration singleton with trivial (or not) getters. Writing configuration for a new project often comes to a copying-pasting-modifying of code from a previous project.

Writing configuration should be a trivial task without code bloating. The main idea is to declare some strongly typed options (neither command line specific nor file specific but general)

```cpp
RACONFIG_OPTION(T1)
RACONFIG_OPTION(T2)
...
RACONFIG_OPTION(Tn)
```

and bind them to the generic configuration singleton.

```cpp
using config = raconfig::config<T1, T2, ..., Tn>;
```

Accessing each option is type safe thus you can't access an unknown option. Also changing option type to non compatible with depending code causes compilation to fail (or to produce a warning message at least).

```cpp
config::instance().get<Tn>();
```

## Quick start

### Hello World

```cpp
#define RACONFIG_VERSION_STRING "MyApp 1.0.0"
#include <raconfig/raconfig.hpp>
#include <iostream>

namespace option
{
RACONFIG_OPTION_EASY(host, RACONFIG_T(std::string), RACONFIG_V("localhost"),
                     "Listening host")
RACONFIG_OPTION_EASY(port, RACONFIG_T(unsigned short), RACONFIG_V(80),
                     "Listening port")
}

using config = raconfig::config<raconfig::default_actions,
                                option::host, option::port>;

int main(int argc, char* argv[])
{
    auto& cfg = config::instance();
    try {
        cfg.parse_cmd_line(argc, argv);
    } catch (raconfig::config_error const& e) {
        std::cerr << "config_error: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Running on " << cfg.get<option::host>()
              << ':' << cfg.get<option::port>() << '\n';
}
```

Now you can run compiled program with declared options.

```sh
$ ./a.out --host=127.0.0.1 --port=8080
Running on 127.0.0.1:8080
```

### Predefined options

Raconfig adds predefined command line options: `help`, `version`, `show-config`, `config`. By default they print something to the standard output and terminate process.

```
$ ./a.out --help
Allowed options:
  --help                Show this message and exit
  --version             Show version and exit
  --show-config         Show final configuration and exit
  --config arg          Load options from file, command line options override
                        ones from file
  --host arg            Listening host
  --port arg            Listening port
```

`version` option depends on definition of `RACONFIG_VERSION_STRING` macro. Option is not visible if the macro is not defined before `raconfig/raconfig.hpp` is included. Very often version is determined before actual compilation (somewhere in CMake script, for example) so there is no need to define `RACONFIG_VERSION_STRING` explicitly in source file.

```
$ ./a.out --version
MyApp 1.0.0
```

`show-config` option dumps config instance state to the standard output stream.

```
$ ./a.out --show-config --host=192.168.1.1
host = 192.168.1.1
port = 80
```

`config` option specifies path to a configuration file. Configuration file format is a simple INI-like format acceptable by **Boost.Program Options** [configuration file parser](https://www.boost.org/doc/libs/1_54_0/doc/html/program_options/overview.html#idp123376208). For example:

```ini
# listening host
host = host.from.file
# listening port
port = 12345
```

You can mix declaration of `config` with other command line options. Command line options have higher priority and override ones from file.

```
$ ./a.out --show-config --config=config.ini --host=host.from.cmd.line
host = host.from.cmd.line
port = 12345
```

You can change program reaction on predefined command line options deriving your custom type from `raconfig::default_actions` and overriding needed methods. See demo project for details.

## Customizing options look & feel

### Change options layout

`RACONFIG_OPTION_EASY` is the shortended version of `RACONFIG_OPTION` macro. `RACONFIG_OPTION` allows you to specify custom option name for command line and file sources respectively. If an option shouldn't be available in command line or file or both, use `RACONFIG_NO_NAME` replacement. In the following example `option::backlog` is accessable only via configuration file.

```cpp
#include <raconfig/raconfig.hpp>
#include <iostream>

namespace option
{
RACONFIG_OPTION(host, RACONFIG_T(std::string), RACONFIG_V("localhost"),
                "server-host", "server.host",
                "Listening host")
RACONFIG_OPTION(port, RACONFIG_T(unsigned short), RACONFIG_V(80),
                "server-port", "server.port",
                "Listening port")
RACONFIG_OPTION(backlog, RACONFIG_T(unsigned), RACONFIG_V(512),
                RACONFIG_NO_NAME, "server.backlog",
                "Connection backlog")
}

using config = raconfig::config<raconfig::default_actions,
                                option::host, option::port, option::backlog>;

int main(int argc, char* argv[])
{
    auto& cfg = config::instance();
    try {
        cfg.parse_cmd_line(argc, argv);
    } catch (raconfig::config_error const& e) {
        std::cerr << "config_error: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Running on " << cfg.get<option::host>()
              << ':' << cfg.get<option::port>()
              << " [" << cfg.get<option::backlog>() << "]\n";
}
```

For the following configuration file

```ini
[server]
host = 11.12.13.14
port = 2012
backlog = 2048
```

expected output is

```
$ ./a.out --config=config.ini
Running on 11.12.13.14:2012 [2048]
```

### Add value constraints

Raconfig allows you to specify option value verifier via `RACONFIG_OPTION_CHECKED` macro. The 4th parameter is the callable (normal function or in-place lambda) returning `true` if an input value is correct. If the callable returns `false` exception `raconfig::config_error` is thrown.

```cpp
bool check_port(unsigned short v) { return v == 80 || v > 1023; }

namespace option
{
RACONFIG_OPTION(host, RACONFIG_T(std::string), RACONFIG_V("localhost"),
                "server-host", "server.host",
                "Listening host")
RACONFIG_OPTION_CHECKED(port, RACONFIG_T(unsigned short), RACONFIG_V(80),
                &check_port,
                "server-port", "server.port",
                "Listening port")
RACONFIG_OPTION_CHECKED(backlog, RACONFIG_T(unsigned), RACONFIG_V(512),
                [](unsigned v) { return 0 < v && v <= 4096; },
                RACONFIG_NO_NAME, "server.backlog",
                "Connection backlog")
}
```

## Using STL containers

### vector

`std::vector` is natively supported by **Boost.Program Options** library and Raconfig handles such option as well.

```cpp
namespace option
{
// ...
RACONFIG_OPTION(blacklist, RACONFIG_T(std::vector<std::string>),
                RACONFIG_V({"localhost", "127.0.0.1"}),
                "blackhost", "blacklist.item", "List of dangerous hosts")
// ...
}

using config = raconfig::config<raconfig::default_actions, option::host,
                                option::port, option::backlog, option::blacklist>;
```

```
$ ./a.out --show-config
host = localhost
port = 80
backlog = 512
blacklist[0] = localhost
blacklist[1] = 127.0.0.1
```

### set, multiset, unordered_set, unordered_miltiset

**Boost.Program Options** library doesn't support ordered/unordered set containers out of the box but you can use them with Raconfig. `set/multiset` and `unordered_set/unordered_multiset` are available in modules `raconfig/raconfig_set.hpp` and `raconfig/raconfig_unordered_set.hpp` respectively.

```cpp
#include <raconfig/raconfig.hpp>
#include <raconfig/raconfig_set.hpp>

namespace option
{
// ...
RACONFIG_OPTION(blacklist,
                RACONFIG_T(std::set<std::string, std::less<std::string>>),
                RACONFIG_V({}),
                "blackhost", "blacklist.item", "List of dangerous hosts")
// ...
}
```

```
$ ./a.out --show-config --blackhost=h1 --blackhost=h2 --blackhost=h1
host = localhost
port = 80
backlog = 512
blacklist[0] = h1
blacklist[1] = h2
```

Raconfig option with ordered/unordered set type is based on the standard vector performing appropriate conversion after successful parsing. Thus verifier for such option should accept a reference to `std::vector<T, Allocator>`.

```cpp
bool check_host(std::string const& v)
{
    return std::all_of(v.begin(), v.end(), [](char c) {
        return c == '.' || ::isalnum(c);
    });
}

namespace option
{
// ...
RACONFIG_OPTION_CHECKED(blacklist,
                RACONFIG_T(std::set<std::string, std::less<std::string>>),
                RACONFIG_V({}),
                [](std::vector<std::string> const& v) {
                    return std::all_of(v.begin(), v.end(), &check_host);
                },
                "blackhost", "blacklist.item", "List of dangerous hosts")
// ...
}
```

## Config update callbacks

Raconfig supports callbacks on configuration changes. Callbacks allow to initialize different subsystems locally without bloating the main function. Config parsing is assumed to happen in main thread before any action thus callbacks are not thread safe.

```cpp
using config = raconfig::config<raconfig::default_actions, ...>;

// global variable cb
config::callback const cb([]() {
    auto& cfg = config::instance();
    set_listen_to(cfg.get<option::host>(), cfg.get<option::port>(),
                  cfg.get<option::backlog>());
});
```