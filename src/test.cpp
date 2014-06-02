#include "../include/ginseng/ginseng.hpp"

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

using DB = Ginseng::Database<>;
using Ginseng::Not;
using EntID = DB::EntID;
using ComID = DB::ComID;

template <typename T>
void print(T&& t)
{
    //cout << t << endl;
}

template <typename D, typename F>
D bench(F&& f)
{
    auto b = steady_clock::now();
    f();
    auto e = steady_clock::now();
    return duration_cast<D>(e-b);
}

struct A
{
    volatile int a;
};

struct B
{
    volatile double b;
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

    auto work = [&]
    {
        for (int i=0; i<100000; ++i)
        {
            random_action(db);
        }
    };

    cout << bench<milliseconds>(work).count() << endl;
}

mt19937& rng()
{
    static mt19937 mt;
    return mt;
}

void random_action(DB& db)
{
    int roll = rng()()%50;

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
        case 0: print(db.query<>().size()); break;
        case 1: print(db.query<A>().size()); break;
        case 2: print(db.query<B>().size()); break;
        case 3: print(db.query<C>().size()); break;
        case 4: print(db.query<A,B>().size()); break;
        case 5: print(db.query<A,C>().size()); break;
        case 6: print(db.query<B,C>().size()); break;
        case 7: print(db.query<A,B,C>().size()); break;
        case 8: print(db.query<Not<A>>().size()); break;
        case 9: print(db.query<Not<B>>().size()); break;
        case 10: print(db.query<Not<C>>().size()); break;
        case 11: print(db.query<Not<A>,B>().size()); break;
        case 12: print(db.query<Not<A>,C>().size()); break;
        case 13: print(db.query<Not<B>,C>().size()); break;
        case 14: print(db.query<Not<A>,B,C>().size()); break;
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
