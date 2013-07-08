//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "expenses.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"

using namespace budget;

static data_handler<expense> expenses;

void validate_account(const std::string& account){
    if(!account_exists(account)){
        throw budget_exception("The account " + account + " does not exist");
    }
}

void budget::handle_expenses(const std::vector<std::string>& args){
    load_expenses();
    load_accounts();

    if(args.size() == 1){
        show_expenses();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            if(args.size() == 2){
                show_expenses();
            } else if(args.size() == 3){
                show_expenses(boost::gregorian::greg_month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                show_expenses(
                    boost::gregorian::greg_month(to_number<unsigned short>(args[2])),
                    boost::gregorian::greg_year(to_number<unsigned short>(args[3])));
            } else {
                throw budget_exception("Too many arguments to expense show");
            }
        } else if(subcommand == "all"){
            show_all_expenses();
        } else if(subcommand == "add"){
            enough_args(args, 5);

            expense expense;
            expense.guid = generate_guid();
            expense.expense_date = boost::gregorian::day_clock::local_day();

            expense.account = args[2];
            validate_account(expense.account);

            expense.amount = parse_money(args[3]);
            not_negative(expense.amount);

            for(std::size_t i = 4; i < args.size(); ++i){
                expense.name += args[i] + " ";
            }

            add_data(expenses, std::move(expense));
        } else if(subcommand == "addd"){
            enough_args(args, 6);

            expense expense;
            expense.guid = generate_guid();
            expense.expense_date = boost::gregorian::from_string(args[2]);

            expense.account = args[3];
            validate_account(expense.account);

            expense.amount = parse_money(args[4]);
            not_negative(expense.amount);

            for(std::size_t i = 5; i < args.size(); ++i){
                expense.name += args[i] + " ";
            }

            add_data(expenses, std::move(expense));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(expenses, id)){
                throw budget_exception("There are no expense with id ");
            }

            remove(expenses, id);

            std::cout << "Expense " << id << " has been deleted" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }

    save_expenses();
}

void budget::load_expenses(){
    load_data(expenses, "expenses.data");
}

void budget::save_expenses(){
    save_data(expenses, "expenses.data");
}

void budget::show_expenses(){
    date today = boost::gregorian::day_clock::local_day();

    show_expenses(today.month(), today.year());
}

void budget::show_expenses(boost::gregorian::greg_month month){
    date today = boost::gregorian::day_clock::local_day();

    show_expenses(month, today.year());
}

void budget::show_expenses(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    money total;
    std::size_t count = 0;

    for(auto& expense : expenses.data){
        if(expense.expense_date.year() == year && expense.expense_date.month() == month){
            contents.push_back({to_string(expense.id), to_string(expense.expense_date), expense.account, expense.name, to_string(expense.amount)});

            total += expense.amount;
            ++count;
        }
    }

    if(count == 0){
        std::cout << "No expenses for " << month << "-" << year << std::endl;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total)});

        display_table(columns, contents);
    }
}

void budget::show_all_expenses(){
    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& expense : expenses.data){
        contents.push_back({to_string(expense.id), to_string(expense.expense_date), expense.account, expense.name, to_string(expense.amount)});
    }

    display_table(columns, contents);
}

std::ostream& budget::operator<<(std::ostream& stream, const expense& expense){
    return stream << expense.id  << ':' << expense.guid << ':' << expense.account << ':' << expense.name << ':' << expense.amount << ':' << to_string(expense.expense_date);
}

void budget::operator>>(const std::vector<std::string>& parts, expense& expense){
    expense.id = to_number<std::size_t>(parts[0]);
    expense.guid = parts[1];
    expense.account = parts[2];
    expense.name = parts[3];
    expense.amount = parse_money(parts[4]);
    expense.expense_date = boost::gregorian::from_string(parts[5]);
}

std::vector<expense>& budget::all_expenses(){
    return expenses.data;
}