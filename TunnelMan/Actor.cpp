#include "Actor.h"
#include "StudentWorld.h"

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
Nugget::Nugget(int x, int y, bool visible, bool tunnelManPickUp, std::string nugState, TunnelMan* tm, StudentWorld* sw) : GameObject(TID_GOLD, x, y, Direction::right, 1.0, 2, tm, sw){
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
    
    //Protestor picks up nugget
    
    //Nugget decays
    if(state == "temporary"){
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


/*========== TunnelMan ==========*/
TunnelMan::TunnelMan(StudentWorld* sw) : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0, nullptr, sw){
    setVisible(true);
    hitPoints = 10;
    water = 5;
    sonar = 1;
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
            if(x > 0 && getDirection() == Direction::left && !sw()->checkTunnelManNearBoulder(getX(), getY(), "left")){
                moveTo(x - 1, y);
                updateX(-1);
            }
            else setDirection(Direction::left);
        }
        else if(ch == KEY_PRESS_RIGHT){
            if(x < 60 && getDirection() == Direction::right && !sw()->checkTunnelManNearBoulder(getX(), getY(), "right")){
                moveTo(x + 1, y);
                updateX(1);
            }
            else setDirection(Direction::right);
        }
        else if(ch == KEY_PRESS_DOWN){
            if(y > 0 && getDirection() == Direction::down && !sw()->checkTunnelManNearBoulder(getX(), getY(), "down")){
                moveTo(x, y - 1);
                updateY(-1);
            }
            else setDirection(Direction::down);
        }
        else if(ch == KEY_PRESS_UP){
            if(y < 60 && getDirection() == Direction::up && !sw()->checkTunnelManNearBoulder(getX(), getY(), "up")){
                moveTo(x, y + 1);
                updateY(1);
            }
            else setDirection(Direction::up);
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

int TunnelMan::getNuggets(){
    return nuggets;
}

void TunnelMan::updateNuggets(){
    nuggets++;
}

int TunnelMan::getSonar(){
    return sonar;
}

TunnelMan::~TunnelMan(){
    setVisible(false);  //Delete tunnelman, remove
    dead();
}

