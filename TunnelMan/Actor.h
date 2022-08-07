#ifndef ACTOR_H_
#define ACTOR_H_

//#include "GraphObject.h"
#include "GameObject.h"

class StudentWorld;

//For each object, you must specify: imageID, x, y at minimum
class Earth : public GameObject {
public:
    Earth(int x, int y);

    int getX() const;
    int getY() const;
    
    virtual ~Earth();
private:
    int x;
    int y;
};

class TunnelMan : public GameObject {
public:
    TunnelMan(StudentWorld* sw);
    
    virtual void doSomething();
    
    int getX() const;
    int getY() const;
    
    virtual ~TunnelMan();

private:
    int hitPoints;
    int water;
    int sonar;
    int nuggets;
    //Anchor point x and y
    int x;
    int y;
    
    StudentWorld* m_studentWorld;
};

#endif // ACTOR_H_
