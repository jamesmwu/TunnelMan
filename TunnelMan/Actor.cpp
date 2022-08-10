#include "Actor.h"
#include "StudentWorld.h"

//Used (so far) for testing:
#include<iostream>
using namespace std;


/*========== GameObject (base class) ==========*/
GameObject::GameObject(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth)
: GraphObject(imageID, startX, startY, dir, size, depth){
    x = startX;
    y = startY;
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

void GameObject::doSomething(){}

void GameObject::annoyed(int val){}

GameObject::~GameObject(){
    
}

/*========== Earth ==========*/
Earth::Earth(int xLoc, int yLoc) : GameObject(TID_EARTH, xLoc, yLoc, Direction::right, 0.25, 3){
    setVisible(true);

}

Earth::~Earth(){
    setVisible(false);  //Deleting earth object, remove
}

/*========== Boulder ==========*/
Boulder::Boulder(int x, int y, StudentWorld* sw) : GameObject(TID_BOULDER, x, y, Direction::down, 1.0, 1){
    setVisible(true);
    alive = true;
    state = "stable";
    m_studentWorld = sw;
    tick = 30;
}

void Boulder::doSomething(){
    if(!alive) return;
    
    m_studentWorld->earthOverlap(getX(), getY());
    
    if(state == "stable"){
        //Check to see if there is any earth in 4 squares immediately below boulder (same Y val). If there is ANY earth, do nothing.
        if(!m_studentWorld->checkEarthUnderBoulder(getX(), getY())) state = "waiting";
        
        return;
    }
    //If NONE of the 4 squares have earth, change state to "waiting" for the next 30 ticks
    else if(state == "waiting"){
        tick--;
        if(tick == 0){
            state = "falling";
            m_studentWorld->playSound(SOUND_FALLING_ROCK);
        }
        return;
    }
    else if(state == "falling"){
        
        //Boulder is at bottom of oil field, run into earth, another boulder, or another earth object
        if(getY() == 0 || m_studentWorld->checkEarthUnderBoulder(getX(), getY()) || m_studentWorld->checkObjectUnderBoulder(getX(), getY())){
            alive = false;
            return;
        }
        
        moveTo(getX(), getY() - 1);
        updateY(-1);
    }
    //Then, change to falling state and play sound SOUND_FALLING_ROCK. It must continue to move downward one square each tick until it hits the bottom of the oil field, runs into the top of another Boulder, or runs into Earth. Then set state to dead. If, in this state, the boulder comes within a radius of 3 (inclusive) of any Protestors or TunnelMan, it does 100 pts annoyance (killing them instantly).
    
    
}

Boulder::~Boulder(){
    setVisible(false);
    m_studentWorld = nullptr;
}

/*========== Barrel ==========*/
Barrel::Barrel(int x, int y, TunnelMan* tm, StudentWorld* sw) : GameObject(TID_BARREL, x, y, Direction::right, 1.0, 2){
    alive = true;
    setVisible(false);
    m_tunnelMan = tm;
    m_studentWorld = sw;
}

void Barrel::doSomething(){
    if(!alive) return;
    
    //If not visible and within radius of TunnelMan make visible
    if(!isVisible() && distance(getX(), getY(), m_tunnelMan->getX(), m_tunnelMan->getY(), 4)){
        setVisible(true);
        return;
    }
    
    //Barrel is visible, if in range then collect
    if(distance(getX(), getY(), m_tunnelMan->getX(), m_tunnelMan->getY(), 3)){
        alive = false;
        m_studentWorld->playSound(SOUND_FOUND_OIL);
        m_studentWorld->increaseScore(1000);
        m_studentWorld->decBarrel();
    }
    
    
}

Barrel::~Barrel(){
    setVisible(false);
    m_tunnelMan = nullptr;
    m_studentWorld = nullptr;
}

/*========== TunnelMan ==========*/
TunnelMan::TunnelMan(StudentWorld* sw) : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0){
    setVisible(true);
    hitPoints = 10;
    water = 5;
    sonar = 1;
    nuggets = 0;
    
    m_studentWorld = sw;
}

void TunnelMan::doSomething(){
    if(hitPoints <= 0) return;
    
    int x = getX();
    int y = getY();
    
    bool earth = m_studentWorld->earthOverlap(x, y);
    if(earth) m_studentWorld->playSound(SOUND_DIG);

    int ch;
    if (m_studentWorld->getKey(ch) == true)
    {
        // user hit a key this tick!
        if(ch == KEY_PRESS_LEFT){
            if(x > 0 && getDirection() == Direction::left){
                moveTo(x - 1, y);
                updateX(-1);
            }
            else setDirection(Direction::left);
        }
        else if(ch == KEY_PRESS_RIGHT){
            if(x < 60 && getDirection() == Direction::right){
                moveTo(x + 1, y);
                updateX(1);
            }
            else setDirection(Direction::right);
        }
        else if(ch == KEY_PRESS_DOWN){
            if(y > 0 && getDirection() == Direction::down){
                moveTo(x, y - 1);
                updateY(-1);
            }
            else setDirection(Direction::down);
        }
        else if(ch == KEY_PRESS_UP){
            if(y < 60 && getDirection() == Direction::up){
                moveTo(x, y + 1);
                updateY(1);
            }
            else setDirection(Direction::up);
        }
        else if(ch == KEY_PRESS_ESCAPE){
            hitPoints = 0;  //Kill tunnelman
        }
    }
}

void TunnelMan::annoyed(int val){
    hitPoints -= val;
    if(hitPoints <= 0){
        m_studentWorld->playSound(SOUND_PLAYER_GIVE_UP);
    }
}

bool TunnelMan::isAlive(){
    return hitPoints <= 0 ? false : true;
}

TunnelMan::~TunnelMan(){
    setVisible(false);  //Delete tunnelman, remove
    m_studentWorld = nullptr;
}


/*========== Auxiliary Functions ==========*/
bool distance(int x, int y, int x2, int y2, int radius){
    //Cover just the anchor point of the 4x4 sprites
    int dist = sqrt(pow(x - x2, 2) + pow(y - y2, 2));
    
    return dist <= radius ? true : false;
}
