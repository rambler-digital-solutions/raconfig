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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE raconfig
#define RACONFIG_VERSION_STRING "version test"
#include <boost/test/unit_test.hpp>
#include <raconfig/raconfig.hpp>
#include <raconfig/raconfig_set.hpp>
#include <raconfig/raconfig_unordered_set.hpp>
#include <fstream>

namespace option
{

RACONFIG_OPTION(text, std::string, "default text",
    "text", "common.text", "Some text")
RACONFIG_OPTION(number, unsigned short, 80,
    "number", "common.number", "Unsigned short number")
RACONFIG_OPTION(flag, bool, false,
    "flag,f", "common.flag", "Boolean flag")
RACONFIG_OPTION(cmd_only_int, int, 100,
    "cmd-only-int", RACONFIG_NO_NAME, "Can be set via command line only")
RACONFIG_OPTION(cfg_only_int, int, 500,
    RACONFIG_NO_NAME, "cfg_only_int", "Can be set via config file only")
RACONFIG_OPTION_CHECKED(power2, std::vector<unsigned>, {},
    [](std::vector<unsigned> const& v) {
        for (unsigned x : v)
            if (x == 0 || (x & (x - 1)) != 0)
                return false;
        return true;
    },
    "power2", "power2.item", "Power of 2 numbers")

} // namespace option

struct actions: raconfig::default_actions
{
    static std::string res;

    void help(boost::program_options::options_description const& desc) override
    {
        std::stringstream ss;
        ss << desc;
        res = ss.str();
    }

    void version(const char* ver) override
    {
        res = ver;
    }

    void show_config_begin() override
    {
        res = "# config begin\noptions:\n";
    }

    void show_config(const char* name, std::string value) override
    {
        res.append(" ").append(name).append(": ").append(value).append("\n");
    }

    void show_config(const char* name, std::vector<std::string> value) override
    {
        res.append(" ").append(name).append(":\n");
        for (auto& str: value)
            res.append("  - ").append(str).append("\n");
    }

    void show_config_end() override
    {
        res.append("# config end\n");
    }
};

std::string actions::res;

using config = raconfig::config<actions,
    option::text,
    option::number,
    option::flag,
    option::cmd_only_int,
    option::cfg_only_int,
    option::power2>;

template<class T>
struct basic_file_fixture
{
    basic_file_fixture()
    {
        std::remove("test.ini");
        std::ofstream file;
        file.exceptions(std::ios_base::failbit);
        file.open("test.ini");
        static_cast<T*>(this)->write(file);
    }
};

BOOST_AUTO_TEST_SUITE(easy_test_suite)

RACONFIG_OPTION_EASY(name, std::string, "", "Option tag")
RACONFIG_OPTION_EASY(cmd_name, std::string, "", "Command line name")
RACONFIG_OPTION_EASY(cfg_name, std::string, "", "Config file name")
RACONFIG_OPTION_EASY(description, std::string, "", "Option description")
RACONFIG_OPTION_EASY(check, bool, true, "Validity")
RACONFIG_OPTION_EASY(value, int, 0, "Value")
RACONFIG_OPTION_EASY(value_type, int, 0, "Value type")

using config = raconfig::config<raconfig::default_actions, name, cmd_name,
                                cfg_name, description, check,
                                value, value_type>;

BOOST_AUTO_TEST_CASE(test_cmd_line)
{
    const char *argv[] = {"",
        "--name=NAME1",
        "--cmd_name=CMD_NAME1",
        "--cfg_name=CFG_NAME1",
        "--description=DESCRIPTION1",
        "--check=off",
        "--value=123",
        "--value_type=456"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(8, argv);
    BOOST_CHECK_EQUAL(cfg.get<name>(), "NAME1");
    BOOST_CHECK_EQUAL(cfg.get<cmd_name>(), "CMD_NAME1");
    BOOST_CHECK_EQUAL(cfg.get<cfg_name>(), "CFG_NAME1");
    BOOST_CHECK_EQUAL(cfg.get<description>(), "DESCRIPTION1");
    BOOST_CHECK_EQUAL(cfg.get<check>(), false);
    BOOST_CHECK_EQUAL(cfg.get<value>(), 123);
    BOOST_CHECK_EQUAL(cfg.get<value_type>(), 456);
}

struct easy_file_fixture: basic_file_fixture<easy_file_fixture>
{
    void write(std::ostream& file)
    {
        file << "name=NAME2\n"
                "cmd_name=CMD_NAME2\n"
                "cfg_name=CFG_NAME2\n"
                "description=DESCRIPTION2\n"
                "check=0\n"
                "value=135\n"
                "value_type=246\n";
    }
};

BOOST_FIXTURE_TEST_CASE(test_cfg_file, easy_file_fixture)
{
    const char *argv[] = {"",
        "--config=test.ini"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(2, argv);
    BOOST_CHECK_EQUAL(cfg.get<name>(), "NAME2");
    BOOST_CHECK_EQUAL(cfg.get<cmd_name>(), "CMD_NAME2");
    BOOST_CHECK_EQUAL(cfg.get<cfg_name>(), "CFG_NAME2");
    BOOST_CHECK_EQUAL(cfg.get<description>(), "DESCRIPTION2");
    BOOST_CHECK_EQUAL(cfg.get<check>(), false);
    BOOST_CHECK_EQUAL(cfg.get<value>(), 135);
    BOOST_CHECK_EQUAL(cfg.get<value_type>(), 246);
}

BOOST_AUTO_TEST_SUITE_END() // easy_test_suite

BOOST_AUTO_TEST_SUITE(containers_test_suite)

bool is_odd(std::vector<int> const& numbers)
{
    return std::all_of(numbers.begin(), numbers.end(), [](int x) {
        return x % 2 != 0;
    });
}

RACONFIG_OPTION_EASY(def_vector_1,
    RACONFIG_T(std::vector<int, std::allocator<int>>), RACONFIG_V({1, 2, 3}),
    "Default vector 1")
RACONFIG_OPTION(def_vector_2,
    RACONFIG_T(std::vector<int, std::allocator<int>>), RACONFIG_V({4, 5, 6}),
    RACONFIG_NO_NAME, RACONFIG_NO_NAME, "Default vector 2")
RACONFIG_OPTION_CHECKED(def_vector_3,
    RACONFIG_T(std::vector<int, std::allocator<int>>), RACONFIG_V({7, 8, 9}), &is_odd,
    RACONFIG_NO_NAME, RACONFIG_NO_NAME, "Default vector 3")
RACONFIG_OPTION_CHECKED(vector, std::vector<int>, {}, &is_odd,
    "vector-item", RACONFIG_NO_NAME, "Vector")
RACONFIG_OPTION_CHECKED(set, std::set<int>, {}, &is_odd,
    "set-item", RACONFIG_NO_NAME, "Ordered set")
RACONFIG_OPTION_CHECKED(default_set, RACONFIG_T(std::set<int, std::greater<int>>),
    RACONFIG_V({1, 3}), &is_odd,
    RACONFIG_NO_NAME, RACONFIG_NO_NAME, "Default ordered set")
RACONFIG_OPTION_CHECKED(multiset, std::multiset<int>, {}, &is_odd,
    "multiset-item", RACONFIG_NO_NAME, "Ordered multiset")
RACONFIG_OPTION_CHECKED(unordered_set, std::unordered_set<int>, {}, &is_odd,
    "unordered-set-item", RACONFIG_NO_NAME, "Unordered set")
RACONFIG_OPTION_CHECKED(unordered_multiset, std::unordered_multiset<int>, {}, &is_odd,
    "unordered-multiset-item", RACONFIG_NO_NAME, "Unordered multiset")

using config = raconfig::config<raconfig::default_actions,
                                def_vector_1, def_vector_2, def_vector_3, vector,
                                set, default_set, multiset, unordered_set, unordered_multiset>;

BOOST_AUTO_TEST_CASE(test_vector)
{
     const char *argv[] = {"",
        "--vector-item=7",
        "--vector-item=7",
        "--vector-item=5",
        "--vector-item=3",
        "--vector-item=3",
        "--vector-item=1"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK((cfg.get<def_vector_1>() == std::vector<int>{1, 2, 3}));
    BOOST_CHECK((cfg.get<def_vector_2>() == std::vector<int>{4, 5, 6}));
    BOOST_CHECK((cfg.get<def_vector_3>() == std::vector<int>{7, 8, 9}));
    BOOST_CHECK((cfg.get<vector>() == std::vector<int>{7, 7, 5, 3, 3, 1}));
}

BOOST_AUTO_TEST_CASE(test_set)
{
    const char *argv[] = {"",
        "--set-item=7",
        "--set-item=7",
        "--set-item=5",
        "--set-item=3",
        "--set-item=3",
        "--set-item=1"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK_EQUAL(cfg.get<set>().size(), 4);
    BOOST_CHECK_EQUAL(cfg.get<set>().count(1), 1);
    BOOST_CHECK_EQUAL(cfg.get<set>().count(3), 1);
    BOOST_CHECK_EQUAL(cfg.get<set>().count(5), 1);
    BOOST_CHECK_EQUAL(cfg.get<set>().count(7), 1);
    BOOST_CHECK_EQUAL(cfg.get<default_set>().size(), 2);
    BOOST_CHECK_EQUAL(cfg.get<default_set>().count(1), 1);
    BOOST_CHECK_EQUAL(cfg.get<default_set>().count(3), 1);
}

BOOST_AUTO_TEST_CASE(test_multiset)
{
    const char *argv[] = {"",
        "--multiset-item=7",
        "--multiset-item=7",
        "--multiset-item=5",
        "--multiset-item=3",
        "--multiset-item=3",
        "--multiset-item=1"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK_EQUAL(cfg.get<multiset>().size(), 6);
    BOOST_CHECK_EQUAL(cfg.get<multiset>().count(1), 1);
    BOOST_CHECK_EQUAL(cfg.get<multiset>().count(3), 2);
    BOOST_CHECK_EQUAL(cfg.get<multiset>().count(5), 1);
    BOOST_CHECK_EQUAL(cfg.get<multiset>().count(7), 2);
}

BOOST_AUTO_TEST_CASE(test_unordered_set)
{
    const char *argv[] = {"",
        "--unordered-set-item=7",
        "--unordered-set-item=7",
        "--unordered-set-item=5",
        "--unordered-set-item=3",
        "--unordered-set-item=3",
        "--unordered-set-item=1"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK_EQUAL(cfg.get<unordered_set>().size(), 4);
    BOOST_CHECK_EQUAL(cfg.get<unordered_set>().count(1), 1);
    BOOST_CHECK_EQUAL(cfg.get<unordered_set>().count(3), 1);
    BOOST_CHECK_EQUAL(cfg.get<unordered_set>().count(5), 1);
    BOOST_CHECK_EQUAL(cfg.get<unordered_set>().count(7), 1);
}

BOOST_AUTO_TEST_CASE(test_unordered_multiset)
{
    const char *argv[] = {"",
        "--unordered-multiset-item=7",
        "--unordered-multiset-item=7",
        "--unordered-multiset-item=5",
        "--unordered-multiset-item=3",
        "--unordered-multiset-item=3",
        "--unordered-multiset-item=1"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK_EQUAL(cfg.get<unordered_multiset>().size(), 6);
    BOOST_CHECK_EQUAL(cfg.get<unordered_multiset>().count(1), 1);
    BOOST_CHECK_EQUAL(cfg.get<unordered_multiset>().count(3), 2);
    BOOST_CHECK_EQUAL(cfg.get<unordered_multiset>().count(5), 1);
    BOOST_CHECK_EQUAL(cfg.get<unordered_multiset>().count(7), 2);
}

BOOST_AUTO_TEST_SUITE_END() // containers_test_suite

BOOST_AUTO_TEST_SUITE(actions_test_suite)

BOOST_AUTO_TEST_CASE(test_help)
{
    const char *argv[] = {"",
        "--help"
    };
    actions::res.clear();
    config::instance().parse_cmd_line(2, argv);
    BOOST_CHECK(!actions::res.empty()); // result depends on boost
}

BOOST_AUTO_TEST_CASE(test_version)
{
    const char *argv[] = {"",
        "--version"
    };
    actions::res.clear();
    config::instance().parse_cmd_line(2, argv);
    BOOST_CHECK_EQUAL(actions::res, "version test");
}

BOOST_AUTO_TEST_CASE(test_show_config)
{
    const char *argv[] = {"",
        "--show-config",
        "--power2=4",
        "--power2=8"
    };
    actions::res.clear();
    config::instance().parse_cmd_line(4, argv);
    BOOST_CHECK_EQUAL(actions::res, "# config begin\noptions:\n"
        " text: default text\n"
        " number: 80\n"
        " flag: 0\n"
        " cmd_only_int: 100\n"
        " cfg_only_int: 500\n"
        " power2:\n"
        "  - 4\n"
        "  - 8\n"
        "# config end\n");
}

BOOST_AUTO_TEST_SUITE_END() // action_test_suite

BOOST_AUTO_TEST_CASE(test_defaults)
{
    const char *argv[] = {""};
    auto& cfg = config::instance();
    cfg.parse_cmd_line(1, argv);
    BOOST_CHECK_EQUAL(cfg.get<option::text>(), "default text");
    BOOST_CHECK_EQUAL(cfg.get<option::number>(), 80);
    BOOST_CHECK_EQUAL(cfg.get<option::flag>(), false);
    BOOST_CHECK_EQUAL(cfg.get<option::cmd_only_int>(), 100);
    BOOST_CHECK_EQUAL(cfg.get<option::cfg_only_int>(), 500);
    BOOST_CHECK(cfg.get<option::power2>().empty());
}

BOOST_AUTO_TEST_CASE(test_cmd_line)
{
    const char *argv[] = {"",
        "--text=hello",
        "--number=143",
        "-f1", // use short name
        "--power2=8",
        "--cmd-only-int=42",
        "--power2=32"
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(7, argv);
    BOOST_CHECK_EQUAL(cfg.get<option::text>(), "hello");
    BOOST_CHECK_EQUAL(cfg.get<option::number>(), 143);
    BOOST_CHECK_EQUAL(cfg.get<option::flag>(), true);
    BOOST_CHECK_EQUAL(cfg.get<option::cmd_only_int>(), 42);
    BOOST_CHECK_EQUAL(cfg.get<option::cfg_only_int>(), 500);
    BOOST_CHECK((cfg.get<option::power2>() == std::vector<unsigned>{8, 32}));
}

struct cfg_file_fixture: basic_file_fixture<cfg_file_fixture>
{
    void write(std::ostream& file)
    {
        file << "cfg_only_int=12345\n"
             << "[common]\n"
             << "text=text from file\n"
             << "number=8080\n"
             << "flag=1\n"
             << "[power2]\n"
             << "item=64\n"
             << "item=128\n"
             << "item=256\n";
    }
};

BOOST_FIXTURE_TEST_CASE(test_cfg_file, cfg_file_fixture)
{
    auto& cfg = config::instance();
    cfg.parse_file("test.ini");
    BOOST_CHECK_EQUAL(cfg.get<option::text>(), "text from file");
    BOOST_CHECK_EQUAL(cfg.get<option::number>(), 8080);
    BOOST_CHECK_EQUAL(cfg.get<option::flag>(), true);
    BOOST_CHECK_EQUAL(cfg.get<option::cmd_only_int>(), 100);
    BOOST_CHECK_EQUAL(cfg.get<option::cfg_only_int>(), 12345);
    BOOST_CHECK((cfg.get<option::power2>() == std::vector<unsigned>{64, 128, 256}));
}

BOOST_FIXTURE_TEST_CASE(test_cmd_line_cfg_file, cfg_file_fixture)
{
    const char *argv[] = {"",
        "--config=test.ini",
        "--number=1", // override config
        "--flag=off", // override config
    };
    auto& cfg = config::instance();
    cfg.parse_cmd_line(4, argv);
    BOOST_CHECK_EQUAL(cfg.get<option::text>(), "text from file");
    BOOST_CHECK_EQUAL(cfg.get<option::number>(), 1);
    BOOST_CHECK_EQUAL(cfg.get<option::flag>(), false);
    BOOST_CHECK_EQUAL(cfg.get<option::cmd_only_int>(), 100);
    BOOST_CHECK_EQUAL(cfg.get<option::cfg_only_int>(), 12345);
    BOOST_CHECK((cfg.get<option::power2>() == std::vector<unsigned>{64, 128, 256}));
}

BOOST_AUTO_TEST_CASE(test_cfg_only_option_in_cmd_line)
{
    const char *argv[] = {"",
        "--cfg_only_int=10" // not allowed in cmd line
    };
    BOOST_CHECK_THROW(config::instance().parse_cmd_line(2, argv), raconfig::config_error);
}

struct bad_cfg_file_fixture: basic_file_fixture<bad_cfg_file_fixture>
{
    void write(std::ostream& file)
    {
        file << "cmd-only-int=12345\n";
    }
};

BOOST_FIXTURE_TEST_CASE(test_cmd_only_option_in_cfg_file, bad_cfg_file_fixture)
{
    const char *argv[] = {"",
        "--config=test.ini"
    };
    BOOST_CHECK_THROW(config::instance().parse_cmd_line(2, argv), raconfig::config_error);
}

BOOST_AUTO_TEST_CASE(test_no_cfg_file)
{
    const char *argv[] = {"",
        "--config=abcdefghijklmnopqrstuvwxyz"
    };
    BOOST_CHECK_THROW(config::instance().parse_cmd_line(2, argv), raconfig::config_error);
}

BOOST_AUTO_TEST_CASE(test_option_check_failed)
{
    const char *argv[] = {"",
        "--power2=16",
        "--power2=17" // not the power of 2
    };
    BOOST_CHECK_THROW(config::instance().parse_cmd_line(3, argv), raconfig::config_error);
}

BOOST_AUTO_TEST_CASE(test_callbacks)
{
    static unsigned short number = 0;
    static bool flag = false;
    using config = raconfig::config<raconfig::default_actions, option::number, option::flag>;
    config::callback const cb1{[](){
        number = config::instance().get<option::number>();
    }};
    config::callback const cb2{[](){
        flag = config::instance().get<option::flag>();
    }};
    const char *argv[] = {"",
        "--number=42",
        "--flag=on"
    };
    config::instance().parse_cmd_line(3, argv);
    BOOST_CHECK_EQUAL(number, 42);
    BOOST_CHECK(flag);
}
