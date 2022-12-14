#include "Actor.h"
#include "StudentWorld.h"
#include <queue>    //Used for protester navigation
#include <vector>

//Used (so far) for testing:
#include<iostream>
using namespace std;


/*========== GameObject (base class) ==========*/
GameObject::GameObject(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, TunnelMan* tm, StudentWorld* sw)
: GraphObject(imageID, startX, startY, dir, size, depth){
    x = startX;
    y = startY;
    alive = true;
    
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

bool GameObject::isStunned(){return false;}

GameObject::~GameObject(){
    m_tunnelMan = nullptr;
    m_studentWorld = nullptr;
    setVisible(false);
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
    dead();
};

/*========== Sonar Kit ==========*/
SonarKit::SonarKit(int x, int y, TunnelMan* t, StudentWorld* s) : GameObject(TID_SONAR, x, y, Direction::right, 1.0, 2, t, s){
    setVisible(true);
    int level = sw()->getLevel();
    ticks = max(100, (300 - 10 * level));
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
    dead();
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
    dead();
}


/*========== Protester Base Class ==========*/
Protester::Protester(TunnelMan* t, StudentWorld* s, int hp, const int ID) : GameObject(ID, 60, 60, Direction::left, 1.0, 0, t, s){
    hitPoints = hp;
    leaveTheOilFieldState = false;
    
    int level = sw()->getLevel();
    
    ticks = 0;
    shoutCooldown = 0;
    perpTurnCooldown = 0;
    
    ticksToWait = max(0, 5 - level / 4);
    numSquaresMoved = 8 + rand()%61;
    
    stunned = 0;
    
    setVisible(true);
}

void Protester::doSomething(){
    if(!isAlive()) return;
    
    //Determine if stunned
    if(stunned > 0){
        stunned--;
        return;
    }
    
    //Determine if in rest state
    if(ticks >= 0){
        ticks--;
        return;
    }
    else ticks = ticksToWait;
    
    //Determine if in leave-the-oil-field state
    if(leaveTheOilFieldState){
        if(getX() == 60 && getY() == 60){
            dead();
            return;
        }
        
        //Navigate protester towards exit
        sw()->pathing(this);
        
        return;
    }
    
    //Track shout cooldown
    if(shoutCooldown != 0) shoutCooldown--;
    //Track perpturn cooldown
    if(perpTurnCooldown != 0) perpTurnCooldown--;
    
    //Check to see if within shouting distance of TunnelMan
    if(distance(getX(), getY(), tm()->getX(), tm()->getY(), 4)){
        if(shoutCooldown == 0){
            sw()->playSound(SOUND_PROTESTER_YELL);
            shoutCooldown = 15;
            tm()->annoyed(2);
        }
        return;
    }
    
    //Hardcore: Check for pathing by doing a BFS that will find TunnelMan
    if(getID() == TID_HARD_CORE_PROTESTER){
        int M = 16 + (int)(sw()->getLevel()) * 2;
        int x = getX();
        int y = getY();
        
        //Determine if hardcore protester is within M legal moves to TunnelMan
        Direction newdir = sw()->hardcoreSensePlayer(this, M);
        
        //Block does nothing if newdir is none, meaning the HC Protester does not detect Tunnelman.
        if(newdir == Direction::up){
            setDirection(Direction::up);
            moveTo(x, y + 1);
            updateY(1);
            return;
        }
        else if(newdir == Direction::down){
            setDirection(Direction::down);
            moveTo(x, y - 1);
            updateY(-1);
            return;
        }
        else if(newdir == Direction::left){
            setDirection(Direction::left);
            moveTo(x - 1, y);
            updateX(-1);
            return;
        }
        else if(newdir == Direction::right){
            setDirection(Direction::right);
            moveTo(x + 1, y);
            updateX(1);
            return;
        }
    }
    
    //Used for auxiliary functions
    string dir;
    switch (getDirection()) {
        case Direction::up:
            dir = "up";
            break;
        case Direction::down:
            dir = "down";
            break;
        case Direction::left:
            dir = "left";
            break;
        case Direction::right:
            dir = "right";
            break;
            
        default:
            break;
    }
    
    //Determine if in line of sight with Tunnelman
    if(sw()->tunnelManLineOfSight(getX(), getY(), this)){
                
        if(!sw()->nearObj(getX(), getY(), dir, "protester")){
            int x = getX();
            int y = getY();
            
            switch (getDirection()) {
                case Direction::up:
                    moveTo(x, y + 1);
                    updateY(1);
                    break;
                case Direction::down:
                    moveTo(x, y - 1);
                    updateY(-1);
                    break;
                case Direction::left:
                    moveTo(x - 1, y);
                    updateX(-1);
                    break;
                case Direction::right:
                    moveTo(x + 1, y);
                    updateX(1);
                    break;
                    
                default:
                    break;
            }
            
            numSquaresMoved = 0;
            return;
        }
        //Protester is blocked for some reason
        else {
            numSquaresMoved = 0;
        }
    }
    
    //Protester can't see TunnelMan. Pick a new direction to move.
    if(numSquaresMoved <= 0){
        //Cycle for a new direction until valid one is found
        do {
            //Change to random new direction
            switch (rand()%4) {
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
        } while(sw()->nearObj(getX(), getY(), dir, "protester"));
        numSquaresMoved = 8 + rand()%61;    //Reset squares to move
    }
    //Otherwise if steps still remain, check if a perpendicular turn can be made
    else if(perpTurnCooldown == 0 && checkPerpendicular()){
        numSquaresMoved = 8 + rand()%61;
        perpTurnCooldown = 200;
    }
    
    switch (getDirection()) {
        case Direction::up:
            dir = "up";
            break;
        case Direction::down:
            dir = "down";
            break;
        case Direction::left:
            dir = "left";
            break;
        case Direction::right:
            dir = "right";
            break;
            
        default:
            break;
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
        numSquaresMoved = 0;
        return;
    }
    
    numSquaresMoved--;
    
}

    
void Protester::bribed(){
    sw()->playSound(SOUND_PROTESTER_FOUND_GOLD);

    if(getID() == TID_PROTESTER){
        leaveTheOilFieldState = true;
        ticks = 0;
        sw()->increaseScore(25);
    }
    //Hardcore Protester
    else {
        sw()->increaseScore(50);
        int level = sw()->getLevel();
        stunned = max(50, 100 - level * 10);
        ticks = 0;
    }
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
        ticks = 0;
        sw()->playSound(SOUND_PROTESTER_GIVE_UP);

    }
    else {
        //Resting ticks
        int level = sw()->getLevel();
        stunned = max(50, 100 - level * 10);
        ticks = 0;
        sw()->playSound(SOUND_PROTESTER_ANNOYED);
    }
}

//Returns whether protester is stunned
bool Protester::isStunned(){
    return stunned > 0 ? true : false;
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

Protester::~Protester(){
    setVisible(false);
    dead();
}

/*========== Hardcore ==========*/
Hardcore::Hardcore(TunnelMan* t, StudentWorld* s) : Protester(t, s, 20, TID_HARD_CORE_PROTESTER){
    //This pretty much is just called to give the ID of hard core protester.
}

Hardcore::~Hardcore(){
    setVisible(false);
    dead();
}


/*========== TunnelMan ==========*/
TunnelMan::TunnelMan(StudentWorld* sw) : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0, nullptr, sw){
    setVisible(true);
    hitPoints = 10;
    water = 5;
//    sonar = 1;
    sonar = 1000;
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
        else if(ch == KEY_PRESS_TAB && nuggets > 0){
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

