#include "Solver.h"

SAT::SAT(std::string& name_file) {
    std::string line;
    std::ifstream in(name_file);
    if (!in.is_open()) {
        std::cerr << "file cannot be open\n";
        return;
    }
    while (std::getline(in, line)) {
        if (line[0] == 'p') {
            std::istringstream iss(line);
            std::string tmp;
            iss >> tmp >> tmp >> size_var >> size_clauses;
            break;
        }
    }
    var.resize(size_var+1);
    clauses.resize(size_clauses+1);
    int x, i = 1;
    while (in >> x) {
        if (x == 0) {
            if (clauses[i].literals.size() == 1){
                //сохраняю переменные которые входят в клаузу единичной длины
                single_literal.push_back(clauses[i].literals[0]);
            }
            i++;
            continue;
        }
        clauses[i].literals.push_back(x);
        //сохраняю номера клауз соотвествующим переменным и добавляю знак, если переменная с отрицаинем
        var[abs(x)].push_back((i)*((x>0)?1:-1));
    }

    //подсчитываю как часто встречается каждая переменная
    for (int k = 0; k < var.size(); ++k){
        lit_claus temp(var[k].size(), k);
        lit_sorted.push_back(temp);
    }
    //сортирую массив, чтобы в начале стояли наиболее часто встречаемые переменные
    std::sort(lit_sorted.begin(), lit_sorted.end(), func_for_sort);

    //обнуляю все размеры, чтобы в будущем использовать их как рейтинг переменных
    for (int k = 0; k < var.size(); ++k){
        lit_sorted[k].size = 0;
    }
    in.close();
    return;
}

//увеличивает рейтинг переменной при конфликте
void SAT::add_conflict(int _var){
static int count = 0;
int i = 0;
    count++;
    for (int k = 0; k < lit_sorted.size(); ++k) {
        if (count == 256) {
            lit_sorted[k].size /= 2;
        }
        if (lit_sorted[k].var == _var) {
            lit_sorted[k].size++;
            if (k>1) i = k-1;
            while (lit_sorted[k].size > lit_sorted[i].size) i--;
            lit_sorted.emplace(lit_sorted.begin() +i+1, lit_sorted[k]);
            lit_sorted.erase(lit_sorted.begin()+k+1);
            if (count != 256)
                break;
        }
    }
    count %= 256;
    return;
}

//удаляет чистые литералы
void SAT::delete_blank_lit(void) {
    for (int j = 1; j < var.size(); ++j) {
        int sign = (var[j][0]>0)?1:-1;
        bool flag = true;
        for (int k = 0; k < var[j].size(); ++k) {
            if (sign*((var[j][k]>0)?1:-1) < 0) {
                flag = false;
                break;
            }
        }
        if (flag) single_literal.push_back(sign*j);  //добавляем в список на удаление
    }
    return;
}

//удаляет все перменные из списка single_literal
void SAT::delete_single_literal(std::vector<int>& assignment) {
    for (int lit : single_literal) {
        int lit_abs = abs(lit);
        //прохожусь по номерам клауз соответсвующей переменной lit_abs
        for (int k = 0; k < var[lit_abs].size(); ++k) {
            int claus = abs(var[lit_abs][k]);
            
            if(lit*var[lit_abs][k] > 0)  {
                for(int lit : clauses[claus].literals){
                    std::erase(var[abs(lit)], claus);
                }
                clauses[claus].literals.clear();
                //нужно так же добавить удаление claus из всего массива var

            } else                  //если в клаузе литерал противолложного знака
                std::erase(clauses[claus].literals, (-1)*lit);
        }
        var[lit_abs].clear();
        assignment[lit_abs] = (lit > 0)? 1 : -1;
    }
    single_literal.clear();
    return;
}

//проверка выполнимость кнф на наборе assignment и выявление конфликтов
bool SAT::unitPropaget(std::vector<int>& assignment, std::stack<int>& decision) {
    bool changed;
    do {
        changed = false;
        for (int j = 1; j < clauses.size(); ++j) {
            if (clauses[j].literals.size() == 0)
                continue;
            if  (is_satisfiable(clauses[j], assignment))
                continue;
            
            int unassigned = 0, unit_lit = 0;
            for (int lit : clauses[j].literals) {
                if (assignment[abs(lit)] == 0) {
                    unassigned++;
                    unit_lit = lit;
                }
            }

            if (unassigned == 0)
                return false; // конфликт
            if (unassigned == 1) {
                //единственный не назначенный литерал в клаузе
                int v_abs = abs(unit_lit);
                assignment[v_abs] = (unit_lit > 0)? 1 : -1;
                decision.push(unit_lit);
                changed = true;
            }
        }
    }while (changed);
    return true;

}

bool SAT::solver(void) {
    std::vector<int> assignment(size_var+1, 0); //вектор значений переменных
    std::stack<int> decision, level; //запоминаем решения и уровень для отката

    //небольшая предобработка
    delete_blank_lit();
    delete_single_literal(assignment);

    while (true) {
        if (!unitPropaget(assignment, decision)) {
            //если возник конфликт

            bool flag_step_back = false;
            while (!level.empty()) {
                int last = level.top();
                while (last != decision.top()) {
                    assignment[abs(decision.top())] = 0;
                    decision.pop();
                }
                int v_abs = abs(last);
                decision.pop();
                level.pop();
                add_conflict(v_abs);   //добавляем рейтинг конфликтной переменной

                if ((last > 0 && assignment[v_abs]== 1) ||
                    (last < 0 && assignment[v_abs]==-1)) {
                    //меняем на противоположное значение
                    assignment[v_abs] = (last > 0)? -1 : 1;
                    decision.push(last);
                    level.push(last);
                    flag_step_back = true; //получилось сделать шаг назад
                    break;
                }
                //если мы уже меняли на противоположное
                assignment[v_abs] = 0;
            }
            
            if (!flag_step_back) return false; //все варианты были исчерпаны
            continue;
        }

        //вручную назначаем переменную
        bool satisfiable = true;
        for (int j = 0; j < size_var; ++j)
            if (assignment[lit_sorted[j].var] == 0) {
                assignment[lit_sorted[j].var] = 1;
                level.push(lit_sorted[j].var);
                decision.push(lit_sorted[j].var);
                satisfiable = false;
                break;
            }
        
        //все переменные определены и конфликтов не возникло
        if (satisfiable) return true; 
    }
}

std::string SAT::get_result(void) {
    if (solver())
        return "SAT";
    else
        return "UNSAT";
}

bool func_for_sort(const lit_claus a, const lit_claus b){
    if (a.size > b.size)
        return true;
    else
        return false;
}

bool is_satisfiable(Clause& clause, std::vector<int> assignment) {
    for (int lit : clause.literals) {
        if ((lit > 0 && assignment[abs(lit)] == 1) ||
            (lit < 0 && assignment[abs(lit)] ==-1)) {
            return true;
        }
    }
    return false;
}
