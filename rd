#include <bits/stdc++.h>
using namespace std;

/*
Grammar:
E  → T E'
E' → + T E' | ε
T  → F T'
T' → * F T' | ε
F  → ( E ) | id
*/

map<string, map<string, vector<string>>> table;  // Parsing table

bool isTerminal(const string &s) {
    return !(s == "E" || s == "E'" || s == "T" || s == "T'" || s == "F");
}

void initTable() {
    table["E"]["id"] = {"T", "E'"};
    table["E"]["("]  = {"T", "E'"};

    table["E'"]["+"] = {"+", "T", "E'"};
    table["E'"]["id"] = {"ε"};
    table["E'"]["("] = {"ε"};
    table["E'"][")"] = {"ε"};
    table["E'"]["$"] = {"ε"};

    table["T"]["id"] = {"F", "T'"};
    table["T"]["("]  = {"F", "T'"};

    table["T'"]["+"] = {"ε"};
    table["T'"]["*"] = {"*", "F", "T'"};
    table["T'"][")"] = {"ε"};
    table["T'"]["$"] = {"ε"};

    table["F"]["id"] = {"id"};
    table["F"]["("]  = {"(", "E", ")"};
}

int main() {
    initTable();

    // Input
    cout << "Enter token sequence (space separated). End with $:\n";
    // Example: id + id * id $
    
    vector<string> input;
    string tok;
    
    while (cin >> tok) {
        input.push_back(tok);
        if (tok == "$") break;
    }

    // Parsing stack
    stack<string> st;
    st.push("$");
    st.push("E");

    int ip = 0;  // pointer to input

    cout << "\nDerivation Steps:\n";

    while (!st.empty()) {
        string top = st.top();
        string cur = input[ip];

        cout << "Stack top: " << top << ", Input: " << cur << endl;

        if (top == cur && top == "$") {
            cout << "Parsing successful!\n";
            break;
        }
        else if (isTerminal(top)) {
            if (top == cur) {
                st.pop();
                ip++;
            } 
            else {
                cout << "Error: Terminal mismatch (" << top << " != " << cur << ")\n";
                return 0;
            }
        } 
        else {
            // top is non-terminal
            if (table[top].count(cur) == 0) {
                cout << "Error: No rule for (" << top << ", " << cur << ")\n";
                return 0;
            }

            vector<string> production = table[top][cur];
            st.pop();

            if (!(production.size() == 1 && production[0] == "ε")) {
                for (int i = production.size() - 1; i >= 0; i--)
                    st.push(production[i]);
            }

            cout << top << " → ";
            for (string s : production) cout << s << " ";
            cout << endl;
        }
    }

    return 0;
}
