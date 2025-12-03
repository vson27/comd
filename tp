#include <iostream>
#include <stack>
using namespace std;

int prec(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

int main() {
    string infix, postfix;
    cout << "Enter infix: ";
    cin >> infix;

    stack<char> st;

    for (char c : infix) {
        // Operand
        if (isalnum(c))
            postfix += c;

        // Left bracket
        else if (c == '(')
            st.push(c);

        // Right bracket
        else if (c == ')') {
            while (st.top() != '(') {
                postfix += st.top();
                st.pop();
            }
            st.pop(); // remove '('
        }

        // Operator
        else {
            while (!st.empty() && prec(st.top()) >= prec(c)) {
                postfix += st.top();
                st.pop();
            }
            st.push(c);
        }
    }

    // Remaining operators
    while (!st.empty()) {
        postfix += st.top();
        st.pop();
    }

    cout << "Postfix: " << postfix << endl;
}
