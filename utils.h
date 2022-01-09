#ifndef UTILS_H
#define UTILS_H

#include <bits/stdc++.h>
using namespace std;

namespace Utils {
    string red(string str) { return "\x1B[31m" + str + "\033[0m"; }
    string green(string str) { return "\x1B[32m" + str + "\033[0m"; }

    vector<string> checkForCycles(set<string>& nodes, map<string, vector<string>>& graph) {
        map<string, string> father;
        vector<string> cycle;
        function<void(string, string)> dfs = [&](string node, string fath) {
            father[node] = fath;
            for (string nghb : graph[node])
                if (father[nghb] == "")
                    dfs(nghb, node);
                else if (cycle.empty()) {
                    while (node != "$") {
                        cycle.push_back(node);
                        node = father[node];
                    }
                    reverse(cycle.begin(), cycle.end());
                }
        };
        for (string node : nodes)
            if (father[node] == "")
                dfs(node, "$");
        return cycle;
    }
};

#endif
