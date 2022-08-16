#include "Actor.h"
#include "StudentWorld.h"
#include <queue>    //Used for protester navigation

//Used (so far) for testing:
#include<iostream>
using namespace std;


/*========== GameObject (base class) ==========*/
GameObject::GameObject(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, TunnelMan* tm, StudentWorld* sw)
: GraphObject(imageID, startX, startY, dir, size, depth){
    x = startX;
    y = startY;
    alive = true;
    bldr = false;
    sonar = false;
    protester = false;
    m_tunnelMan = tm;
    m_studentWorld = sw;
}

int GameObject::getX() const {
    return x;
}

int GameObject::getY() const {
    return y;
}

void GameObject::updateX(int val){
    x += val;
}

void GameObject::updateY(int val){
    y += val;
}

bool GameObject::isAlive() const{
    return alive;
}

void GameObject::dead(){
    alive = false;
}

void GameObject::live(){
    alive = true;
}

bool GameObject::isBoulder() const{
    return bldr;
}

void GameObject::imABoulder(){
    bldr = true;
}

bool GameObject::isSonar() const {
    return sonar;
}

void GameObject::imASonar() {
    sonar = true;
}

bool GameObject::isProtester() const {
    return protester;
}

void GameObject::imAProtester() {
    protester = true;
}

TunnelMan* GameObject::tm() const{
    return m_tunnelMan;
}

StudentWorld* GameObject::sw() const{
    return m_studentWorld;
}

bool GameObject::distance(int x, int y, int x2, int y2, int radius){
    //Cover just the anchor point of the 4x4 sprites
    int dist = sqrt(pow(x - x2, 2) + pow(y - y2, 2));
    
    return dist <= radius ? true : false;
}

void GameObject::doSomething(){}

void GameObject::annoyed(int val){}

void GameObject::annoyed(int val, std::string annoyer){}

void GameObject::bribed(){}

GameObject::~GameObject(){
    m_tunnelMan = nullptr;
    m_studentWorld = nullptr;
}

/*========== Earth ==========*/
Earth::Earth(int xLoc, int yLoc) : GameObject(TID_EARTH, xLoc, yLoc, Direction::right, 0.25, 3){
    setVisible(true);
}

Earth::~Earth(){
    setVisible(false);  //Deleting earth object, remove
    dead();
}

/*========== Boulder ==========*/
Boulder::Boulder(int x, int y, StudentWorld* sw, TunnelMan* tm) : GameObject(TID_BOULDER, x, y, Direction::down, 1.0, 1, tm, sw){
    setVisible(true);
    state = "stable";
    tick = 30;
    imABoulder();
}

void Boulder::doSomething(){
    if(!isAlive()) return;
    
    sw()->earthOverlap(getX(), getY());
    
    if(state == "stable"){
        //Check to see if there is any earth in 4 squares immediately below boulder (same Y val). If there is ANY earth, do nothing.
        if(!sw()->checkEarthUnderBoulder(getX(), getY())) state = "waiting";
        
        return;
    }
    //If NONE of the 4 squares have earth, change state to "waiting" for the next 30 ticks
    else if(state == "waiting"){
        tick--;
        if(tick == 0){
            state = "falling";
            sw()->playSound(SOUND_FALLING_ROCK);
        }
        return;
    }
    else if(state == "falling"){
        
        //Boulder is at bottom of oil field, run into earth, another boulder, or another earth object
        if(getY() == 0 || sw()->checkEarthUnderBoulder(getX(), getY()) || sw()->checkObjectUnderBoulder(getX(), getY(), this)){
            dead();
            setVisible(false);
            return;
        }
        
        moveTo(getX(), getY() - 1);
        updateY(-1);
        
        //If boulder hits tunnel man, deals 100 points of annoyance
        if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 3))
            tm()->annoyed(100);
        
        //If boulder hits protesters, deals 100 points of annoyance
        sw()->protesterAnnoyed(getX(), getY());
    }
    
    
}

Boulder::~Boulder(){
    setVisible(false);
    dead();
}

/*========== Barrel ==========*/
Barrel::Barrel(int x, int y, TunnelMan* tm, StudentWorld* sw) : GameObject(TID_BARREL, x, y, Direction::right, 1.0, 2, tm, sw){
    setVisible(false);
}

void Barrel::doSomething(){
    if(!isAlive()) return;
    
    //If not visible and within radius of TunnelMan make visible
    if(!isVisible() && distance(getX(), getY(), tm()->getX(), tm()->getY(), 4)){
        setVisible(true);
        return;
    }
    
    //Barrel is visible, if in range then collect
    if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 3)){
        dead();
        setVisible(false);
        sw()->playSound(SOUND_FOUND_OIL);
        sw()->increaseScore(1000);
        sw()->decBarrel();
    }
    
    
}

Barrel::~Barrel(){
    setVisible(false);
    dead();
}

/*========== Nugget ==========*/
//State can either be "temporary" or "permanent"
Nugget::Nugget(int x, int y, bool visible, bool tunnelManPickUp, string nugState, TunnelMan* tm, StudentWorld* sw) : GameObject(TID_GOLD, x, y, Direction::right, 1.0, 2, tm, sw){
    setVisible(visible);
    tunnelManCanPickUp = tunnelManPickUp;
    state = nugState;
    ticks = 100;
}

void Nugget::doSomething(){
    if(!isAlive()) return;
    
    //If TunnelMan is within range, display
    if(!isVisible() && distance(getX(), getY(), tm()->getX(), tm()->getY(), 4)){
        setVisible(true);
        return;
    }
    
    //TunnelMan picks up nugget
    if(tunnelManCanPickUp && distance(getX(), getY(), tm()->getX(), tm()->getY(), 3)){
        dead();
        setVisible(false);
        sw()->playSound(SOUND_GOT_GOODIE);
        sw()->increaseScore(10);
        tm()->updateNuggets();
    }
    
    //Nuggets is dropped
    if(state == "temporary"){
        //Protestor picks up nugget
        if(!tunnelManCanPickUp && sw()->nearProtester(getX(), getY(), "NA", "nugget")){
            dead();
            setVisible(false);
            sw()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            sw()->increaseScore(25);
        }
        
        //Nugget decays
        if(ticks <= 0){
            dead();
            setVisible(false);
        }
        else
        ticks--;
    }
}

Nugget::~Nugget(){
    setVisible(false);
    dead();
}

/*========== Squirt ==========*/
Squirt::Squirt(int x, int y, TunnelMan* tm, StudentWorld* sw) : GameObject(TID_WATER_SPURT, x, y, tm->getDirection(), 1.0, 1, tm, sw){
    setVisible(true);
    distTraveled = 0;
    if(tm->getDirection() == Direction::up){
        dir = "up";
    }
    else if(tm->getDirection() == Direction::down){
        dir = "down";
    }
    else if(tm->getDirection() == Direction::left){
        dir = "left";
    }
    else if(tm->getDirection() == Direction::right){
        dir = "right";
    }
}

void Squirt::doSomething(){
    //If in radius of protestor
    
    if(distTraveled == 4){
        dead();
    }
    
    
    if(sw()->nearObj(getX(), getY(), dir, "squirt")){
        dead();
    }
    //Annoy protesters
    else if(sw()->nearProtester(getX(), getY(), dir, "squirt")){
        dead();
    }
    else{
        distTraveled++;
        if(dir == "up"){
            moveTo(getX(), getY() + 1);
            updateY(1);
        }
        else if(dir == "down"){
            moveTo(getX(), getY() - 1);
            updateY(-1);
        }
        else if(dir == "left"){
            moveTo(getX() - 1, getY());
            updateX(-1);
        }
        else{
            moveTo(getX() + 1, getY());
            updateX(1);
        }
    }
    
    
}

Squirt::~Squirt(){
    setVisible(false);
};

/*========== Sonar Kit ==========*/
SonarKit::SonarKit(int x, int y, TunnelMan* t, StudentWorld* s) : GameObject(TID_SONAR, x, y, Direction::right, 1.0, 2, t, s){
    setVisible(true);
    int level = sw()->getLevel();
    ticks = max(100, (300 - 10 * level));
    imASonar();
}

void SonarKit::doSomething(){
    if(!isAlive()) return;
    
    if(ticks <= 0){
        dead();
        return;
    }
    
    if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 3)){
        dead();
        sw()->playSound(SOUND_GOT_GOODIE);
        tm()->updateSonar();
        sw()->increaseScore(75);
    }
    
    ticks--;
    
}

SonarKit::~SonarKit(){
    setVisible(false);
}

/*========== Water Pool ==========*/
WaterPool::WaterPool(int x, int y, TunnelMan* t, StudentWorld* s) : GameObject(TID_WATER_POOL, x, y, Direction::right, 1.0, 2, t, s){
    setVisible(true);
    int level = sw()->getLevel();
    ticks = max(100, 300 - 10 * level);
}

void WaterPool::doSomething(){
    if(!isAlive()) return;
    
    if(ticks <= 0){
        dead();
        return;
    }
    
    if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 3)){
        dead();
        sw()->playSound(SOUND_GOT_GOODIE);
        tm()->updateSquirts();
        sw()->increaseScore(100);
    }
    
    ticks--;
}

WaterPool::~WaterPool(){
    setVisible(false);
}


/*========== Protester ==========*/
Protester::Protester(TunnelMan* t, StudentWorld* s) : GameObject(TID_PROTESTER, 60, 60, Direction::left, 1.0, 0, t, s){
    hitPoints = 5;
    leaveTheOilFieldState = false;
    moved = 8 + rand()%61;
    setVisible(true);
    
    int level = sw()->getLevel();
    ticksBetween = max(0, 5 - level / 4);
    ticks = 0;
    shoutCooldown = 0;
    perpTurnCooldown = 0;
    
//    for(int i = 0; i < 60; i++){
//        //Cols (x)
//        for(int j = 0; j < 64; j++){
//            earthSnapshot[i][j] = NULL;
//        }
//    }
    
    imAProtester();
}

void Protester::doSomething(){
    if(!isAlive()) return;
    
    if(ticks >= 0){
        ticks--;
        return;
    }
    else ticks = ticksBetween;
    
    //Track shout cooldown
    if(shoutCooldown != 0) shoutCooldown--;
    
    //Track perp cooldown
    if(perpTurnCooldown != 0) perpTurnCooldown--;
    
    //Leaves the oil field
    if(leaveTheOilFieldState){
        if(getX() == 60 && getY() == 60){
            dead();
            return;
        }
        
        //Navigate protester towards exit
//        pathing(earthSnapshot, getX(), getY());
        
        return;
    }
    
    string dir;
    if(getDirection() == Direction::up){
        dir = "up";
    }
    else if(getDirection() == Direction::down){
        dir = "down";
    }
    else if(getDirection() == Direction::left){
        dir = "left";
    }
    else if(getDirection() == Direction::right){
        dir = "right";
    }
    
    //Check if within distance of 4 units to TunnelMan (account for distance protester facing as well)
    if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 4)){
        if(shoutCooldown == 0){
            sw()->playSound(SOUND_PROTESTER_YELL);
            shoutCooldown = 15;
            tm()->annoyed(2);
        }
        return;
    }
    
    //Within line of sight to tunnelman
    if(sw()->tunnelManLineOfSight(getX(), getY(), dir, this)){
        
        cout << moved << endl;
        
        if(!sw()->nearObj(getX(), getY(), dir, "protester")){
            int x = getX();
            int y = getY();
            
            if(getDirection() == Direction::up){
                moveTo(x, y + 1);
                updateY(1);
            }
            else if(getDirection() == Direction::down){
                moveTo(x, y - 1);
                updateY(-1);
            }
            else if(getDirection() == Direction:: left){
                moveTo(x - 1, y);
                updateX(-1);
            }
            else{
                moveTo(x + 1, y);
                updateX(1);
            }
            
            moved--;
            return;
        }
        //Protester is blocked for some reason
        else {
            moved = 0;
        }
    }
    
    //See if Protester movement needs to switch directions
    if(moved <= 0){
        string dir = "";
        int newDir = rand()%4; //0 to 3
        //Change to random new direction
        switch (newDir) {
            case 0:
                setDirection(Direction::left);
                dir = "left";
                break;
            
            case 1:
                setDirection(Direction::right);
                dir = "right";
                break;
                
            case 2:
                setDirection(Direction::up);
                dir = "up";
                break;
                
            case 3:
                setDirection(Direction::down);
                dir = "down";
                break;
                
            default:
                break;
        }
        
        //Keep cycling for a new direction
        while(sw()->nearObj(getX(), getY(), dir, "protester")){
            newDir = rand()%4; //0 to 3
            dir = "";
            //Change to random new direction
            switch (newDir) {
                case 0:
                    setDirection(Direction::left);
                    dir = "left";
                    break;
                
                case 1:
                    setDirection(Direction::right);
                    dir = "right";
                    break;
                    
                case 2:
                    setDirection(Direction::up);
                    dir = "up";
                    break;
                    
                case 3:
                    setDirection(Direction::down);
                    dir = "down";
                    break;
                    
                default:
                    break;
            }
        }
        
        moved = 8 + rand()%61;
    }
    
    //Check if protester can move in a perpendicular direction and hasn't made a perpendicular turn in last 200 non-rest ticks
//    cout << perpTurnCooldown << endl;
    if(perpTurnCooldown == 0 && checkPerpendicular()){
        moved = 8 + rand()%61;
        perpTurnCooldown = 200;
    }
    
    if(getDirection() == Direction::up){
        dir = "up";
    }
    else if(getDirection() == Direction::down){
        dir = "down";
    }
    else if(getDirection() == Direction::left){
        dir = "left";
    }
    else if(getDirection() == Direction::right){
        dir = "right";
    }
    
    //Attempt to take one step in its current facing direction
    if(!sw()->nearObj(getX(), getY(), dir, "protester")){
        int x = getX();
        int y = getY();
        
        if(getDirection() == Direction::up){
            moveTo(x, y + 1);
            updateY(1);
        }
        else if(getDirection() == Direction::down){
            moveTo(x, y - 1);
            updateY(-1);
        }
        else if(getDirection() == Direction:: left){
            moveTo(x - 1, y);
            updateX(-1);
        }
        else{
            moveTo(x + 1, y);
            updateX(1);
        }
    }
    //Protester is blocked for some reason
    else {
        moved = 0;
        return;
    }
    
    moved--;
    
    //Annoyed implementation can be found in objects that annoy the Protester (boulder + squirt)
    //Bribing implementation can be found in nugget
    
}

void Protester::bribed(){
    leaveTheOilFieldState = true;
    ticks = 0;
}

void Protester::annoyed(int val, string annoyer){
    if(leaveTheOilFieldState) return;   //This prevents the sound from being played repeatedly
    
    if(annoyer == "boulder"){
        sw()->increaseScore(500);
    }
    else if(annoyer == "squirt"){
        sw()->increaseScore(100);
    }
    
    hitPoints -= val;
    
    if(hitPoints <= 0){
        leaveTheOilFieldState = true;
        sw()->getEarthArray(earthSnapshot);
        ticks = 0;
        sw()->playSound(SOUND_PROTESTER_GIVE_UP);

    }
    else {
        //Resting ticks
        int level = sw()->getLevel();
        ticks = max(50, 100 - level * 10);
        sw()->playSound(SOUND_PROTESTER_ANNOYED);
    }
}

//Helper function to navigate protester to exit when they die
void Protester::pathing(std::string maze[60][64], int x, int y){
    
    BFS.push(maze[y][x]);
    
//    if(BFS.front() == "E") break;    //Done!
//    else
    BFS.front() = "v";
    
    BFS.pop();
    
    //North
    if((y + 1 >= 0 && maze[y + 1][x] == ".") || (y >= 60)){
        BFS.push(maze[y + 1][x]);
        moveTo(getX(), getY() + 1);
        updateY(1);
    }
    //West
    if(x - 1 >= 0 && maze[y][x - 1] == "."){
        BFS.push(maze[y][x - 1]);
        moveTo(getX() - 1, getY());
        updateX(-1);
    }
    
    //South
    if(y - 1 < 60 && maze[y - 1][x] == "."){
        BFS.push(maze[y - 1][x]);
        moveTo(getX(), getY() - 1);
        updateY(-1);

    }
    
    //East
    if((x + 1 < 64 && maze[y][x + 1] == ".") || y >= 60){
        BFS.push(maze[y][x + 1]);
        moveTo(getX(), getX() + 1);
        updateX(1);

    }
        
}

Protester::~Protester(){
    setVisible(false);
}

//Returns whether the protester can move in a direction perpendicular to it, and sets direction to that if so
bool Protester::checkPerpendicular(){

    //Set direction to perpendicular
    if(getDirection() == Direction::up || getDirection() == Direction::down){
        if(!sw()->nearObj(getX(), getY(), "right", "protester")){
            setDirection(Direction::right);
            return true;
        }
        else if(!sw()->nearObj(getX(), getY(), "left", "protester")){
            setDirection(Direction::left);
            return true;
        }
    }
    else if(getDirection() == Direction::left || getDirection() == Direction::right){
        if(!sw()->nearObj(getX(), getY(), "up", "protester")){
            setDirection(Direction::up);
            return true;
        }
        else if(!sw()->nearObj(getX(), getY(), "down", "protester")){
            setDirection(Direction::down);
            return true;
        }
    }
    
    return false;

}


/*========== TunnelMan ==========*/
TunnelMan::TunnelMan(StudentWorld* sw) : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0, nullptr, sw){
    setVisible(true);
    hitPoints = 10;
//    water = 5;
    water = 10000;
    sonar = 1;
//    sonar = 10000;
    nuggets = 0;
}

void TunnelMan::doSomething(){
    if(hitPoints <= 0) return;
    
    int x = getX();
    int y = getY();
    
    bool earth = sw()->earthOverlap(x, y);
    if(earth) sw()->playSound(SOUND_DIG);

    int ch;
    if (sw()->getKey(ch) == true)
    {
        // user hit a key this tick!
        if(ch == KEY_PRESS_LEFT){
            if(x > 0 && getDirection() == Direction::left && !sw()->nearObj(getX(), getY(), "left", "tunnelman")){
                moveTo(x - 1, y);
                updateX(-1);
            }
            else setDirection(Direction::left);
        }
        else if(ch == KEY_PRESS_RIGHT){
            if(x < 60 && getDirection() == Direction::right && !sw()->nearObj(getX(), getY(), "right", "tunnelman")){
                moveTo(x + 1, y);
                updateX(1);
            }
            else setDirection(Direction::right);
        }
        else if(ch == KEY_PRESS_DOWN){
            if(y > 0 && getDirection() == Direction::down && !sw()->nearObj(getX(), getY(), "down", "tunnelman")){
                moveTo(x, y - 1);
                updateY(-1);
            }
            else setDirection(Direction::down);
        }
        else if(ch == KEY_PRESS_UP){
            if(y < 60 && getDirection() == Direction::up && !sw()->nearObj(getX(), getY(), "up", "tunnelman")){
                moveTo(x, y + 1);
                updateY(1);
            }
            else setDirection(Direction::up);
        }
        else if(ch == KEY_PRESS_TAB && nuggets >= 1){
            //Drop nugget
            nuggets--;
            sw()->dropNugget(getX(), getY());
        }
        else if(ch == KEY_PRESS_SPACE && water > 0){
            sw()->playSound(SOUND_PLAYER_SQUIRT);

            string dir;
            if(getDirection() == Direction::up){
                dir = "up";
            }
            else if(getDirection() == Direction::down){
                dir = "down";
            }
            else if(getDirection() == Direction::left){
                dir = "left";
            }
            else if(getDirection() == Direction::right){
                dir = "right";
            }
            water--;
            if(!sw()->nearObj(getX(), getY(), dir, "squirt"))
                sw()->squirt(getX(), getY(), dir);
        }
        else if((ch == 'z' || ch == 'Z' )&& sonar > 0){
            sonar--;
            sw()->sonarCharge();
        }
        else if(ch == KEY_PRESS_ESCAPE){
            hitPoints = 0;  //Kill tunnelman
            dead();
        }
    }
}

void TunnelMan::annoyed(int val){
    hitPoints -= val;
    if(hitPoints <= 0){
        sw()->playSound(SOUND_PLAYER_GIVE_UP);
        dead();
        setVisible(false);
    }
}

//Returns percentage
int TunnelMan::getHealth(){
    return hitPoints * 10;
}

int TunnelMan::getSquirts(){
    return water;
}

void TunnelMan::updateSquirts(){
    water += 5;
}

int TunnelMan::getNuggets(){
    return nuggets;
}

void TunnelMan::updateNuggets(){
    nuggets++;
}

int TunnelMan::getSonar(){
    return sonar;
}

void TunnelMan::updateSonar(){
    sonar++;
}

TunnelMan::~TunnelMan(){
    setVisible(false);  //Delete tunnelman, remove
    dead();
}

