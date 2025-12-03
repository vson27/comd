#include <iostream>
#include <vector>
using namespace std;

int main() {
    string exp;
    cout << "Enter infix expression: ";
    cin >> exp;

    vector<string> e;
    for(char c: exp) e.push_back(string(1,c));

    int t=1;

    // First handle * and /
    for(int i=0;i<e.size();i++){
        if(e[i]=="*"||e[i]=="/"){
            cout<<"t"<<t<<" = "<<e[i-1]<<" "<<e[i]<<" "<<e[i+1]<<endl;
            e.erase(e.begin()+i-1,e.begin()+i+2);
            e.insert(e.begin()+i-1,"t"+to_string(t++));
            i=0;
        }
    }

    // Then handle + and -
    for(int i=0;i<e.size();i++){
        if(e[i]=="+"||e[i]=="-"){
            cout<<"t"<<t<<" = "<<e[i-1]<<" "<<e[i]<<" "<<e[i+1]<<endl;
            e.erase(e.begin()+i-1,e.begin()+i+2);
            e.insert(e.begin()+i-1,"t"+to_string(t++));
            i=0;
        }
    }

    cout<<"Final result in: "<<e[0]<<endl;
}
