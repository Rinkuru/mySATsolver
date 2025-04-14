#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <sstream>
#include <queue>

class Clause {
public:
    std::vector<int> literals;
};

struct lit_claus{
    int size;
    int var;
    lit_claus(int _size, int _var): size(_size), var(_var) {}
};

class SAT{
    std::vector<std::vector<int>>var;  //номера в клауз в которой нохидтся i переменная
    std::vector<Clause>clauses;        //хранение кнф
    int size_var, size_clauses;
    std::vector<lit_claus> lit_sorted; //массив для оптимального выбора переменной
    std::vector<int>single_literal;    //для предобработки
    std::vector<std::vector<int>> watch_list_pos; //положительные
    std::vector<std::vector<int>> watch_list_neg; //отрицательные

    bool unitPropaget(std::vector<int>& assignment, std::stack<int>& decision, std::queue<int>& units);
    void raise_rating(int _var);
    void delete_single_literal(std::vector<int>& assignment);
    bool solver(void);
    void re_pars(void);
    bool all_assigned(std::vector<int>& assignment, std::stack<int>& decision, std::queue<int>& units);
public:
    SAT(std::string& name_file);
    std::string get_result(void);
};

bool func_for_sort(const lit_claus a, const lit_claus b);
bool is_satisfiable(Clause& clause, std::vector<int> assignment);
void clear_queue(std::queue<int>& units);

