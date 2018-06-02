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

#ifndef RACONFIG_UNORDERED_SET_HPP
#define RACONFIG_UNORDERED_SET_HPP

#include <unordered_set>
#include "raconfig_range.hpp"

namespace raconfig
{
namespace detail
{

template<class T, class Hash, class Equal, class Allocator>
struct type_proxy<std::unordered_set<T, Hash, Equal, Allocator>>
    : type_proxy_range<std::unordered_set<T, Hash, Equal, Allocator>>
{};

template<class T, class Hash, class Equal, class Allocator>
class option_value<std::unordered_set<T, Hash, Equal, Allocator>>
    : public convertible_option_value<
        std::unordered_set<T, Hash, Equal, Allocator>,
        std::vector<T, Allocator>>
{
public:
    option_value(std::unordered_set<T, Hash, Equal, Allocator> const& value)
        : convertible_option_value<
            std::unordered_set<T, Hash, Equal, Allocator>,
            std::vector<T, Allocator>>{value}
    {}

    option_value(std::unordered_set<T, Hash, Equal, Allocator>&& value)
        : convertible_option_value<
            std::unordered_set<T, Hash, Equal, Allocator>,
            std::vector<T, Allocator>>{std::move(value)}
    {}
};

template<class T, class Hash, class Equal, class Allocator>
struct type_proxy<std::unordered_multiset<T, Hash, Equal, Allocator>>
    : type_proxy_range<std::unordered_multiset<T, Hash, Equal, Allocator>>
{};

template<class T, class Hash, class Equal, class Allocator>
class option_value<std::unordered_multiset<T, Hash, Equal, Allocator>>
    : public convertible_option_value<
        std::unordered_multiset<T, Hash, Equal, Allocator>,
        std::vector<T, Allocator>>
{
public:
    option_value(std::unordered_multiset<T, Hash, Equal, Allocator> const& value)
        : convertible_option_value<
            std::unordered_multiset<T, Hash, Equal, Allocator>,
            std::vector<T, Allocator>>{value}
    {}

    option_value(std::unordered_multiset<T, Hash, Equal, Allocator>&& value)
        : convertible_option_value<
            std::unordered_multiset<T, Hash, Equal, Allocator>,
            std::vector<T, Allocator>>{std::move(value)}
    {}
};

} // namespace detail
} // namespace raconfig

#endif