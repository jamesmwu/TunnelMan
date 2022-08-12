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
    sonarActive = false;
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
    int nugget = max(5 - level / 2, 2);
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
    
    //Generate nuggets
    generate(nugget, xRange, yRange, "nugget");
    
//    gameObjects.push_back(new SonarKit(0, 60, tunnelManPtr, this));

    
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
    
    //Remove dead actors
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        if(!(*it)->isAlive()){
            if((*it)->isSonar()) sonarActive = false;
            delete *it;
            *it = nullptr;
            it = gameObjects.erase(it);
            it--;
        }
    }
    
    if(!tunnelManPtr->isAlive()){
        decLives();
        playSound(SOUND_PLAYER_GIVE_UP);
        delete tunnelManPtr;
        tunnelManPtr = nullptr;
        return GWSTATUS_PLAYER_DIED;
    }
    
    tunnelManPtr->doSomething();
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        (*it)->doSomething();
    }
    
    //Random chance for sonar / water pool to be added (1 in G chance)
    //If something IS added, 1/5 chance for Sonar and 4/5 chance for Water
    int G = level * 25 + 300;
    int num = rand() % G;
    if(num == 1){
        num = rand() % 5;
        // 1 out of 5 chance for sonar
        if(num == 1 && !sonarActive){
            gameObjects.push_back(new SonarKit(0, 60, tunnelManPtr, this));
            sonarActive = true;
        }
        //Else, water
        else {
            generate(1, 60, 56, "water");
        }
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
    result += "Lvl: " + to_string(level) + "  ";
    
    //Lives
    result += "Lives: " + to_string(lives) + "  ";

    //Health
    result += "Hlth: " + to_string(health) + "%  ";
    
    //Water
    result += "Wtr: " + to_string(squirts) + "  ";
    
    //Gold
    result += "Gld: " + to_string(gold) + "  ";
    
    //Sonar
    result += "Sonar: " + to_string(sonar) + "  ";
    
    //Oil
    result += "Oil Left: " + to_string(barrelsLeft) + "  ";
    
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
            
            GameObject* bldr = new Boulder(x, y, this, tunnelManPtr);
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
    //Nugget
    else if(type == "nugget"){
        for(int i = 0; i < amt; i++){
            
            //Generate 2 random x and y locations for nugget
            int x = rand() % xRange;
            int y = rand() % yRange;
            bool inRange = distance(x, y);
            
            //Ensure nugget is within the oil field and not in the tunnel and not in range of other objects
            while(x < 0 || x > 60 || y < 20 || y > 56 || (x >= 25 && x <= 35 && y >= 1) || inRange){
                x = rand() % xRange;
                y = rand() % yRange;
                inRange = distance(x, y);
            }
            
            GameObject* nug = new Nugget(x, y, false, true, "permanent", tunnelManPtr, this);

            gameObjects.push_back(nug);
        }
    }
    else if(type == "water"){
        for(int i = 0; i < amt; i++){
            
            //Generate 2 random x and y locations for water
            int x = rand() % xRange;
            int y = rand() % yRange;
            bool inRange = distance(x, y);
            bool earth = checkEarth(x, y);
            
            //Ensure water is within the oil field in a valid, 4x4 area
            while(x < 0 || x > 60 || y < 20 || y > 56 || inRange || earth){
                x = rand() % xRange;
                y = rand() % yRange;
                inRange = distance(x, y);
                earth = checkEarth(x, y);
                
            }
            
            GameObject* wtr = new WaterPool(x, y, tunnelManPtr, this);
            gameObjects.push_back(wtr);
        }
    }
    
}

//Checks to see if earth is present at the given coord
bool StudentWorld::checkEarth(int x, int y){
    //Check for Earth
    for(int i = 0; i < 4; i++){
        if(y >= 60) break;
        if(x >= 64) break;
        
        for(int j = 0; j < 4; j++){
            if(earthObjects[y][x] != nullptr){
                return true;
            }
            
            x++;
        }
        x -= 4;
        y++;
    }
    
    return false;
}


//DELETES earth at the given coord
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

bool StudentWorld::nearObj(int x, int y, std::string direction, std::string type){
    if(direction == "left") x--;
    else if(direction == "right") x++;
    else if(direction == "up") y++;
    else if(direction == "down") y--;
    
    if(type == "tunnelman"){
        for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
            
            int bldrX = (*it)->getX();
            int bldrY = (*it)->getY();
            
            if((*it)->isBoulder() && (*it)->distance(x, y, bldrX, bldrY, 3)){
                return true;
            }
        }
    }
    else if(type == "squirt"){
        //Check for boundaries of oil field
        if((direction == "left" && x < 0) || (direction == "right" && x >= 60) || (direction == "up" && y >= 60) || (direction == "down" && y < 0)){
            return true;
        }
        
        
        //Check for Earth (can't use checkEarth function since this requires different y increments based on direction
        int ogY = y;
        int ogX = x;
        if(y < 60){
            for(int i = 0; i < 4; i++){
                if(y >= 60) break;
                if(x >= 64) break;
                
                for(int j = 0; j < 4; j++){
                    if(earthObjects[y][x] != nullptr){
                        return true;
                    }
                    
                    x++;
                }
                x -= 4;
                direction == "down" ? y-- : y++;
            }
        }
        //Reset x and y
        y = ogY;
        x = ogX;
        //Uncomment this if the squirt should not show up at all if it overlaps with boulders.
//        if(direction == "left") x -= 3;
//        else if(direction == "right") x += 3;
//        else if(direction == "up") y += 3;
//        else if(direction == "down") y -= 3;
        
        //Check for boulders
        for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
            
            int bldrX = (*it)->getX();
            int bldrY = (*it)->getY();
            
            if((*it)->isBoulder() && (*it)->distance(x, y, bldrX, bldrY, 3)){
                return true;
            }
        }
    }
    
    return false;
}

void StudentWorld::squirt(int x, int y, std::string dir){
    
    if(dir == "up"){
        y += 4;
    }
    else if(dir == "down"){
        y -= 4;
    }
    else if(dir == "left"){
        x -= 4;
    }
    else if(dir == "right"){
        x += 4;
    }
    
    
    GameObject* sqrt = new Squirt(x, y, tunnelManPtr, this);
    gameObjects.push_back(sqrt);
    playSound(SOUND_PLAYER_SQUIRT);
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
bool StudentWorld::checkObjectUnderBoulder(int x, int y, Boulder* bldr){
    y--;
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        if((*it) == bldr) continue;
        
        if((*it)->getY() + 3 == y && ((x - 3 <= (*it)->getX()) && ((*it)->getX() <= x + 3))){
            return true;
        }

    }
    
    return false;
}

//Add nugget into playing field
void StudentWorld::dropNugget(int x, int y){
    //Nugget is pickupable by protestors and temporary
    GameObject* insert = new Nugget(x, y, true, false, "temporary", tunnelManPtr, this);
    
    gameObjects.push_back(insert);
}

//Reveals objects within radius 12 of the TunnelMan's current distance
void StudentWorld::sonarCharge(){
    
    int x = tunnelManPtr->getX();
    int y = tunnelManPtr->getY();
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        int objX = (*it)->getX();
        int objY = (*it)->getY();
        
        if(tunnelManPtr->distance(x, y, objX, objY, 12)){
            (*it)->setVisible(true);
        }
    }
}

void StudentWorld::cleanUp(){
    //Free tunnelMan
    if(tunnelManPtr){
        delete tunnelManPtr;
        tunnelManPtr = nullptr;
    }

    //Free gameObjects
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        delete *it;
        *it = nullptr;
        it = gameObjects.erase(it);
        it--;
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
