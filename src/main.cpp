#include "ginseng.hpp"

#include <iostream>
using namespace std;

using namespace Ginseng;

struct Alpha
    : public Component<Alpha>
{
    int val = 1;
    
    virtual ostream& debugPrint(ostream& out) const override
    {
        out << "Alpha: " << val;
        return out;
    }
};

struct Beta
    : public Component<Beta>
{
    int val = 2;
    
    virtual ostream& debugPrint(ostream& out) const override
    {
        out << "Beta: " << val;
        return out;
    }
};

int main()
{
    Database el;
    
    el.registerComponent<Alpha>();
    el.registerComponent<Beta>();
    
    auto e1 = el.newEntity();
    auto e2 = el.newEntity();
    auto e3 = el.newEntity();
    
    auto c1a = el.newComponent<Alpha>(e1);
    auto c1b = el.newComponent<Beta>(e1);
    
    auto c2a = el.newComponent<Alpha>(e2);
    
    auto c3b = el.newComponent<Beta>(e3);
    
    for (auto&& i : el.getEntities<Alpha>())
    {
        cout << get<1>(i)->val << endl;
    }
    
    for (auto&& i : el.getEntities<Beta>())
    {
        cout << get<1>(i)->val << endl;
    }
    
    for (auto&& i : el.getEntities<Alpha,Beta>())
    {
        cout << get<1>(i)->val << get<2>(i)->val << endl;
    }
    
    el.debugPrint(cout);
}
