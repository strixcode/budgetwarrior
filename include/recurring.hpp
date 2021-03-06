//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef RECURRING_H
#define RECURRING_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"

namespace budget {

struct recurring_module {
    void load();
    void unload();
    void preload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<recurring_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "recurring";
};

struct recurring {
    std::size_t id;
    std::string guid;
    std::string name;
    std::size_t old_account;
    money amount;
    std::string recurs;
    std::string account;
};

std::ostream& operator<<(std::ostream& stream, const recurring& recurring);
void operator>>(const std::vector<std::string>& parts, recurring& recurring);

void load_recurrings();
void save_recurrings();

void migrate_recurring_1_to_2();

std::vector<recurring>& all_recurrings();

void set_recurrings_changed();
void set_recurrings_next_id(std::size_t next_id);

} //end of namespace budget

#endif
