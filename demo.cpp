#include <Serializer.h>
using namespace bdf;

#include <iostream>
#include <sstream>
#include <vector>
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
    stringstream ss;
    auto s = serializer(ss);

    {
        ss.clear();
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

    return 0;
}
