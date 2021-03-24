#include<iostream>
#include<fstream>
#include<string>
#include <vector>
#include <bitset>
#include <set>
#include <list>
#include <iterator>
#include <utility>
using namespace std;

long int cacheRef, cacheMiss, readAcc, writeAcc, compMiss, capMiss, confMiss, readMiss, writeMiss, dirtyEvict;

struct CacheBlock{
	string tag;
	int valid;
	int dirty;
	CacheBlock();
	CacheBlock(string, int, int);
};

CacheBlock::CacheBlock()
{
	tag = "";
	valid = 0;
	dirty = 0;
}

CacheBlock::CacheBlock(string t, int v, int d)
{
	tag = t;
	valid = v;
	dirty = d;
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
	vector<list<CacheBlock>> LRUList;
    vector<PLRUTree> cacheTrees;
    long capacity;
    long sets;
    long size;
    int ways;
    int replacementPolicy;
    long block;
    int blockOffset;
    int setOffset;
    set<pair<string,unsigned long>> seenAddresses;

protected:
    void LRURW(const string& tag, const unsigned long &set, bool &write);
    void PsuedoLRURW(const string& tag, const unsigned long &set, bool &write);
    void RandomRW(const string& tag, const unsigned long &set, bool &write);
    void DirectMappedRW(const string& tag, const unsigned long &set, bool &write);
    string addressConversion(string address);
    int searchTagArray(unsigned int set, const string& tag);
public:
    Cache();
    void load(string address);
};

Cache::Cache() {
    cin >> size >> block >> ways >> replacementPolicy;
    capacity = size / block;
    if (ways == 0) {
        sets = 1;
        cache.resize(1);
		LRUList.resize(1);
        cache[0].resize(capacity);
        LRUList[0].resize(capacity);
        ways = capacity;
    } else {
        sets = capacity / ways;
        cache.resize(sets);
		LRUList.resize(sets);
        for (int i = 0; i < sets; i++) {
            cache[i].resize(ways);
			LRUList[i].resize(ways);
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

int Cache::searchTagArray(unsigned int set, const string& tag) {
    int res = -1;
    auto currSet = cache[set];
    if(ways>0) {
        for (int i = 0; i < ways; ++i) {
            if (currSet[i].valid==1 && currSet[i].tag == tag) {
                res = i;
                break;
            }
        }
    }
    return res;
}

void Cache:: LRURW(const string& tag, const unsigned long &set, bool &write){
	list<CacheBlock>::iterator it;
	int v, d;
	for (it = LRUList[set].begin(); it != LRUList[set].end(); it++){
		if(it->tag == tag) {			// Cache hit
			v = (*it).valid;
			d = write&(it->dirty);
			LRUList[set].erase(it);
			break;
		}
	}
	if(it == LRUList[set].end()) {			// Element not in cache
		cacheMiss++;
		write? writeMiss++:readMiss++;
		it = LRUList[set].begin();
		while(it->valid == 1 && it != LRUList[set].end()) it++;
		if(it!= LRUList[set].end() && it->valid == 0) {		// Found unassigned cache block
			LRUList[set].erase(it);
			v = 1;
			d = write;
		}
		else {					// Cache is full
			confMiss++;
			if(sets == 1) capMiss++;
			CacheBlock temp = LRUList[set].back();
			if(temp.dirty) {			// Dirty block eviction
				dirtyEvict++;
			}
			LRUList.pop_back();		// Remove last element
			v = 1;
			d = write;
		}
	}
	CacheBlock newBlock = {tag, v, d};
	LRUList[set].emplace_front(newBlock);
}

void Cache::PsuedoLRURW(const string& tag, const unsigned long &set, bool &write) {
    int hitInd = searchTagArray(set, tag), replaceInd;
    if (hitInd != -1) {//cache hit
        cacheTrees[set].read(hitInd);
        if(write) {
            cache[set][hitInd].dirty = 1;
            writeAcc++;
        }else {
            readAcc++;
        }
    }else{//miss
        CacheBlock b;
        b.tag = tag;
        b.valid =1;
        replaceInd = cacheTrees[set].replace();

        ++cacheMiss;
        if(seenAddresses.find({tag,set})!=seenAddresses.end())
            confMiss++;
        else
            ++compMiss;

        if(cache[set][replaceInd].dirty==1)
            ++dirtyEvict;


        if(write){
            b.dirty = 1;
            cache[set][replaceInd] = b;
            writeAcc++;
            writeMiss++;
        }else{
            b.dirty = 0;
            cache[set][replaceInd] = b;
            readAcc++;
            readMiss++;
    }
}


}

void Cache::RandomRW(const string& tag, const unsigned long &set, bool &write)
{
	int present  = searchTagArray(set, tag);
	if(present>=0)
	{
		if(write)
		{
			cache[set][present].dirty = 1;
			writeAcc++;
		}
		else
		{
			readAcc++;
		}
	}
	else
	{
		CacheBlock b;
		b.tag = tag;
		b.valid = 1;
		if(write)
		{
			b.dirty=1;
			writeAcc++;
		}
		else
		{
			b.dirty=0;
			readAcc++;
		}
		int vacant;
		for(vacant = 0; vacant < ways; vacant++)
			if(cache[set][vacant].valid == 0)
				break;
		if(vacant < ways)
		{
			cache[set][vacant] = b;
			compMiss++;
		}
		else
		{
			vacant = rand() % ways;
			if(seenAddresses.find({tag,set})!=seenAddresses.end())
	           		confMiss++;
	        	else
	            		++compMiss;
	            	if(cache[set][vacant].dirty==1)
	            		dirtyEvict++;
	            	cache[set][vacant] = b;
		}
	}
}

void Cache::DirectMappedRW(const string& tag, const unsigned long &set, bool &write)
{
	CacheBlock b;
	b.tag = tag;
	b.valid = 1;
	int present  = searchTagArray(set, tag);
	if(present==0)
	{
		if(write)
		{
			cache[set][0].dirty = 1;
			writeAcc++;	
		}
		else
		{
			readAcc++;
		}
	}
	else
	{   
		++cacheMiss;
	        if(seenAddresses.find({tag,set})!=seenAddresses.end())
	            confMiss++;
	        else
	            ++compMiss;

		if(write)
		{
			b.dirty = 1;
			cache[set][0] = b;
			writeAcc++;
			writeMiss++;
		}
		else
		{
			b.dirty = 0;
			cache[set][0] = b;
			readAcc++;
			readMiss++;
		}
	}
}

string Cache::addressConversion(string address)
{
	string ba;
	int l = address.length();
	char ch;
    	for (int i = 0; i < l; i++) { 
        	ch = address.at(i);
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
    string setStr = binaryAddress.substr(32-blockOffset-setOffset, setOffset);
    bitset<6> setBitSet(setStr);

    unsigned long set = setBitSet.to_ulong();
    bool write = address.at(0)=='1';
    string tag = binaryAddress.substr(1,31-blockOffset-setOffset);

    ++cacheRef; //check if logic is correct

	if(ways==1)
	{
		DirectMappedRW(tag, set, write);
	}
	else if(replacementPolicy == 0)
	{
		RandomRW(tag, set, write);
	}
	else if(replacementPolicy == 1)
	{
		LRURW(tag, set, write);
	}
	else
	{
        PsuedoLRURW(tag, set, write);
	}
	seenAddresses.emplace(make_pair(tag,set));
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
