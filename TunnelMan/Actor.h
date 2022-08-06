#ifndef ACTOR_H_
#define ACTOR_H_

//#include "GraphObject.h"
#include "GameObject.h"

//For each object, you must specify: imageID, x, y at minimum

class Earth : public GameObject {
public:
    Earth(int x, int y);

    virtual ~Earth();
};

class TunnelMan : public GameObject {
public:
    TunnelMan();
    
    virtual void doSomething();
    
    virtual ~TunnelMan();

private:
    int hitPoints;
    int water;
    int sonar;
    int nuggets;
    int x;
    int y;
};

#endif // ACTOR_H_
