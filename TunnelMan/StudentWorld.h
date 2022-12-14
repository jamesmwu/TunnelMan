#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>


class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);

    virtual int init();

    virtual int move();

    bool earthOverlap(int x, int y);
    
    bool checkEarthUnderBoulder(int x, int y);
    
    bool checkObjectUnderBoulder(int x, int y, Boulder* bldr);
    
    bool nearObj(int x, int y, std::string direction, std::string type);
    
    bool nearProtester(int x, int y, std::string direction, std::string parent);
    
    bool distance(int x, int y);
    
    void generate(int amt, int xRange, int yRange, std::string type);
    
    void decBarrel();
    
    std::string format(int level, int lives, int health, int squirts, int gold, int barrelsLeft, int sonar, int score);

    int digits(int num);
    
    void dropNugget(int x, int y);
    
    void squirt(int x, int y, std::string dir);
    
    bool tunnelManLineOfSight(int x, int y, Protester* prot);
    
    void sonarCharge();
    
    bool checkEarth(int x, int y);
    
    void protesterAnnoyed(int x, int y);
        
    void pathing(Protester* pr);
    
    bool canMove(int x, int y, GameObject::Direction direction);
        
    bool boulderExists(int x, int y, int radius);
    
    GraphObject::Direction hardcoreSensePlayer(Protester* pro, int M);
    
    virtual void cleanUp();
    
    ~StudentWorld();

private:
    TunnelMan* tunnelManPtr;
    std::vector<GameObject*> gameObjects;
    Earth* earthObjects[60][64];  //Rows 0 - 59 are filled with Earth objs
    int barrels;
    bool sonarActive;
    int ticksSinceLastProtesterAdded;
    int protesters;
    int maze[64][64];
    
    struct Coord {
        int x;
        int y;
        Coord(int a, int b) : x(a), y(b) {}
    };
    
};

#endif // STUDENTWORLD_H_
