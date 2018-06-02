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

#ifndef RACONFIG_SET_HPP
#define RACONFIG_SET_HPP

#include <set>
#include "raconfig_range.hpp"

namespace raconfig
{
namespace detail
{

template<class T, class Compare, class Allocator>
struct type_proxy<std::set<T, Compare, Allocator>>
    : type_proxy_range<std::set<T, Compare, Allocator>>
{};

template<class T, class Compare, class Allocator>
class option_value<std::set<T, Compare, Allocator>>
    : public convertible_option_value<
        std::set<T, Compare, Allocator>,
        std::vector<T, Allocator>>
{
public:
    option_value(std::set<T, Compare, Allocator> const& value)
        : convertible_option_value<
            std::set<T, Compare, Allocator>,
            std::vector<T, Allocator>>{value}
    {}

    option_value(std::set<T, Compare, Allocator>&& value)
        : convertible_option_value<
            std::set<T, Compare, Allocator>,
            std::vector<T, Allocator>>{std::move(value)}
    {}
};

template<class T, class Compare, class Allocator>
struct type_proxy<std::multiset<T, Compare, Allocator>>
    : type_proxy_range<std::multiset<T, Compare, Allocator>>
{};

template<class T, class Compare, class Allocator>
class option_value<std::multiset<T, Compare, Allocator>>
    : public convertible_option_value<
        std::multiset<T, Compare, Allocator>,
        std::vector<T, Allocator>>
{
public:
    option_value(std::multiset<T, Compare, Allocator> const& value)
        : convertible_option_value<
            std::multiset<T, Compare, Allocator>,
            std::vector<T, Allocator>>{value}
    {}

    option_value(std::multiset<T, Compare, Allocator>&& value)
        : convertible_option_value<
            std::multiset<T, Compare, Allocator>,
            std::vector<T, Allocator>>{std::move(value)}
    {}
};

} // namespace detail
} // namespace raconfig

#endif