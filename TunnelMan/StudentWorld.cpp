#include "StudentWorld.h"
#include <string>
#include <vector>

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

/*========== StudentWorld Implementations ==========*/
StudentWorld::StudentWorld(std::string assetDir): GameWorld(assetDir){
    tunnelManPtr = nullptr;
}

//Initialize data structures, construct new oil field, allocate / insert TunnelMan Object
int StudentWorld::init(){
    //Fill rows 0 to 59 with Earth objects w exception of vertical mine shaft in middle of field
    //Rows (y)
    for(int i = 0; i < 60; i++){
        //Cols (x)
        for(int j = 0; j < 64; j++){
            //4 Squares wide at cols 30-33 and 56 squares deep are empty
            if(j >= 30 && j <= 33 && i >= 4){
                earthObjects[i][j] = nullptr;
                continue;
            }
            
            earthObjects[i][j] = new Earth(j, i);
        }
    }
    
    //Create TunnelMan obj
    tunnelManPtr = new TunnelMan(this);

    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move(){
    setGameStatText("Scr: 000000 Lvl: 0 Lives: 3 Hlth: 100% Wtr: 5 Gld: 1 Sonar: 1 Oil Left: 2");
    
    tunnelManPtr->doSomething();
    
    decLives();
    return GWSTATUS_CONTINUE_GAME;

//        return GWSTATUS_PLAYER_DIED;
}

void StudentWorld::tunnelManEarthOverlap(){
    
    //Delete any earth objects that have same coords as where the tunnelMan is
    int x = tunnelManPtr->getX();
    int y = tunnelManPtr->getY();
    
    
    //Delete the Earth objects with the coordinates where the tunnelMan is
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            if(y >= 60) return; //Catch edge case where tunnelman is on top of Earth

            if(earthObjects[y][x] != nullptr){
                delete earthObjects[y][x];
                earthObjects[y][x] = nullptr;
            }
            
            x++;
        }
        
        x -= 4;
        y++;
    }
            
}

void StudentWorld::cleanUp(){
    //Free tunnelMan
    delete tunnelManPtr;
    tunnelManPtr = nullptr;
    
    //Free gameObjects
    for(int i = 0; i < gameObjects.size(); i++){
        GameObject* ptr = gameObjects[i];
        delete ptr;
        ptr = nullptr;
        
        delete gameObjects[i];
    }
    
    
    //Free earthObjects
    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 64; j++){
            delete earthObjects[i][j];
        }
    }
}

StudentWorld::~StudentWorld(){
    cleanUp();
}

//HELPER: Print earth array for debug testing
//void print(){
//    cout << endl;
//    //Rows (y)
//    for(int i = 60; i >= 0; i--){
//        //Cols (x)
//        for(int j = 0; j < 64; j++){
//            if(earthObjects[i][j] == nullptr) cout << "*";
//            else cout << ".";
//        }
//        cout << endl;
//    }
//
//    cout << endl;
//}
