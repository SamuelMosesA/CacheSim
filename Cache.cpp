#include<iostream>
#include<fstream>
#include<string.h>
#include <vector>

using namespace std;

class PLRUTree {

    int n, k; //n = 2^k -1 elements
    vector<bool> tree;

    static int left(int i) {
        return 2 * i + 1;
    }

    static int right(int i) {
        return 2 * i + 2;
    }


public:
    PLRUTree(int assc, int k);

    int replace();

    void read(int ind);
};

PLRUTree::PLRUTree(int assc, int k) {
    n = assc - 1;
    this->k = k;
    tree.resize(n, 0);
}

int PLRUTree::replace() {
    int i = 0;
    while (i < n) {
        bool bit = tree[i];
        tree[i] = !bit;
        if (bit) {
            i = right(i);
        } else {
            i = left(i);
        }
    }

    return i - n;
}

void PLRUTree::read(int ind) {
    int i = 0;
    for (int j = k; j >= 0; --j) {
        bool bit = (ind & (1 << j)) > 0;
        tree[i] = !bit;
        if (bit)
            i = right(i);
        else
            i = left(i);
    }
}


class Cache {
    vector<vector<string>> cache;
    vector<PLRUTree> cacheTrees;
    long capacity;
    long sets;
    long size;
    int ways;
    int replacementPolicy;
    long block;
public:
    Cache();

    ~Cache();

    void read(string address);

    void write(string address);

    int searchTagArray(int ind, int tag);
};

Cache::Cache() {
    cin >> size >> block >> ways >> replacementPolicy;
    capacity = size / block;
    if (ways == 0) {
        sets = 1;
        cache.resize(1);
        cache[0].resize(capacity);
    } else {
        sets = capacity / ways;
        cache.resize(sets);
        for (int i = 0; i < sets; i++) {
            cache[i].resize(ways);
        }
    }
    if (ways >= 2 && replacementPolicy == 2) {
        int k = 0;
        while ((ways & (1 << k))==0)
            ++k;
        cacheTrees.resize(sets, PLRUTree(ways, k));
    }
}


void Cache::read(string address) {
    if (replacementPolicy == 2) {
        int ind=0, tag = 0; //from address;
        int res = searchTagArray(ind, tag);
        if (res != -1) {
            cacheTrees[ind].read(res);
            //increment the counters
        }else{
            cacheTrees[ind].replace();
            //increment the counters
        }
    }
}

void Cache::write(string address) {
    if (replacementPolicy == 2) {
        int ind=0, tag = 0; //from address;
        int res = searchTagArray(ind, tag);
        if (res != -1) {
            cacheTrees[ind].read(res);
            //increment the counters
        }else{
            cacheTrees[ind].replace();
            //increment the counters
        }
    }
    //dirty bit
}

int Cache::searchTagArray(int ind, int tag) {
    return 0;
}

int main() {
    Cache c;
    string inp;
    cin >> inp;
    ifstream addresses(inp);
    while (getline(addresses, inp))
        c.read(inp);
    addresses.close();
    return 0;
}
