#include <iostream>
#include <stack>
using namespace std;

int main() {
    string post;
    cout << "Enter postfix: ";
    cin >> post;

    stack<string> st;

    for (char c : post) {
        // Operand
        if (isalnum(c))
            st.push(string(1, c));

        // Operator
        else {
            string b = st.top(); st.pop();
            string a = st.top(); st.pop();

            string exp = "(" + a + c + b + ")";
            st.push(exp);
        }
    }

    cout << "Infix: " << st.top() << endl;
}
