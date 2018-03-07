#include <Serializer.h>
using namespace bdf;

#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
using namespace std;

struct DB {
    typedef unsigned long Id;
    struct Table {
        struct Entry {
            Entry(const string & data_="") : data(data_) {}

            string data;
        private:

            SERIALIZE_MEMBERS(Entry, data)
        };

        Table(const string & descr_="") : _id(++lastId), _descr(descr_) {}
        Id id() const { return _id; }
        Entry & addEntry(const string & d="") { _entries.emplace_back(d); return _entries.back(); }
    private:
        static Id lastId;
        Id _id;
        string _descr;
        vector<Entry> _entries;

        SERIALIZE_MEMBERS(Table, _id, _descr, _entries)
    };

    Table & newTable(const string & descr="") { Table t(descr); return _tables[t.id()] = move(t); }
private:
    unordered_map<Id, Table> _tables;

    SERIALIZE_MEMBERS(DB,_tables)
};
DB::Id DB::Table::lastId=0;

int main() {

    {
        stringstream ss;
        auto s = serializer(ss);
        {
            DB db;
            {
                auto & t = db.newTable("First table");
                t.addEntry("a");
                t.addEntry("s");
                t.addEntry("d");
                t.addEntry("f");
            }
            {
                auto & t = db.newTable("Second table");
                t.addEntry("a");
                t.addEntry("s");
                t.addEntry("d");
                t.addEntry("f");
            }
            s << db;
        }
        {
            DB db;
            s >> db;
        }
    }

    {
        stringstream ss;
        auto s = serializer(ss);

        vector<char> aVector = {'a','s','d','f'};
        s << aVector;
        vector<char> bVector;
        s >> bVector;
        for(auto a: bVector) cout << a << ",";
        cout << endl;
        cout << "vector-> " << ss.str() << endl;
    }
    {
        stringstream ss;
        auto s = serializer(ss);

        array<char,4> aArray = {{'a','s','d','f'}};
        s << aArray;
        array<char,4> bArray;
        s >> bArray;
        for(auto a: bArray) cout << a << ",";
        cout << endl;
        cout << "array-> " << ss.str() << endl;
    }
    {
        stringstream ss;
        auto s = serializer(ss);

        set<char> aSet = {'a','s','d','f'};
        s << aSet;
        set<char> bSet;
        s >> bSet;
        for(auto a: bSet) cout << a << ",";
        cout << endl;
        cout << "set-> " << ss.str() << endl;
    }
    {
        stringstream ss;
        auto s = serializer(ss);

        unordered_set<char> aSet = {'a','s','d','f'};
        s << aSet;
        unordered_set<char> bSet;
        s >> bSet;
        for(auto a: bSet) cout << a << ",";
        cout << endl;
        cout << "unordered_set-> " << ss.str() << endl;
    }

    {
        stringstream ss;
        auto s = serializer(ss);

        map<char,char> aMap = {{'a','a'},{'b','b'},{'x','x'},};
        s << aMap;
        map<char,char> bMap;
        s >> bMap;
        for(auto a: bMap) cout << a.first << "=" << a.second << ",";
        cout << endl;
        cout << "map-> " << ss.str() << endl;
    }
    {
        stringstream ss;
        auto s = serializer(ss);

        unordered_map<char,char> aMap = {{'a','a'},{'b','b'},{'x','x'},};
        s << aMap;
        unordered_map<char,char> bMap;
        s >> bMap;
        for(auto a: bMap) cout << a.first << "=" << a.second << ",";
        cout << endl;
        cout << "unordered_map-> " << ss.str() << endl;
    }

    return 0;
}
