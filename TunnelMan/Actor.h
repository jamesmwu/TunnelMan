#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

/*========== GameObject base class ==========*/
class GameObject : public GraphObject {
public:
    //Default values of GraphObject constructor
    GameObject(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0);
    
    virtual void doSomething(){}
    
    int getX() const;
    void updateX(int val);
    
    int getY() const;
    void updateY(int val);
    
    virtual ~GameObject();

private:
    int x;
    int y;
    
};

/*========== Earth class ==========*/
class Earth : public GameObject {
public:
    //For each object, you must specify: imageID, x, y at minimum
    Earth(int x, int y);
    
    virtual ~Earth();

};

/*========== Boulder class ==========*/
class Boulder : public GameObject {
public:
    Boulder(int x, int y, StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual ~Boulder();
    
private:
    bool alive;
    std::string state;
    
    StudentWorld* m_studentWorld;
};

/*========== TunnelMan class ==========*/
class TunnelMan : public GameObject {
public:
    TunnelMan(StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual ~TunnelMan();

private:
    int hitPoints;
    int water;
    int sonar;
    int nuggets;
    
    StudentWorld* m_studentWorld;
};

#endif // ACTOR_H_
