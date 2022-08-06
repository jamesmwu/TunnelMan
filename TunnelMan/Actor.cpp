#include "Actor.h"
#include "StudentWorld.h"

//Used (so far) for testing:
#include<iostream>
using namespace std;

/*========== Earth ==========*/
Earth::Earth(int xLoc, int yLoc) : GameObject(TID_EARTH, xLoc, yLoc, Direction::right, 0.25, 3){
    setVisible(true);
    x = xLoc;
    y = yLoc;
}

int Earth::getX() const {
    return x;
}

int Earth::getY() const {
    return y;
}

Earth::~Earth(){
    setVisible(false);  //Deleting earth object, remove
}

/*========== TunnelMan ==========*/
TunnelMan::TunnelMan(StudentWorld* sw) : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0){
    setVisible(true);
    
    hitPoints = 10;
    water = 5;
    sonar = 1;
    nuggets = 0;
    x = 30;
    y = 60;
    m_studentWorld = sw;
}

void TunnelMan::doSomething(){
    if(hitPoints <= 0) return;
    
    m_studentWorld->tunnelManEarthOverlap();

    int ch;
    if (m_studentWorld->getKey(ch) == true)
    {
        // user hit a key this tick!
        if(ch == KEY_PRESS_LEFT){
            if(x > 0 && getDirection() == Direction::left){
                moveTo(x - 1, y);
                x -= 1;
            }
            else setDirection(Direction::left);
        }
        else if(ch == KEY_PRESS_RIGHT){
            if(x < 60 && getDirection() == Direction::right){
                moveTo(x + 1, y);
                x += 1;
            }
            else setDirection(Direction::right);
        }
        else if(ch == KEY_PRESS_DOWN){
            if(y > 0 && getDirection() == Direction::down){
                moveTo(x, y - 1);
                y -= 1;
            }
            else setDirection(Direction::down);
        }
        else if(ch == KEY_PRESS_UP){
            if(y < 60 && getDirection() == Direction::up){
                moveTo(x, y + 1);
                y += 1;
            }
            else setDirection(Direction::up);
        }
        else if(ch == KEY_PRESS_ESCAPE){
            hitPoints = 0;  //Kill tunnelman
        }
    }
}

int TunnelMan::getX() const{
    return x;
}

int TunnelMan::getY() const{
    return y;
}

TunnelMan::~TunnelMan(){
    setVisible(false);  //Delete tunnelman, remove
}
