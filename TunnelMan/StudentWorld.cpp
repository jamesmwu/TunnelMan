#include "StudentWorld.h"
#include <string>
#include <stdlib.h> //Used for rand when creating gameObjects
#include <time.h>   //Used for time in using srand to reset rand
#include <vector>
#include <cmath>    //Used to calculate distance to other gameObjects

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

/*========== StudentWorld Implementations ==========*/
StudentWorld::StudentWorld(std::string assetDir): GameWorld(assetDir){
    tunnelManPtr = nullptr;
    barrels = 0;
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

    //Create other game objects
    int level = getLevel();
    int boulder = min(level / 2 + 2, 9);
//    int nugget = max(5 - level / 2, 2);
    int barrel = min(2 + level, 21);
    
    barrels = barrel;
    
    //Subtract 4 from the range since the anchor point of the object is the bottom left.
    int xRange = 60;
    int yRange = 56;
    
    srand((unsigned)time(NULL));
    
    //Generate boulders
    generate(boulder, xRange, yRange, "boulder");
    
    //Generate barrels
    generate(barrel, xRange, yRange, "barrel");
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move(){
    
    //Status bar
    int level = getLevel();
    int lives = getLives();
    int health = tunnelManPtr->getHealth();
    int squirts = tunnelManPtr->getSquirts();
    int gold = tunnelManPtr->getNuggets();
    int barrelsLeft = barrels;
    int sonar = tunnelManPtr->getSonar();
    int score = getScore();
    
    string res = format(level, lives, health, squirts, gold, barrelsLeft, sonar, score);
    
    setGameStatText(res);
    
    if(barrels == 0){
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    if(!tunnelManPtr->isAlive()){
        decLives();
        playSound(SOUND_PLAYER_GIVE_UP);
        return GWSTATUS_PLAYER_DIED;
    }
    
    tunnelManPtr->doSomething();
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        (*it)->doSomething();
    }
    
    return GWSTATUS_CONTINUE_GAME;

}

/*========== Helper functions ==========*/
std::string StudentWorld::format(int level, int lives, int health, int squirts, int gold, int barrelsLeft, int sonar, int score){
    
    string result;
    
    //Score
    result += "Scr: ";
    for(int i = digits(score); i < 6; i++){
        result += "0";
    }
    result += to_string(score)+ "  ";
    
    //Level
    if(digits(level) == 1){
        result += "Lvl:  " + to_string(level) + "  ";
    }
    else result += "Lvl: " + to_string(level) + "  ";
    
    //Lives
    if(digits(lives) == 1){
        result += "Lives:  " + to_string(lives) + "  ";
    }
    else result += "Lives: " + to_string(lives) + "  ";
    
    //Health
    result += "Hlth: ";
    for(int i = digits(health); i < 3; i++){
        result += " ";
    }
    result += to_string(health) + "%  ";
    
    //Water
    if(digits(squirts) == 1){
        result += "Wtr:  " + to_string(squirts) + "  ";
    }
    else result += "Wtr: " + to_string(squirts) + "  ";
    
    //Gold
    if(digits(gold) == 1){
        result += "Gld:  " + to_string(gold) + "  ";
    }
    else result += "Gld: " + to_string(gold) + "  ";
    
    //Sonar
    if(digits(sonar) == 1){
        result += "Sonar:  " + to_string(sonar) + "  ";
    }
    else result += "Sonar: " + to_string(gold) + "  ";
    
    //Oil
    if(digits(barrelsLeft) == 1){
        result += "Oil Left:  " + to_string(barrelsLeft) + "  ";
    }
    else result += "Oil Left: " + to_string(barrelsLeft) + "  ";
    
    return result;
}

int StudentWorld::digits(int num){
    int digits = 0;
    
    while(num > 0){
        num /= 10;
        digits++;
    }
    
    //Edge case
    if(digits == 0) digits = 1;
    
    return digits;
}


void StudentWorld::decBarrel(){
    barrels--;
}


void StudentWorld::generate(int amt, int xRange, int yRange, std::string type){
    //Boulder
    if(type == "boulder"){
        for(int i = 0; i < amt; i++){
            
            //Generate 2 random x and y locations for boulder
            int x = rand() % xRange;
            int y = rand() % yRange;
            bool inRange = distance(x, y);
            
            //Ensure boulder is within the oil field and not in the tunnel and not in range of other objects
            while(x < 0 || x > 60 || y < 20 || y > 56 || (x >= 25 && x <= 35 && y >= 1) || inRange){
                x = rand() % xRange;
                y = rand() % yRange;
                inRange = distance(x, y);
            }
            
            GameObject* bldr = new Boulder(x, y, this);
            gameObjects.push_back(bldr);
        }
    }
    //Barrel
    else if(type == "barrel"){
        for(int i = 0; i < amt; i++){
            
            //Generate 2 random x and y locations for barrel
            int x = rand() % xRange;
            int y = rand() % yRange;
            bool inRange = distance(x, y);
            
            //Ensure barrel is within the oil field and not in the tunnel and not in range of other objects
            while(x < 0 || x > 60 || y < 20 || y > 56 || (x >= 25 && x <= 35 && y >= 1) || inRange){
                x = rand() % xRange;
                y = rand() % yRange;
                inRange = distance(x, y);
            }
            
            GameObject* brl = new Barrel(x, y, tunnelManPtr, this);
            gameObjects.push_back(brl);
        }
    }
    
}


bool StudentWorld::earthOverlap(int x, int y){
    
    //Delete any earth objects that have same coords as a given obj
    bool ret = false;
    
    //Delete the Earth objects with the same coordinates
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            if(y >= 60) return ret; //Catch edge case where tunnelman is on top of Earth

            if(earthObjects[y][x] != nullptr){
                delete earthObjects[y][x];
                earthObjects[y][x] = nullptr;
                ret = true;
            }
            
            x++;
        }
        
        x -= 4;
        y++;
    }
    
    return ret;
            
}

//Uses distance formula to determine if a given coordinate is in the range of a gameObject
bool StudentWorld::distance(int x, int y){
    
    //Cover just the anchor point of the 4x4 sprites
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        //Calculate distance to boulder
        int tempX = (*it)->getX();
        int tempY = (*it)->getY();
        
        int dist = sqrt(pow(tempX - x, 2) + pow(tempY - y, 2));
        if(dist <= 6) return true;
    }
    
    return false;
}

//Returns whether or not there is earth under the boulder
bool StudentWorld::checkEarthUnderBoulder(int x, int y){
    
    //Y is constant, but x changes
    y--;
    
    for(int i = 0; i < 4; i++){
        if(earthObjects[y][x + i] != nullptr) return true;
    }
    
    return false;
    
}

//Returns whether there is another game object under boulder
bool StudentWorld::checkObjectUnderBoulder(int x, int y){
    y--;
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        if((*it)->getY() + 3 == y && (x - 3 <= (*it)->getX() <= x + 3)) return true;
    }
    
    return false;
}

void StudentWorld::cleanUp(){
    //Free tunnelMan
    delete tunnelManPtr;
    tunnelManPtr = nullptr;
    
    //Free gameObjects
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        delete *it;
    }
    gameObjects.clear();
    
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
