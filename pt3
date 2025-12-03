#include <iostream>
#include <stack>
using namespace std;

int main() {
    string post;
    cout << "Enter postfix: ";
    cin >> post;

    stack<string> st;
    int t = 1;

    for(char c : post){
        if(isalnum(c)){
            st.push(string(1,c));
        }
        else{
            string b = st.top(); st.pop();
            string a = st.top(); st.pop();
            string temp = "t" + to_string(t++);
            cout << temp << " = " << a << " " << c << " " << b << endl;
            st.push(temp);
        }
    }

    cout << "Result in: " << st.top();
}
