// cfg_to_cnf.cpp
// Robust CFG -> CNF converter
// Compile: g++ cfg_to_cnf.cpp -O2 -std=c++17 -o cfg_to_cnf
// Run: ./cfg_to_cnf
//
// Uploaded file (for reference): /mnt/data/landmarks.py

#include <bits/stdc++.h>
using namespace std;

static const string EPS = "#";
static const string END = "$";

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Split alternatives by top-level '|'
vector<string> split_alts(const string &rhs) {
    vector<string> res;
    string cur;
    for (char c : rhs) {
        if (c == '|') {
            res.push_back(trim(cur));
            cur.clear();
        } else cur.push_back(c);
    }
    if (!cur.empty()) res.push_back(trim(cur));
    return res;
}

// Tokenize RHS: prefer whitespace split; if none, group alnum/'/_ sequences and single-char punctuation
vector<string> tokenize_rhs(const string &s) {
    if (s.find_first_of(" \t") != string::npos) {
        vector<string> out;
        string token;
        stringstream ss(s);
        while (ss >> token) out.push_back(token);
        return out;
    }
    vector<string> out;
    string cur;
    for (size_t i = 0; i < s.size();) {
        char c = s[i];
        if (isspace((unsigned char)c)) { ++i; continue; }
        if (isalnum((unsigned char)c) || c=='\'' || c=='_') {
            cur.push_back(c); ++i;
            while (i < s.size() && (isalnum((unsigned char)s[i]) || s[i]=='\'' || s[i]=='_')) {
                cur.push_back(s[i]); ++i;
            }
            out.push_back(cur);
            cur.clear();
            continue;
        }
        // single character token (symbols like +,*,(,))
        out.push_back(string(1, c));
        ++i;
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

bool isNonTerminal(const string &sym, const unordered_set<string> &nonterms) {
    return nonterms.find(sym) != nonterms.end();
}

// compute generating (can derive terminals) set
unordered_set<string> compute_generating(const unordered_map<string, vector<vector<string>>> &G,
                                         const unordered_set<string> &nonterms) {
    unordered_set<string> gen;
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &p : G) {
            const string &A = p.first;
            if (gen.count(A)) continue;
            for (auto &rhs : p.second) {
                bool ok = true;
                for (auto &sym : rhs) {
                    if (sym == EPS) continue;
                    if (nonterms.count(sym) && !gen.count(sym)) { ok = false; break; }
                    // terminals are fine
                }
                if (ok) {
                    gen.insert(A);
                    changed = true;
                    break;
                }
            }
        }
    }
    return gen;
}

// compute reachable from start
unordered_set<string> compute_reachable(const unordered_map<string, vector<vector<string>>> &G,
                                        const string &start) {
    unordered_set<string> reach;
    queue<string> q;
    reach.insert(start); q.push(start);
    while (!q.empty()) {
        auto A = q.front(); q.pop();
        auto it = G.find(A);
        if (it == G.end()) continue;
        for (auto &rhs : it->second) {
            for (auto &sym : rhs) {
                if (reach.find(sym) == reach.end()) {
                    // only push nonterminals stored as keys in G
                    if (G.find(sym) != G.end()) {
                        reach.insert(sym);
                        q.push(sym);
                    }
                }
            }
        }
    }
    return reach;
}

// remove non-generating and unreachable symbols (cleanup)
void prune_useless(unordered_map<string, vector<vector<string>>> &G, unordered_set<string> &nonterms, const string &start) {
    bool again = true;
    while (again) {
        again = false;
        auto gen = compute_generating(G, nonterms);
        vector<string> todel;
        for (auto &nt : nonterms) {
            if (!gen.count(nt)) todel.push_back(nt);
        }
        if (!todel.empty()) {
            for (auto &d : todel) { G.erase(d); nonterms.erase(d); }
            again = true;
            continue;
        }
        auto reach = compute_reachable(G, start);
        vector<string> toremove;
        for (auto &nt : nonterms) if (!reach.count(nt)) toremove.push_back(nt);
        if (!toremove.empty()) {
            for (auto &d : toremove) { G.erase(d); nonterms.erase(d); }
            again = true;
        }
    }
}

// produce unique key for RHS
string rhs_key(const vector<string> &rhs) {
    string k;
    for (size_t i=0;i<rhs.size();++i) {
        if (i) k.push_back(' ');
        k += rhs[i];
    }
    return k;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Enter number of productions: " << flush;
    int n;
    if (!(cin >> n)) return 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Use '#' for epsilon. Enter productions like: S -> A B | a\n" << flush;

    unordered_map<string, vector<vector<string>>> G;
    unordered_set<string> nonterms;
    vector<pair<string, vector<vector<string>>>> read_order;

    // Read productions
    for (int i=0;i<n;) {
        string line;
        if (!std::getline(cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        size_t arrow = line.find("->");
        if (arrow == string::npos) {
            cerr << "Invalid production (no ->): " << line << "\n";
            return 1;
        }
        string lhs = trim(line.substr(0, arrow));
        string rhs_all = trim(line.substr(arrow+2));
        if (lhs.empty() || rhs_all.empty()) { cerr << "Invalid production: " << line << "\n"; return 1; }
        nonterms.insert(lhs);
        vector<string> alts = split_alts(rhs_all);
        for (auto &alt : alts) {
            auto toks = tokenize_rhs(alt);
            if (toks.size()==1 && toks[0]==EPS) G[lhs].push_back({EPS});
            else G[lhs].push_back(toks);
        }
        ++i;
    }

    // read start symbol robustly
    string START;
    while (true) {
        cout << "Enter start symbol: " << flush;
        if (!getline(cin, START)) { cerr << "No start symbol read\n"; return 1; }
        START = trim(START);
        if (!START.empty()) break;
    }
    if (!nonterms.count(START)) {
        cerr << "Start symbol not present in LHS nonterminals\n";
        return 1;
    }

    // Initial cleanup: ensure all RHS symbols are tracked (terminals vs nonterms)
    unordered_set<string> terminals;
    for (auto &p : G) for (auto &rhs : p.second) for (auto &s : rhs) {
        if (s == EPS) continue;
        // if not a known nonterminal, for now consider terminal (nonterms may be added later)
        if (!nonterms.count(s)) terminals.insert(s);
    }

    // Step 1: Remove useless symbols (non-generating/unreachable) initially
    prune_useless(G, nonterms, START);
    // recompute terminals
    terminals.clear();
    for (auto &p : G) for (auto &rhs : p.second) for (auto &s : rhs) if (s!=EPS && !nonterms.count(s)) terminals.insert(s);

    // Step 2: Compute nullable set
    unordered_set<string> nullable;
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &p : G) {
            const string &A = p.first;
            if (nullable.count(A)) continue;
            for (auto &rhs : p.second) {
                bool allNull = true;
                for (auto &sym : rhs) {
                    if (sym == EPS) continue;
                    if (nonterms.count(sym)) {
                        if (!nullable.count(sym)) { allNull = false; break; }
                    } else { allNull = false; break; }
                }
                if (allNull) { nullable.insert(A); changed = true; break; }
            }
        }
    }
    bool start_nullable = nullable.count(START);

    // Step 3: Epsilon removal
    unordered_map<string, vector<vector<string>>> G_eps;
    for (auto &p : G) {
        const string &A = p.first;
        for (auto &rhs : p.second) {
            // positions of nullable nonterminals
            vector<int> pos;
            for (int i=0;i<(int)rhs.size();++i)
                if (nonterms.count(rhs[i]) && nullable.count(rhs[i])) pos.push_back(i);
            int m = pos.size();
            int subsets = 1<<m;
            for (int mask=0; mask<subsets; ++mask) {
                vector<string> newrhs;
                for (int i=0;i<(int)rhs.size();++i) {
                    bool remove=false;
                    for (int j=0;j<m;++j) if ((mask>>j)&1) if (pos[j]==i) { remove=true; break; }
                    if (!remove) newrhs.push_back(rhs[i]);
                }
                if (newrhs.empty()) {
                    G_eps[A].push_back({EPS});
                } else {
                    G_eps[A].push_back(newrhs);
                }
            }
        }
    }
    // Remove epsilon productions for non-starts
    for (auto &p : G_eps) {
        vector<vector<string>> tmp;
        for (auto &rhs : p.second) {
            if (rhs.size()==1 && rhs[0]==EPS) {
                if (p.first==START && start_nullable) tmp.push_back(rhs);
                // else drop
            } else tmp.push_back(rhs);
        }
        p.second = tmp;
    }
    G.swap(G_eps);

    // Step 4: Remove unit productions using closure method
    // for each A compute unit-closure (including A)
    unordered_map<string, unordered_set<string>> unit_closure;
    for (auto &p : G) {
        const string &A = p.first;
        // BFS
        unordered_set<string> vis;
        queue<string> q;
        vis.insert(A); q.push(A);
        while (!q.empty()) {
            string X = q.front(); q.pop();
            for (auto &rhs : G[X]) {
                if (rhs.size()==1 && nonterms.count(rhs[0])) {
                    string Y = rhs[0];
                    if (!vis.count(Y)) { vis.insert(Y); q.push(Y); }
                }
            }
        }
        unit_closure[A] = vis;
    }
    // build new grammar without unit rules
    unordered_map<string, vector<vector<string>>> G_no_unit;
    for (auto &p : G) {
        const string &A = p.first;
        unordered_set<string> seenRhsKeys;
        for (auto &B : unit_closure[A]) {
            for (auto &rhs : G[B]) {
                if (rhs.size()==1 && nonterms.count(rhs[0])) continue; // unit rule - skip
                string key = rhs_key(rhs);
                if (!seenRhsKeys.count(key)) {
                    G_no_unit[A].push_back(rhs);
                    seenRhsKeys.insert(key);
                }
            }
        }
    }
    G.swap(G_no_unit);

    // prune useless again
    prune_useless(G, nonterms, START);

    // recompute terminals
    terminals.clear();
    for (auto &p : G) for (auto &rhs : p.second) for (auto &s : rhs) if (s!=EPS && !nonterms.count(s)) terminals.insert(s);

    // Step 5: Replace terminals that occur in RHS length > 1 with new variables
    unordered_map<string,string> termVar;
    int termVarCounter = 1;
    for (auto &p : G) {
        for (auto &rhs : p.second) {
            if (rhs.size() > 1) {
                for (auto &sym : rhs) {
                    if (!nonterms.count(sym) && sym != EPS) {
                        if (!termVar.count(sym)) {
                            string v = "T_" + to_string(termVarCounter++);
                            termVar[sym] = v;
                        }
                    }
                }
            }
        }
    }
    // add productions for those terminal variables
    for (auto &tv : termVar) {
        G[tv.second].push_back(vector<string>{tv.first});
        nonterms.insert(tv.second);
    }
    // replace terminals in long RHS
    for (auto &p : G) {
        for (auto &rhs : p.second) {
            if (rhs.size() > 1) {
                for (auto &sym : rhs) {
                    if (!nonterms.count(sym) && sym != EPS && termVar.count(sym)) sym = termVar[sym];
                }
            }
        }
    }

    // Step 6: Binarize RHS with length > 2
    unordered_map<string, vector<vector<string>>> G_bin;
    int freshVarCounter = 1;
    for (auto &p : G) {
        const string &A = p.first;
        for (auto rhs : p.second) {
            if (rhs.size() <= 2) {
                G_bin[A].push_back(rhs);
                continue;
            }
            // create chain: A -> X1 V1 ; V1 -> X2 V2 ; ... ; Vk -> Xk Y
            vector<string> work = rhs;
            string left = A;
            while (work.size() > 2) {
                string newV = "X_" + to_string(freshVarCounter++);
                // left -> work[0] newV
                G_bin[left].push_back(vector<string>{work[0], newV});
                nonterms.insert(newV);
                // shift work: remove first
                work.erase(work.begin());
                left = newV;
            }
            // final rule left -> work[0] work[1]
            G_bin[left].push_back(work);
        }
    }

    // merge G_bin into G (G_bin is new grammar)
    // deduplicate RHS lists
    for (auto &p : G_bin) {
        auto &vec = p.second;
        sort(vec.begin(), vec.end());
        vec.erase(unique(vec.begin(), vec.end()), vec.end());
    }

    // final grammar is G_bin
    G.swap(G_bin);

    // Final prune to remove any newly useless symbols
    prune_useless(G, nonterms, START);

    // Final normalization: ensure rules are either A -> a (single terminal)
    // or A -> B C (two nonterminals), and possibly START -> # if start_nullable
    // Remove any stray rules violating that (should not exist)
    // but ensure that single-symbol rules that are nonterminals are not present (should be removed by unit removal)

    // Print final CNF grammar
    cout << "\n=== CNF Grammar ===\n";
    // Sort for stable output
    vector<string> lhslist;
    for (auto &p : G) lhslist.push_back(p.first);
    sort(lhslist.begin(), lhslist.end());
    for (auto &A : lhslist) {
        cout << A << " -> ";
        auto &vec = G[A];
        for (size_t i=0;i<vec.size();++i) {
            for (size_t j=0;j<vec[i].size();++j) {
                if (j) cout << ' ';
                cout << vec[i][j];
            }
            if (i+1<vec.size()) cout << " | ";
        }
        cout << "\n";
    }

    if (!termVar.empty()) {
        cout << "\nTerminal mappings:\n";
        for (auto &tv : termVar) cout << tv.second << " -> " << tv.first << "\n";
    }

    cout << "\nDone.\n";
    return 0;
}

—-----------------------------
Inputs
—-----------------------------
5
S -> A B | a
A -> a A | #
B -> b B | #
C -> c
D -> a B
S


