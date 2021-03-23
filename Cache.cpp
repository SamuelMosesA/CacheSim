
#include<iostream>
#include<fstream>
#include<string.h>
#include <vector>

using namespace std;

long int cacheRef, cacheMiss, readAcc, writeAcc, compMiss, capMiss, confMiss, readMiss, writeMiss, dirtyEvict;

struct CacheBlock{
	string tag;
	int valid;
	int dirty;
	CacheBlock();
};

CacheBlock::CacheBlock()
{
	valid = 0;
}

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
    vector<vector<CacheBlock>> cache;
    vector<PLRUTree> cacheTrees;
    long capacity;
    long sets;
    long size;
    int ways;
    int replacementPolicy;
    long block;
    int blockOffset;
    int setOffset;
protected:
    void LRURW(string address);
    void PsuedoLRURW(string address);
    void RandomRW(string address);
    void DirectMappedRW(string address);
    string addressConversion(string address);
public:
    Cache();
    ~Cache();
    void load(string address);
};

Cache::Cache() {
    cin >> size >> block >> ways >> replacementPolicy;
    capacity = size / block;
    if (ways == 0) {
        sets = 1;
        cache.resize(1);
        cache[0].resize(capacity);
        ways = capcity;
    } else {
        sets = capacity / ways;
        cache.resize(sets);
        for (int i = 0; i < sets; i++) {
            cache[i].resize(ways);
        }
    }
    
    blockOffset = 0;
    for(int j=2; j<=block; j++)
    	blockOffset++;
    setOffset = 0;
    for(int l=2; l<=sets; l++)
    	setOffset++;
    	
    if (ways >= 2 && replacementPolicy == 2) {
        int k = 0;
        while ((ways & (1 << k))==0)
            ++k;
        cacheTrees.resize(sets, PLRUTree(ways, k));
    }
}

void LRURW(string address){

}

void PsuedoLRURW(string address){

}

void RandomRW(string address){

}

void DirectMappedRW(string address){
	CacheBlock b;
	b.tag = address.substr(1,31-blockOffset-setOffset);
	b.valid = 1;
	string set = address.substr(32-blockOffset-setOffset, setOffset);
	long s = 0, i;
	std::string::reverse_iterator it1; 
	for (it1=str.rbegin(), i=1; it1!=str.rend(); it1++, i*=2)
	{
		if(*it == '1')
			s+=i;
	} 
	if(cache[s][0].tag == b.tag)
	{
		if(address.at(0)=='1')
		{
			cache[s][0].dirty = 1;
			writeAcc++;	
		}
		else
		{
			readAcc++;
		}
	}
	else
	{
		confMiss++;
		if(address.at(0)=='1')
		{
			b.dirty = 1;
			cache[s][0] = b;
			writeAcc++;
			writeMiss++;
		}
		else
		{
			b.dirty = 0;
			cache[s][0] = b;
			readAcc++;
			readMiss++;
		}
	}
}

void Cache::addressConversion(string address)
{
	string ba;
	int l = str.length(); 
	char ch;
    	for (int i = 0; i < l; i++) { 
        	ch = str.at(i); 
        	switch(ch){
        		case '0': ba = ba + "0000";
        		continue;
        		case '1': ba = ba + "0001";
        		continue;
        		case '2': ba = ba + "0010";
        		continue;
        		case '3': ba = ba + "0011";
        		continue;
        		case '4': ba = ba + "0100";
        		continue;
        		case '5': ba = ba + "0101";
        		continue;
        		case '6': ba = ba + "0110";
        		continue;
        		case '7': ba = ba + "0111";
        		continue;
        		case '8': ba = ba + "1000";
        		continue;
        		case '9': ba = ba + "1001";
        		continue;
        		case 'A': ba = ba + "1010";
        		continue;
        		case 'B': ba = ba + "1011";
        		continue;
        		case 'C': ba = ba + "1100";
        		continue;
        		case 'D': ba = ba + "1101";
        		continue;
        		case 'E': ba = ba + "1110";
        		continue;
        		case 'F': ba = ba + "1111";
        		continue;
        	}
        }
        return ba;
}

void Cache::load(string address)
{
	string binaryAddress = addressConversion(address);
	if(ways==1)
	{
		DirectMappedRW(binaryAddress);
	}
	else if(replacementPolicy == 0)
	{
		RandomRW(binaryAddress);
	}
	else if(replacementPolicy == 1)
	{
		LRURW(binaryAddress);
	}
	else
	{
		PsuedoLRURW(binaryAddress);	
	}
}

int main() {
    Cache c;
    string inp;
    cin >> inp;
    ifstream addresses(inp);
    while (getline(addresses, inp))
        c.load(inp);
    addresses.close();
    return 0;
}
