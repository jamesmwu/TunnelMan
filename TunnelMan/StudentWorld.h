#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>

class TunnelMan;
class Actor;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);

    virtual int init();

    virtual int move();

    void earthOverlap(int x, int y);
    
    bool checkEarthUnderBoulder(int x, int y);
    
    bool checkObjectUnderBoulder(int x, int y);
    
    bool distance(int x, int y);

    virtual void cleanUp();
    
    ~StudentWorld();

private:
    TunnelMan* tunnelManPtr;
    std::vector<GameObject*> gameObjects;
    Earth* earthObjects[60][64];  //Rows 0 - 59 are filled with Earth objs
};

#endif // STUDENTWORLD_H_
