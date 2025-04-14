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
void SAT::raise_rating(int _var){
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

//удаляет все перменные из списка single_literal
void SAT::delete_single_literal(std::vector<int>& assignment) {
    if (single_literal.empty()) return;
    int i = -1;
    do {
        i++;
        int lit = single_literal[i];
        int lit_abs = abs(lit);

        //прохожусь по номерам клауз соответсвующей переменной lit_abs
        for (int k = 0; k < var[lit_abs].size(); ++k) {
            int claus = abs(var[lit_abs][k]);
            if(lit*var[lit_abs][k] > 0)  {
                clauses[claus].literals.clear();
            } else {                //если в клаузе литерал противолложного знака
                std::erase(clauses[claus].literals, -lit);
                if (clauses[claus].literals.size() == 1) {
                    single_literal.push_back(clauses[claus].literals[0]);
                }
            }
        }
        var[lit_abs].clear();
        assignment[lit_abs] = (lit > 0)? 1 : -1;
    } while (single_literal.size() != i+1);
    single_literal.clear();
    return;
}

//проверка выполнимость кнф на наборе assignment и выявление конфликтов
bool SAT::unitPropaget(std::vector<int>& assignment, std::stack<int>& decision, std::queue<int>& units) {
    do {
        int lit = units.front();
        units.pop();
        int var = abs(lit);
        std::vector<int>& watch_list = (assignment[var] == 1)? watch_list_neg[var] : watch_list_pos[var];

        for (int i = 0; i < watch_list.size(); ++i) {
            int w = watch_list[i];
            Clause& clause = clauses[w];
            
            int iter_false_lit = (clause.literals[0] == -lit) ? 0 : 1;//на какой позиции стоит ложный смотрящий
            int watch1 = clause.literals[iter_false_lit];
            int watch2 = clause.literals[(iter_false_lit==0)?1:0];

            bool found_new_watch = false;
            for (int j = 2; j < clause.literals.size(); ++j) {
                int alt = clause.literals[j];
                int alt_abs = abs(alt);

                if (assignment[alt_abs] == 0 ||
                    (alt > 0 && assignment[alt_abs] == 1) ||
                    (alt < 0 && assignment[alt_abs] == -1)) {

                    //заменяю на новый смотрящий и перемещаю его в начало клаузы
                    std::erase(watch_list, w);
                    --i;
                    if (alt > 0) watch_list_pos[alt].push_back(w);
                    else watch_list_neg[-alt].push_back(w);

                    clause.literals.erase(clause.literals.begin() + j);//выставляю на первые два места
                    clause.literals.erase(clause.literals.begin() + iter_false_lit);
                    clause.literals.emplace(clause.literals.begin() + iter_false_lit, alt);
                    clause.literals.emplace(clause.literals.begin() + j, watch1);                
                    found_new_watch = true;
                    break;
                }
            }

            if (found_new_watch) continue;
            if ((watch2 > 0 && assignment[abs(watch2)] == 1) ||
                (watch2 < 0 && assignment[abs(watch2)] ==-1)) continue;

            if(assignment[abs(watch2)] == 0) {
                assignment[abs(watch2)] = (watch2 > 0)? 1 : -1;
                decision.push(watch2);
                units.push(watch2);
                
            } else {
                if (is_satisfiable(clause, assignment)) continue;
                for (int lit : clause.literals) {
                    raise_rating(lit);
                }
                return false; //конфликт
            }   
        }
    } while (!units.empty());
    return true;
}

bool SAT::all_assigned(std::vector<int>& assignment, std::stack<int>& decision, std::queue<int>& units){
    bool satisfiable = true;
    //вручную назначаем переменную
    for (int j = 0; j < size_var; ++j)
        if (assignment[lit_sorted[j].var] == 0) {
            assignment[lit_sorted[j].var] = 1;
            decision.push(lit_sorted[j].var);
            satisfiable = false;
            units.push(lit_sorted[j].var);
            break;
        }
    return satisfiable;
}

bool SAT::solver(void) {
    std::vector<int> assignment(size_var+1, 0); //вектор значений переменных
    std::stack<int> decision; //запоминаем решения для отката
    std::queue<int> units;

    //небольшая предобработка
    delete_single_literal(assignment);
    re_pars();

    while (true) {
        //все переменные определены и конфликтов не возникло и ручное присвоение
        if (all_assigned(assignment, decision, units)) return true;

        if (!unitPropaget(assignment, decision, units)) {
            //если возник конфликт
            bool flag_step_back = false;
            while (!decision.empty()) {
                int last = decision.top();
                int v_abs = abs(last);

                if ((last > 0 && assignment[v_abs]== 1) ||
                    (last < 0 && assignment[v_abs]==-1)) {
                    //меняем на противоположное значение
                    assignment[v_abs] = (last > 0)? -1 : 1;
                    flag_step_back = true; //получилось сделать шаг назад
                    clear_queue(units);
                    units.push(last);
                    break;
                }
                //если мы уже меняли на противоположное
                assignment[v_abs] = 0;
                decision.pop();
            }
            
            if (!flag_step_back)return false; //все варианты были исчерпаны
            continue;
        }
    
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

void SAT::re_pars(void) {
    //var.clear();
    watch_list_pos.resize(size_var+1);
    watch_list_neg.resize(size_var+1);
    for (int i = 1; i < size_clauses; ++i) {
        if (clauses[i].literals.size() > 1){
            //назначение первых двух переменных как просматриваемых
            int lit = clauses[i].literals[0];
            if (lit > 0) watch_list_pos[lit].push_back(i);
            else watch_list_neg[-lit].push_back(i);
            
            lit = clauses[i].literals[1];
            if (lit > 0) watch_list_pos[lit].push_back(i);
            else watch_list_neg[-lit].push_back(i);
        }
    }
    return;
}

void clear_queue(std::queue<int>& units) {
    while (!units.empty()) units.pop();
    return;
}
