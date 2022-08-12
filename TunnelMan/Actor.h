#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class TunnelMan;

/*========== GameObject base class ==========*/
class GameObject : public GraphObject {
public:
    //Default values of GraphObject constructor
    GameObject(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0, TunnelMan* tm = nullptr, StudentWorld* sw = nullptr);
    
    virtual void doSomething();
    
    virtual void annoyed(int val);
    
    int getX() const;
    void updateX(int val);
    
    int getY() const;
    void updateY(int val);
    
    bool isAlive() const;
    void dead();
    void live();
    
    bool isBoulder() const;
    void imABoulder();
    
    bool isSonar() const;
    void imASonar();
    
    TunnelMan* tm() const;
    StudentWorld* sw() const;
    
    //Auxiliary function
    bool distance(int x, int y, int x2, int y2, int radius);
    
    virtual ~GameObject();

private:
    int x;
    int y;
    bool alive;
    bool bldr;
    bool sonar;
    TunnelMan* m_tunnelMan;
    StudentWorld* m_studentWorld;
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
    Boulder(int x, int y, StudentWorld* sw, TunnelMan* tm);
    
    virtual void doSomething();
    
    virtual ~Boulder();
    
private:
    std::string state;
    int tick;
};

/*========== Barrel class ==========*/
class Barrel : public GameObject {
public:
    Barrel(int x, int y, TunnelMan* tm, StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual ~Barrel();
    
};

/*========== Nugget class ==========*/
class Nugget : public GameObject {
public:
    Nugget(int x, int y, bool visible, bool tunnelManPickUp, std::string nugState, TunnelMan* tm, StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual ~Nugget();
    
private:
    bool tunnelManCanPickUp;
    std::string state;
    int ticks;
};

/*========== Squirt class ==========*/
class Squirt : public GameObject {
public:
    Squirt(int x, int y, TunnelMan* tm, StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual ~Squirt();
    
private:
    int distTraveled;
    std::string dir;
    
};

/*========== Sonar Kit ==========*/
class SonarKit : public GameObject {
public:
    SonarKit(int x, int y, TunnelMan* t, StudentWorld* s);
    
    virtual void doSomething();
    
    virtual ~SonarKit();

private:
    int ticks;
    
};

/*========== Water Pool ==========*/
class WaterPool : public GameObject {
public:
    WaterPool(int x, int y, TunnelMan* t, StudentWorld* s);
    
    virtual void doSomething();
    
    virtual ~WaterPool();
    
private:
    int ticks;
};

/*========== TunnelMan class ==========*/
class TunnelMan : public GameObject {
public:
    TunnelMan(StudentWorld* sw);
    
    virtual void doSomething();
    
    virtual void annoyed(int val);
        
    int getHealth();
    
    int getSquirts();
    void updateSquirts();
    
    int getNuggets();
    void updateNuggets();
    
    int getSonar();
    void updateSonar();
    
    virtual ~TunnelMan();

private:
    int hitPoints;
    int water;
    int sonar;
    int nuggets;
};


#endif // ACTOR_H_
