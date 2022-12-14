#include "StudentWorld.h"
#include <string>
#include <stdlib.h> //Used for rand when creating gameObjects
#include <time.h>   //Used for time in using srand to reset rand
#include <vector>
#include <cmath>    //Used to calculate distance to other gameObjects
#include <queue>    //Used for protester navigation to exit

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

/*========== StudentWorld Implementations ==========*/
StudentWorld::StudentWorld(std::string assetDir): GameWorld(assetDir){
    tunnelManPtr = nullptr;
    barrels = 0;
    protesters = 0;
    ticksSinceLastProtesterAdded = 0;
    sonarActive = false;
    //Fill rows 0 to 59 with NULL (if you don't do this, the destructor will have bad access if you close the window quickly)
    //Rows (y)
    for(int i = 0; i < 60; i++){
        //Cols (x)
        for(int j = 0; j < 64; j++){
            earthObjects[i][j] = NULL;
        }
    }
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
    protesters = min(15.0, 2 + level * 1.5);
    
    barrels = barrel;
    ticksSinceLastProtesterAdded = max(25, 200 - level);
    
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
    
    //Generate regular protester (only 1 on first tick, and always in same spot)
    gameObjects.push_back(new Protester(tunnelManPtr, this));
    protesters--;
    
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
            if((*it)->getID() == TID_SONAR) sonarActive = false;
            if(((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER)) protesters++;
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
    
    //Regular protester is added after certain amount of ticks
    if(ticksSinceLastProtesterAdded == 0 && protesters > 0){
        int level = getLevel();
        int probabilityOfHardCore = min(90, level * 10 + 30);
        int num = rand() % 100; //Gives num between 0 and 100
        if(num <= probabilityOfHardCore){
            gameObjects.push_back(new Hardcore(tunnelManPtr, this));
        }
        else gameObjects.push_back(new Protester(tunnelManPtr, this));
        
        ticksSinceLastProtesterAdded = max(25, 200 - level);
        protesters--;
    }
    else if(protesters > 0) ticksSinceLastProtesterAdded--;
    
    
    
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
            
            if((*it)->getID() == TID_BOULDER && (*it)->distance(x, y, bldrX, bldrY, 3)){
                return true;
            }
        }
    }
    else if(type == "squirt" || "protester"){
        //Check for boundaries of oil field
        if(type == "squirt" && ((direction == "left" && x < 0) || (direction == "right" && x >= 60) || (direction == "up" && y >= 60) || (direction == "down" && y < 0))){
            return true;
        }
        else if(type == "protester" && ((direction == "left" && x < 1) || (direction == "right" && x >= 60) || (direction == "up" && y >= 60) || (direction == "down" && y < 1))){
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
        if(type == "squirt"){
            if(direction == "left") x -= 2;
            else if(direction == "right") x += 2;
            else if(direction == "up") y += 2;
            else if(direction == "down") y -= 2;
        }

        //Check for boulders
        for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
            
            int objX = (*it)->getX();
            int objY = (*it)->getY();
            
            if((*it)->getID() == TID_BOULDER && (*it)->distance(x, y, objX, objY, 3)){
                return true;
            }
        }
    }
    
    return false;
}

bool StudentWorld::nearProtester(int x, int y, std::string direction, std::string parent){
    if(direction == "left") x -= 2;
    else if(direction == "right") x += 2;
    else if(direction == "up") y += 2;
    else if(direction == "down") y -= 2;
    
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        
        int objX = (*it)->getX();
        int objY = (*it)->getY();
        
        if(parent == "squirt" && ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER) && (*it)->distance(x, y, objX, objY, 3)){
            if(!(*it)->isStunned()){
                (*it)->annoyed(2, "squirt");
            }
            return true;
        }
        else if(parent == "nugget" && ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER) && (*it)->distance(x, y, objX, objY, 3)){
            (*it)->bribed();
            return true;
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
        if((*it) == bldr || ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER)) continue;
        
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
    
    playSound(SOUND_SONAR);
}

bool StudentWorld::tunnelManLineOfSight(int x, int y, Protester* prot){
        
    if(y == tunnelManPtr->getY()){
        
        //Determine what side the tunnelman is on
        bool right = tunnelManPtr->getX() > x ? true : false;
        
        if(right){
            //Check for Earth / Boulder
            //Y
            for(int i = 0; i < 4; i++){
                while(x < tunnelManPtr->getX()){
                    if(y < 60 && x < 64 && earthObjects[y][x] != nullptr) return false;
                    
                    //Boulder check
                    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
                        int bldrX = (*it)->getX();
                        int bldrY = (*it)->getY();
                        
                        if((*it)->getID() == TID_BOULDER && x == bldrX && y == bldrY){
                            return false;
                        }
                    }
                    x++;
                }
                y++;
            }
            prot->setDirection(Protester::Direction::right);
            return true;
        }
        //Else tunnelman is on the left side
        else {
            //Check for Earth / Boulder
            //Y
            for(int i = 0; i < 4; i++){
                while(x > tunnelManPtr->getX()){
                    if(y < 60 && x < 64 && earthObjects[y][x] != nullptr) return false;
                    
                    //Boulder check
                    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
                        int bldrX = (*it)->getX();
                        int bldrY = (*it)->getY();
                        
                        if((*it)->getID() == TID_BOULDER && x == bldrX && y == bldrY){
                            return false;
                        }
                    }
                    x--;
                }
                y++;
            }
            prot->setDirection(Protester::Direction::left);
            return true;
        }

    }
    else if(x == tunnelManPtr->getX()){
        
        //Determine what side the tunnelman is on
        bool up = tunnelManPtr->getY() > y ? true : false;
        
        if(up){
            //Check for Earth / Boulder
            //X
            for(int i = 0; i < 4; i++){
                while(y < tunnelManPtr->getY()){
                    if(y < 60 && x < 64 && earthObjects[y][x] != nullptr) return false;
                    
                    //Boulder check
                    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
                        int bldrX = (*it)->getX();
                        int bldrY = (*it)->getY();
                        
                        if((*it)->getID() == TID_BOULDER && x == bldrX && y == bldrY){
                            return false;
                        }
                    }
                    y++;
                }
                x++;
            }
            prot->setDirection(Protester::Direction::up);
            return true;
        }
        //Else tunnelman is on the bottom
        else {
            //Check for Earth / Boulder
            //X
            for(int i = 0; i < 4; i++){
                while(y > tunnelManPtr->getY()){
                    if(y < 60 && x < 64 && earthObjects[y][x] != nullptr)
                        return false;
                    
                    //Boulder check
                    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
                        int bldrX = (*it)->getX();
                        int bldrY = (*it)->getY();
                        
                        if((*it)->getID() == TID_BOULDER && x == bldrX && y == bldrY){
                            return false;
                        }
                    }
                    y--;
                }
                x++;
            }
            prot->setDirection(Protester::Direction::down);
            return true;
        }

    }
    
    return false;
}

void StudentWorld::protesterAnnoyed(int x, int y){
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        int objX = (*it)->getX();
        int objY = (*it)->getY();
        
        if(((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER) && (*it)->distance(objX, objY, x, y, 3)){
            (*it)->annoyed(100, "boulder");
            break;
        }
    }
    
}

bool StudentWorld::boulderExists(int x, int y, int radius){
    for(auto it = gameObjects.begin(); it != gameObjects.end(); it++){
        if((*it)->getID() == TID_BOULDER && (*it)->distance(x, y, (*it)->getX(), (*it)->getY(), radius)) return true;
    }
    return false;
}

bool StudentWorld::canMove(int x, int y, GameObject::Direction direction){

    switch(direction){
        case GameObject::left:
            return (x > 0 && !checkEarth(x - 1, y) && !boulderExists(x, y, 3));
        case GameObject::right:
            return(x < 60 && !checkEarth(x + 1, y) && !boulderExists(x + 1, y, 3));
        case GameObject::up:
            return (y < 60 && !checkEarth(x, y + 1) && !boulderExists(x, y + 1, 3));
        case GameObject::down:
            return (y > 0 && !checkEarth(x, y - 1) && !boulderExists(x, y - 1, 3));
        default:
            return false;
    }
    return false;

}

//Helps protester exit
void StudentWorld::pathing(Protester* pro){
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 64; j++){
            maze[i][j]=0;
        }
    }
    int a =pro->getX(); //x
    int b =pro->getY(); //y
    queue<Coord> BFS;
    BFS.push(Coord(60,60));
    maze[60][60]=1;
    while (!BFS.empty()) {
        Coord c = BFS.front();
        BFS.pop();
        int x = c.x;
        int y = c.y;

        //left
        if(canMove(x,y, GraphObject::left)&& maze[x-1][y]==0){
            BFS.push(Coord(x-1,y));
            maze[x-1][y] = maze[x][y]+1;
        }
        //right
        if(canMove(x,y, GraphObject::right)&& maze[x+1][y]==0){
            BFS.push(Coord(x+1,y));
            maze[x+1][y] = maze[x][y]+1;
        }
        //up
        if(canMove(x,y, GraphObject::up)&& maze[x][y+1]==0){
            BFS.push(Coord(x,y+1));
            maze[x][y+1] = maze[x][y]+1;
        }
        // down
        if(canMove(x,y, GraphObject::down)&& maze[x][y-1]==0){
            BFS.push(Coord(x,y-1));
            maze[x][y-1] = maze[x][y]+1;
        }
    }
    if(canMove(a,b, GraphObject::left)&& maze[a-1][b]<maze[a][b]){
        pro->setDirection(GameObject::left);
        pro->moveTo(a - 1, b);
        pro->updateX(-1);
    }

    if(canMove(a,b, GraphObject::right)&& maze[a+1][b]<maze[a][b]){
        pro->setDirection(GameObject::right);
        pro->moveTo(a + 1, b);
        pro->updateX(1);
    }
    if(canMove(a,b, GraphObject::up)&& maze[a][b+1]<maze[a][b]){
        pro->setDirection(GameObject::up);
        pro->moveTo(a, b + 1);
        pro->updateY(1);
    }
    if(canMove(a,b, GraphObject::down)&& maze[a][b-1]<maze[a][b]){
        pro->setDirection(GameObject::down);
        pro->moveTo(a, b - 1);
        pro->updateY(-1);
    }
    
}

GraphObject::Direction StudentWorld::hardcoreSensePlayer(Protester *pro, int M){
    //Reset maze
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 64; j++){
            maze[i][j]=0;
        }
    }
    int a = pro->getX();
    int b = pro->getY();
    queue<Coord> BFS;
    BFS.push(Coord(tunnelManPtr->getX(),tunnelManPtr->getY()));
    maze[tunnelManPtr->getX()][tunnelManPtr->getY()]=1; //Set the "endpoint" goal as the current position of the Tunnelman
    
    //Do BFS same way as in pathing to exit
    while (!BFS.empty()) {
        Coord c = BFS.front();
        BFS.pop();
        int x=c.x;
        int y=c.y;
        
        //left
        if(canMove(x,y, GraphObject::left)&& maze[x-1][y]==0){
            BFS.push(Coord(x-1,y));
            maze[x-1][y] = maze[x][y]+1;
        }
        //right
        if(canMove(x,y, GraphObject::right)&& maze[x+1][y]==0){
            BFS.push(Coord(x+1,y));
            maze[x+1][y] = maze[x][y]+1;
        }
        //up
        if(canMove(x,y, GraphObject::up)&& maze[x][y+1]==0){
            BFS.push(Coord(x,y+1));
            maze[x][y+1] = maze[x][y]+1;
        }
        // down
        if(canMove(x,y, GraphObject::down)&& maze[x][y-1]==0){
            BFS.push(Coord(x,y-1));
            maze[x][y-1] = maze[x][y]+1;
        }
    }
    
    //If the position of TM is within range of the M value, then return direction the hardcore protester should move.
    if(maze[a][b] <= M+1){
        if(canMove(a,b, GraphObject::left)&& maze[a-1][b] < maze[a][b])
            return GraphObject::left;
        else if(canMove(a,b, GraphObject::right)&& maze[a+1][b] < maze[a][b])
            return GraphObject::right;
        else if(canMove(a,b, GraphObject::up)&& maze[a][b+1] < maze[a][b])
            return GraphObject::up;
        else if(canMove(a,b, GraphObject::down)&& maze[a][b-1] < maze[a][b])
            return GraphObject::down;
    }
    
    return GraphObject::none;
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
