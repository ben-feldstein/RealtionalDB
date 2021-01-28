//Project Identifier: C0F4DFE8B340D81183C208F70F9D2D797908754D
#include "TableEntry.h"
#include <getopt.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using namespace std;

const int KEY_BOOL = 1;
const int KEY_STRING = 2;
const int KEY_INT = 3;
const int KEY_DOUBLE = 4;

class SillyQL {

    bool is_quiet = false;

    struct Table {
        Table& operator=(const Table& rhs) {
            Table temp;
            temp.num_cols = rhs.num_cols;
            temp.num_rows = rhs.num_rows;
            temp.tablename = rhs.tablename;
            swap(num_cols, temp.num_cols);
            swap(num_rows, temp.num_rows);
            swap(tablename, temp.tablename);
            for (int i = 0; i < num_cols; i++) {
                col_types.push_back(rhs.col_types[i]);
                col_names.push_back(rhs.col_names[i]);
            }
            return *this;
        }// op =

        bool bst = false;
        bool hsh = false;
        int num_cols = 0;
        int num_rows = 0;
        int hash_index_col = -1;
        int bst_index_col = -1;
        string tablename = "";
        vector<int> col_types;
        vector<string> col_names;
        vector<vector<TableEntry>> entries;
        unordered_map<TableEntry, vector<int>> hash_index;
        map<TableEntry, vector<int>> bst_index;

        bool where_compare(int row, int comp_col, TableEntry val, int op) {
            switch (op) {
            case 0:
                return entries[row][comp_col] < val;
            case 1:
                return entries[row][comp_col] > val;
            case 2:
                return entries[row][comp_col] == val;
            default:
                cout << "DEFAULT\n";
                return false;
            }
        }// where_compare

        void print_where(int comp_col, TableEntry val, int op, int& num_printed, const int num_cols, unordered_map<int, int>& c_n_index, bool is_quiet) {
            if (op == 2 && hash_index_col == comp_col) {
                for (auto& it : hash_index[val]) {
                    num_printed++;
                    if (!is_quiet) {
                        for (int k = 0; k < num_cols; k++) {
                            cout << entries[it][c_n_index[k]] << " ";
                        }// for k
                        cout << "\n";
                    }
                }
            } //if hash index exists for col
            else if (op == 0 && bst_index_col == comp_col) {
                auto lb = bst_index.lower_bound(val);
                auto start = bst_index.begin();
                for (; start != lb; ++start) {
                    for (auto& ind : start->second) {
                        num_printed++;
                        if (!is_quiet) {
                            for (int k = 0; k < num_cols; k++) {
                                cout << entries[ind][c_n_index[k]] << " ";
                            }// for k
                            cout << "\n";
                        }
                    }
                }
            }// if bst index exists for col and <
            else if (op == 1 && bst_index_col == comp_col) {
                auto start = bst_index.upper_bound(val);
                for (; start != bst_index.end(); ++start) {
                    for (auto& ind : start->second) {
                        num_printed++;
                        if (!is_quiet) {
                            for (int k = 0; k < num_cols; k++) {
                                cout << entries[ind][c_n_index[k]] << " ";
                            }// for k
                            cout << "\n";
                        }
                    }
                }
            }// if bst index exists for col and >
            else {
                for (int n = 0; n < num_rows; n++) {
                    if (where_compare(n, comp_col, val, op)) {
                        num_printed++;
                        if (!is_quiet) {
                            for (int k = 0; k < num_cols; k++) {
                                cout << entries[n][c_n_index[k]] << " ";
                            }// for k
                            cout << "\n";
                        }
                    }// if where_compare
                }// for n
            }// else
        }// print_where

        struct delete_comp_less {
        private:    
            TableEntry e;
            int comp_col;
        public:
            delete_comp_less(int comp_col_in, const TableEntry& val)
                : e(val), comp_col(comp_col_in) {}
                
            bool operator () (vector<TableEntry> vec) {
                return vec[comp_col] < e;
            }       
        };// delete_comp_less

        struct delete_comp_greater {
        private:
            TableEntry e;
            int comp_col;
        public:
            delete_comp_greater(int comp_col_in, const TableEntry& val)
                : e(val), comp_col(comp_col_in) {}

            bool operator () (vector<TableEntry> vec) {
                return vec[comp_col] > e;
            }
        };// delete_comp_greater
        
        struct delete_comp_equal {
        private:
            TableEntry e;
            int comp_col;
        public:
            delete_comp_equal(int comp_col_in, const TableEntry& val)
                : e(val), comp_col(comp_col_in) {}

            bool operator () (vector<TableEntry> vec) {
                return vec[comp_col] == e;
            }
        };// delete_comp_greater

        void delete_where(int& num_deleted, const TableEntry& val, int comp_col, int op) {
            auto iter = entries.begin();
            switch (op) {
            case 0:
                iter = remove_if(entries.begin(), entries.end(), delete_comp_less(comp_col, val));
                break;
            case 1:
                iter = remove_if(entries.begin(), entries.end(), delete_comp_greater(comp_col, val));
                break;
            case 2:
                iter = remove_if(entries.begin(), entries.end(), delete_comp_equal(comp_col, val));
                break;
            }
            entries.erase(iter, entries.end());
            num_deleted = num_rows - static_cast<int>(entries.size());
            num_rows = static_cast<int>(entries.size());
        }// delete_where
    };// Table

    unordered_map<string, Table> database;

    bool colname_error(string col, string tname) {
        if (find(database[tname].col_names.begin(), database[tname].col_names.end(), col) == database[tname].col_names.end()) {
            cout <<"Error: " << col << " does not name a column in " << tname << "\n";
            return true;
        }
        else {
            return false;
        }
    }

public:

    void get_options(int argc, char *argv[]);

    void read_input();


};

int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);
    cin >> std::boolalpha;
    cout << std::boolalpha;

    SillyQL sil;

    sil.get_options(argc, argv);

    sil.read_input();

    return 0;
}// main

void SillyQL::get_options(int argc, char *argv[]) {
    int option_index = 0, option = 0;
    opterr = false;

    struct option longOpts[] = { { "quiet", no_argument, nullptr, 'q' },
                                { "help", no_argument, nullptr, 'h' },
                                { nullptr, 0, nullptr, '\0' } };

    while ((option = getopt_long(argc, argv, "qh", longOpts, &option_index)) != -1) {
        switch (option) {
        case 'q':
            is_quiet = true;
            break;
        case 'h':
            std::cout << "This program reads user commands,\n"
                << "then executes them,\n"
                << "the program creates and modifies data bases\n"
                << "Usage: \'./silly\n\t[--help | -h]\n"
                << "\t[--quiet | -q]\n"
                << "\t< <command file>\'" << std::endl;
            exit(0);
        }
    }
}

void SillyQL::read_input() {
    string cmd;
    string junk;

    do {
        cout << "% ";
        cin >> cmd;

        if (cmd[0] == '#') {
            getline(cin, junk);
            continue;
        }// if #

        if (cmd == "CREATE") {
            string tname;
            cin >> tname;
            auto it = database.find(tname);
            if (it != database.end()) {
                cout << "Error: " << "Cannot create already existing table " << tname << "\n";
                getline(cin, junk);
                continue;
            }
            Table t;
            t.tablename = tname;
            int num_cols;
            cin >> num_cols;
            t.num_cols = num_cols;
            for (int i = 0; i < num_cols; i++) {
                string col_type;
                int col_type_key;
                cin >> col_type;
                if ("bool" == col_type) {
                    col_type_key = KEY_BOOL;
                } 
                else if ("string" == col_type) {
                    col_type_key = KEY_STRING;
                }
                else if ("int" == col_type) {
                    col_type_key = KEY_INT;
                }
                else {
                    col_type_key = KEY_DOUBLE;
                }
                t.col_types.emplace_back(col_type_key);
            }
            for (int i = 0; i < num_cols; i++) {
                string col_name;
                cin >> col_name;
                t.col_names.emplace_back(col_name);
            }
            database[tname] = t;
            cout << "New table " << tname << " with column(s) ";
            for (auto& i : t.col_names) {
                cout << i << " ";
            }
            cout << "created\n";
            continue;
        }// if CREATE

        if (cmd == "REMOVE") {
            string tname;
            cin >> tname;
            auto it = database.find(tname);
            if (it == database.end()) {
                cout << "Error: " << tname << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            database.erase(tname);
            cout << "Table " << tname << " deleted\n";
            continue;
        }// if REMOVE

        if (cmd == "INSERT") { //TODO: use GENERATE'd index here
            cin >> junk;
            string tname;
            cin >> tname;
            bool no_name = false;
            auto it = database.find(tname);
            if (it == database.end()) {
                cout << "Error: " << tname << " does not name a table in the database\n";
                //getline(cin, junk);
                no_name = true;
            }
            int num_rows;
            cin >> num_rows;
            string rows;
            cin >> rows;
            if (no_name) {
                getline(cin, junk);
                for (int i = 0; i < num_rows; i++) {
                    getline(cin, junk);
                }
                continue;
            }
            
            int startN = database[tname].num_rows;
            database[tname].num_rows += num_rows;
            int endN = database[tname].num_rows - 1;
            
            database[tname].entries.resize(database[tname].num_rows);
            for (int i = startN; i <= endN; i++) {
                bool hash_it = false;
                bool bst_it = false;
                for (int j = 0; j < database[tname].num_cols; j++) {
                    if (database[tname].hash_index_col == j) {
                        hash_it = true;
                    }
                    if (database[tname].bst_index_col == j) {
                        bst_it = true;
                    }
                    int key = database[tname].col_types[j];
                    switch (key) {
                    case KEY_BOOL:
                        bool value_b;
                        cin >> value_b;
                        database[tname].entries[i].emplace_back(TableEntry(value_b));
                        if (hash_it) {
                            database[tname].hash_index[TableEntry(value_b)].push_back(i);
                            hash_it = false;
                        }
                        if (bst_it) {
                            database[tname].bst_index[TableEntry(value_b)].push_back(i);
                            bst_it = false;
                        }
                        break;
                    case KEY_STRING:
                    {
                        string value_s;
                        cin >> value_s;
                        database[tname].entries[i].emplace_back(TableEntry(value_s));
                        if (hash_it) {
                            database[tname].hash_index[TableEntry(value_s)].push_back(i);
                            hash_it = false;
                        }
                        if (bst_it) {
                            database[tname].bst_index[TableEntry(value_s)].push_back(i);
                            bst_it = false;
                        }
                        break;
                    }
                    case KEY_INT:
                        int value_i;
                        cin >> value_i;
                        database[tname].entries[i].emplace_back(TableEntry(value_i));
                        if (hash_it) {
                            database[tname].hash_index[TableEntry(value_i)].push_back(i);
                            hash_it = false;
                        }
                        if (bst_it) {
                            database[tname].bst_index[TableEntry(value_i)].push_back(i);
                            bst_it = false;
                        }
                        break;
                    case KEY_DOUBLE:
                        double value_d;
                        cin >> value_d;
                        database[tname].entries[i].emplace_back(TableEntry(value_d));
                        if (hash_it) {
                            database[tname].hash_index[TableEntry(value_d)].push_back(i);
                            hash_it = false;
                        }
                        if (bst_it) {
                            database[tname].bst_index[TableEntry(value_d)].push_back(i);
                            bst_it = false;
                        }
                        break;
                    default:
                        string value;
                        cin >> value;
                        database[tname].entries[i].emplace_back(TableEntry(value));
                        if (hash_it) {
                            database[tname].hash_index[TableEntry(value)].push_back(i);
                            hash_it = false;
                        }
                        if (bst_it) {
                            database[tname].bst_index[TableEntry(value)].push_back(i);
                            bst_it = false;
                        }
                    }
                }
            }
            cout << "Added " << num_rows << " rows to " << tname << " from position " << startN << " to " << endN << "\n";
            continue;
        }// if INSERT

        if (cmd == "PRINT") { //TODO: use GENERATE'd index here
            bool leave = false;
            cin >> junk;
            string tname;
            cin >> tname;
            auto it = database.find(tname);
            if (it == database.end()) {
                cout << "Error: " << tname << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            int num_cols_printed;
            cin >> num_cols_printed;
            unordered_map<int, int> col_name_index;
            for (int i = 0; i < num_cols_printed; i++) {
                string col_name;
                cin >> col_name;
                if (find(database[tname].col_names.begin(), database[tname].col_names.end(), col_name) == database[tname].col_names.end()) {
                    cout << "Error: " << col_name << " does not name a column in " << tname << "\n";
                    getline(cin, junk);
                    leave = true;
                    break;
                }
                for (int j = 0; j < static_cast<int>(database[tname].col_names.size()); j++) {
                    if (col_name == database[tname].col_names[j]) {
                        col_name_index[i] = j;
                        break;
                    }
                }//for j
            }// for i
            if (leave) continue;
            string all_or_where;
            cin >> all_or_where;
           
            if (all_or_where == "ALL") {
                if (!is_quiet) {
                    for (int k = 0; k < num_cols_printed; k++) {
                        cout << database[tname].col_names[col_name_index[k]] << " ";
                    }
                    cout << "\n";
                    for (int n = 0; n < database[tname].num_rows; n++) {
                        for (int k = 0; k < num_cols_printed; k++) {
                            cout << database[tname].entries[n][col_name_index[k]] << " ";
                        }
                        cout << "\n";
                    }
                }
                cout << "Printed " << database[tname].num_rows << " matching rows from " << tname << "\n";
            }
            else { //PRINT WHERE
                int num_rows_printed = 0;
                string colname;
                string op_in;
                cin >> colname >> op_in;
                if (find(database[tname].col_names.begin(), database[tname].col_names.end(), colname) == database[tname].col_names.end()) {
                    cout << "Error: " << colname << " does not name a column in " << tname << "\n";
                    getline(cin, junk);
                    continue;
                }
                if (!is_quiet) {
                    for (int k = 0; k < num_cols_printed; k++) {
                        cout << database[tname].col_names[col_name_index[k]] << " ";
                    }
                    cout << "\n";
                }
                int op;
                if (op_in == "<") op = 0;
                else if (op_in == ">") op = 1;
                else op = 2;
                int coltype = 2;
                int comparison_col_index = 0;
                for (int i = 0; i < database[tname].num_cols; i++) {
                    if (colname == database[tname].col_names[i]) {
                        coltype = database[tname].col_types[i];
                        comparison_col_index = i;
                        break;
                    }
                }
                switch (coltype) {
                case KEY_BOOL:
                    bool val_b;
                    cin >> val_b;
                    database[tname].print_where(comparison_col_index, TableEntry(val_b), op, num_rows_printed, num_cols_printed, col_name_index, is_quiet);
                    break;
                case KEY_DOUBLE:
                    double val_d;
                    cin >> val_d;
                    database[tname].print_where(comparison_col_index, TableEntry(val_d), op, num_rows_printed, num_cols_printed, col_name_index, is_quiet);
                    break;
                case KEY_INT:
                    int val_i;
                    cin >> val_i;
                    database[tname].print_where(comparison_col_index, TableEntry(val_i), op, num_rows_printed, num_cols_printed, col_name_index, is_quiet);
                    break;
                case KEY_STRING:
                    string val_s;
                    cin >> val_s;
                    database[tname].print_where(comparison_col_index, TableEntry(val_s), op, num_rows_printed, num_cols_printed, col_name_index, is_quiet);
                    break;
                }
                cout << "Printed " << num_rows_printed << " matching rows from " << tname << "\n";
            }// PRINT...WHERE
            continue;
        }// if PRINT

        if (cmd == "DELETE") { //TODO: use GENERATE'd index here
            cin >> junk;
            string tname;
            string colname;
            string op_in;
            cin >> tname >> junk >> colname >> op_in;
            auto it = database.find(tname);
            if (it == database.end()) {
                cout << "Error: " << tname << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            if (colname_error(colname, tname)) {
                getline(cin, junk);
                continue;
            }
            int op;
            if (op_in == "<") op = 0;
            else if (op_in == ">") op = 1;
            else op = 2;
            int coltype = 2;
            int comparison_col_index = 0;
            for (int i = 0; i < database[tname].num_cols; i++) {
                if (colname == database[tname].col_names[i]) {
                    coltype = database[tname].col_types[i];
                    comparison_col_index = i;
                }
            }
            int num_deleted = 0;
            switch (coltype) {
            case KEY_BOOL:
                bool val_b;
                cin >> val_b;
                database[tname].delete_where(num_deleted, TableEntry(val_b), comparison_col_index, op);
                break;
            case KEY_DOUBLE:
                double val_d;
                cin >> val_d;
                database[tname].delete_where(num_deleted, TableEntry(val_d), comparison_col_index, op);
                break;
            case KEY_INT:
                int val_i;
                cin >> val_i;
                database[tname].delete_where(num_deleted, TableEntry(val_i), comparison_col_index, op);
                break;
            case KEY_STRING:
                string val_s;
                cin >> val_s;
                database[tname].delete_where(num_deleted, TableEntry(val_s), comparison_col_index, op);
                break;
            }
            cout << "Deleted " << num_deleted << " rows from " << tname << "\n";
            if (database[tname].hsh) {
                database[tname].hash_index.clear();
                int counter = 0;
                for (auto& v : database[tname].entries) {
                    database[tname].hash_index[v[database[tname].hash_index_col]].push_back(counter);
                    counter++;
                }
            }
            if (database[tname].bst) {
                database[tname].bst_index.clear();
                int counter = 0;
                for (auto& v : database[tname].entries) {
                    database[tname].bst_index[v[database[tname].bst_index_col]].push_back(counter);
                    counter++;
                }
            }
            continue;
        }// if DELETE

        if (cmd == "GENERATE") {
            cin >> junk;
            string tname;
            string index_type;
            string colname;
            cin >> tname >> index_type >> junk >> junk >> colname;
            auto it = database.find(tname);
            if (it == database.end()) {
                cout << "Error: "<< tname << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            if (find(database[tname].col_names.begin(), database[tname].col_names.end(), colname) == database[tname].col_names.end()) {
                cout << "Error: " << colname << " does not name a column in " << tname << "\n";
                getline(cin, junk);
                continue;
            }
            if (index_type == "hash") {
                database[tname].hsh = true;
                database[tname].hash_index.clear(); 
                database[tname].bst_index_col = -1;
                database[tname].bst = false;
                for (int i = 0; i < static_cast<int>(database[tname].col_names.size()); i++) {
                    if (colname == database[tname].col_names[i]) {
                        database[tname].hash_index_col = i;
                        break;
                    }
                }
                int counter = 0;
                for (auto& v : database[tname].entries) {
                    database[tname].hash_index[v[database[tname].hash_index_col]].push_back(counter);
                    counter++;
                }
            }
            else { //bst
                database[tname].bst = true;
                database[tname].bst_index.clear();
                database[tname].hash_index_col = -1;
                database[tname].hsh = false;
                for (int i = 0; i < static_cast<int>(database[tname].col_names.size()); i++) {
                    if (colname == database[tname].col_names[i]) {
                        database[tname].bst_index_col = i;
                        break;
                    }
                }
                int counter = 0;
                for (auto& v : database[tname].entries) {
                    database[tname].bst_index[v[database[tname].bst_index_col]].push_back(counter);
                    counter++;
                }
            }// else

            cout << "Created " << index_type << " index for table " << tname << " on column " << colname << "\n";
            continue;
        }// if GENERATE

        if (cmd == "JOIN") { //TODO: use GENERATE'd index here
            string tablename1, tablename2;
            cin >> tablename1 >> junk >> tablename2;
            auto check = database.find(tablename1);
            if (check == database.end()) {
                cout << "Error: " << tablename1 << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            check = database.find(tablename2);
            if (check == database.end()) {
                cout << "Error: " << tablename2 << " does not name a table in the database\n";
                getline(cin, junk);
                continue;
            }
            string colname1, colname2;
            char op;
            cin >> junk >> colname1 >> op >> colname2;
            if (colname_error(colname1, tablename1)) {
                getline(cin, junk);
                continue;
            }
            if (colname_error(colname2, tablename2)) {
                getline(cin, junk);
                continue;
            }
            int colname1_index = 0;
            int colname2_index = 0;
            for (int i = 0; i < database[tablename1].num_cols; i++) {
                if (colname1 == database[tablename1].col_names[i]) {
                    colname1_index = i;
                    break;
                }
            }
            for (int i = 0; i < database[tablename2].num_cols; i++) {
                if (colname2 == database[tablename2].col_names[i]) {
                    colname2_index = i;
                    break;
                }
            }
            cin >> junk;
            cin >> junk;
            int n_cols;
            cin >> n_cols;
            string colname;
            int table_1_or_2;
            vector<pair<int, int>> col_indexes; //vec of pairs of {col index, table number}
            bool is_error = false;
            for (int i = 0; i < n_cols; i++) {
                cin >> colname >> table_1_or_2;
                if (table_1_or_2 == 1) {
                    if (colname_error(colname, tablename1)) {
                        is_error = true;
                        getline(cin, junk);
                        break;
                    }
                    int index = 0;
                    for (auto& it : database[tablename1].col_names) {
                        if (it == colname) {
                            col_indexes.push_back(make_pair(index, 1));
                            break;
                        }
                        index++;
                    }// for
                }// table 1
                else {
                    if (colname_error(colname, tablename2)) {
                        is_error = true;
                        getline(cin, junk);
                        break;
                    }
                    int index = 0;
                    for (auto& it : database[tablename2].col_names) {
                        if (it == colname) {
                            col_indexes.push_back(make_pair(index, 2));
                            break;
                        }
                        index++;
                    }// for
                }// else table 2
            }// for
            if (is_error) {
                continue;
            }
            if (!is_quiet) {
                for (int i = 0; i < n_cols; i++) {
                    if (col_indexes[i].second == 1) {
                        cout << database[tablename1].col_names[col_indexes[i].first] << " ";
                    }
                    else {
                        cout << database[tablename2].col_names[col_indexes[i].first] << " ";
                    }
                }
                cout << "\n";
            }

            int num_rows_printed = 0;

            if (database[tablename2].hsh && database[tablename2].hash_index_col == colname2_index) {
                for (auto& it : database[tablename1].entries) {
                    if (database[tablename2].hash_index.find(it[colname1_index]) != database[tablename2].hash_index.end()) {
                        for (auto& in : database[tablename2].hash_index[it[colname1_index]]) {
                            num_rows_printed++;
                            if (!is_quiet) {
                                for (int j = 0; j < n_cols; j++) {
                                    if (col_indexes[j].second == 1) {
                                        cout << it[col_indexes[j].first] << " ";
                                    }
                                    else {
                                        cout << database[tablename2].entries[in][col_indexes[j].first] << " ";
                                    }
                                }
                                cout << "\n";
                            }
                        }
                    }
                }
                cout << "Printed " << num_rows_printed << " rows from joining " << tablename1 << " to " << tablename2 << "\n";
                continue;
            }// if hash table for table2 exists on correct col

            if (database[tablename2].bst && database[tablename2].bst_index_col == colname2_index) {
                for (auto& it : database[tablename1].entries) {
                    if (database[tablename2].bst_index.find(it[colname1_index]) != database[tablename2].bst_index.end()) {
                        for (auto& in : database[tablename2].bst_index[it[colname1_index]]) {
                            num_rows_printed++;
                            if (!is_quiet) {
                                for (int j = 0; j < n_cols; j++) {
                                    if (col_indexes[j].second == 1) {
                                        cout << it[col_indexes[j].first] << " ";
                                    }
                                    else {
                                        cout << database[tablename2].entries[in][col_indexes[j].first] << " ";
                                    }
                                }
                                cout << "\n";
                            }
                        }
                    }
                }
                cout << "Printed " << num_rows_printed << " rows from joining " << tablename1 << " to " << tablename2 << "\n";
                continue;
            }// if bst table for table2 exists on correct col

            for (auto& it : database[tablename1].entries) {
                for (auto& it2 : database[tablename2].entries) {
                    if (it[colname1_index] == it2[colname2_index]) {
                        num_rows_printed++;
                        if (!is_quiet) {
                            for (int j = 0; j < n_cols; j++) {
                                if (col_indexes[j].second == 1) {
                                    cout << it[col_indexes[j].first] << " ";
                                }
                                else {
                                    cout << it2[col_indexes[j].first] << " ";
                                }
                            }//for each col to be printed
                            cout << "\n";
                        }
                    }//if comp cols ==
                }//for each row of table 2
            }//for each row of table 1
            cout << "Printed " << num_rows_printed << " rows from joining " << tablename1 << " to " << tablename2 << "\n";
            continue;
        }// if JOIN

        if (cmd[0] != 'Q') {
            getline(cin, junk);
            cout << "Error: unrecognized command\n";
            continue;
        }// bad command
       

    } while (cmd != "QUIT");

    cout << "Thanks for being silly!\n";
}