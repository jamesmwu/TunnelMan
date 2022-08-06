#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>

//For testing purposes
#include <iostream>
using namespace std;

class TunnelMan;
class Actor;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir)
	{
        tunnelManPtr = nullptr;
	}

    //Initialize data structures, construct new oil field, allocate / insert TunnelMan Object
	virtual int init()
	{
        //Fill rows 0 to 59 with Earth objects w exception of vertical mine shaft in middle of field
        //Rows (y)
        for(int i = 0; i < 60; i++){
            //Cols (x)
            for(int j = 0; j < 64; j++){
                //4 Squares wide at cols 30-33 and 56 squares deep are empty
                if(j >= 30 && j <= 33 && i <= 55) continue;
                
                earthObjects[i][j] = new Earth(j, 59 - i);
            }
        }
        
        //Create TunnelMan obj
        tunnelManPtr = new TunnelMan(this);

        
		return GWSTATUS_CONTINUE_GAME;
	}

	virtual int move()
	{
        setGameStatText("Scr: 000000 Lvl: 0 Lives: 3 Hlth: 100% Wtr: 5 Gld: 1 Sonar: 1 Oil Left: 2");
        
        tunnelManPtr->doSomething();
        
		decLives();
        return GWSTATUS_CONTINUE_GAME;

//		return GWSTATUS_PLAYER_DIED;
	}
    
    void tunnelManEarthOverlap(){
        int x = tunnelManPtr->getX();
        int y = tunnelManPtr->getY();
        

        
    }

	virtual void cleanUp()
	{
        //Delete Earth
        for(int i = 0; i < 60; i++){
            for(int j = 0; j < 64; j++){
                //4 Squares wide at cols 30-33 and 56 squares deep are empty
                delete earthObjects[i][j];
            }
        }
        
        delete tunnelManPtr;
	}

private:
    TunnelMan* tunnelManPtr;
    std::vector<Actor*> gameObjects;
    Earth* earthObjects[64][60];  //Rows 0 - 59 are filled with Earth objs
};

#endif // STUDENTWORLD_H_
