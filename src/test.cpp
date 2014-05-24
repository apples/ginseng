#include "../include/ginseng/ginseng.hpp"

#include <iostream>

using namespace std;

using DB = Ginseng::Database<>;
using Ginseng::Not;
using EntID = DB::EntID;
using ComID = DB::ComID;

struct A
{
    int a;
};

struct B
{
    double b;
};

struct C
{
    string c;
};

mt19937& rng();
void random_action(DB& db);
void add_random_entity(DB& db);
void add_random_component(DB& db, EntID eid);
void random_query(DB& db);
void random_delete(DB& db);
void random_delete_entity(DB& db);
void random_delete_component(DB& db);
void random_slaughter(DB& db);

int main()
{
    DB db;

    for (int i=0; i<10000; ++i)
    {
        random_action(db);
    }
}

mt19937& rng()
{
    static mt19937 mt;
    return mt;
}

void random_action(DB& db)
{
    int roll = rng()()%10;
    
    switch (roll)
    {
        case 0: random_query(db); break;
        case 1: random_delete(db); break;
        case 2: random_slaughter(db); break;
        default: add_random_entity(db); break;
    }
}

void add_random_entity(DB& db)
{
    EntID eid = db.makeEntity();
    
    for (int i=0; i<3; ++i)
    {
        int roll = rng()()%3;
        if (roll==0) continue;
        add_random_component(db, eid);
    }
}

void add_random_component(DB& db, EntID eid)
{
    int roll = rng()()%3;
    
    static string strs[] = {"poop","dick","butt"};
    
    switch (roll)
    {
        case 0: db.makeComponent(eid, A{int(rng()()%10)}); break;
        case 1: db.makeComponent(eid, B{(rng()()%100)/100.0}); break;
        case 2: db.makeComponent(eid, C{strs[rng()()%3]}); break;
    }
}

void random_query(DB& db)
{
    int roll = rng()()%15;
    
    switch (roll)
    {
        case 0: cout << db.query<>().size() << endl; break;
        case 1: cout << db.query<A>().size() << endl; break;
        case 2: cout << db.query<B>().size() << endl; break;
        case 3: cout << db.query<C>().size() << endl; break;
        case 4: cout << db.query<A,B>().size() << endl; break;
        case 5: cout << db.query<A,C>().size() << endl; break;
        case 6: cout << db.query<B,C>().size() << endl; break;
        case 7: cout << db.query<A,B,C>().size() << endl; break;
        case 8: cout << db.query<Not<A>>().size() << endl; break;
        case 9: cout << db.query<Not<B>>().size() << endl; break;
        case 10: cout << db.query<Not<C>>().size() << endl; break;
        case 11: cout << db.query<Not<A>,B>().size() << endl; break;
        case 12: cout << db.query<Not<A>,C>().size() << endl; break;
        case 13: cout << db.query<Not<B>,C>().size() << endl; break;
        case 14: cout << db.query<Not<A>,B,C>().size() << endl; break;
    }
}

void random_delete(DB& db)
{
    int roll = rng()()%2;
    
    if (roll==0)
    {
        random_delete_entity(db);
    }
    else
    {
        random_delete_component(db);
    }
}

void random_delete_entity(DB& db)
{
    auto all = db.query<>();
    if (all.empty()) return;
    int roll = rng()()%all.size();
    db.eraseEntity(get<0>(all[roll]));
}

void random_delete_component(DB& db)
{
    int roll = rng()()%3;
    
    switch (roll)
    {
        case 0:
        {
            auto all = db.query<A>();
            if (all.empty()) return;
            int roll = rng()()%all.size();
            db.eraseComponent(get<1>(all[roll]).second);
        } break;
        
        case 1:
        {
            auto all = db.query<B>();
            if (all.empty()) return;
            int roll = rng()()%all.size();
            db.eraseComponent(get<1>(all[roll]).second);
        } break;
        
        case 2:
        {
            auto all = db.query<C>();
            if (all.empty()) return;
            int roll = rng()()%all.size();
            db.eraseComponent(get<1>(all[roll]).second);
        } break;
    }
}

void random_slaughter(DB& db)
{
    int roll = rng()()%3;
    
    switch (roll)
    {
        case 0:
        {
            auto all = db.query<A>();
            for (auto&& e : all)
                db.eraseEntity(get<0>(e));
        } break;
        
        case 1:
        {
            auto all = db.query<B>();
            for (auto&& e : all)
                db.eraseEntity(get<0>(e));
        } break;
        
        case 2:
        {
            auto all = db.query<C>();
            for (auto&& e : all)
                db.eraseEntity(get<0>(e));
        } break;
        
    }
}
