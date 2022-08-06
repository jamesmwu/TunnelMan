#include "Actor.h"
#include "StudentWorld.h"

/*========== Earth ==========*/
Earth::Earth(int x, int y) : GameObject(TID_EARTH, x, y, Direction::right, 0.25, 3){
    setVisible(true);
}

Earth::~Earth(){}

/*========== TunnelMan ==========*/
TunnelMan::TunnelMan() : GameObject(TID_PLAYER, 30, 60, Direction::right, 1.0, 0){
    setVisible(true);
    
    hitPoints = 10;
    water = 5;
    sonar = 1;
    nuggets = 0;
    x = 30;
    y = 60;
}

void TunnelMan::doSomething(){
    if(hitPoints <= 0) return;
    
    int ch;
//    if (getWorld()->getKey(ch) == true)
//    {
//        // user hit a key this tick!
//        switch (ch)
//        {
//            case KEY_PRESS_LEFT:
//                moveTo(x - 3, y);
//            break;
//            case KEY_PRESS_RIGHT:
//                moveTo(x + 3, y);
//            break;
//            case KEY_PRESS_SPACE:
//            ... add a Squirt in front of the player...;
//            break;
            // etc…
//        }
//    }
}

TunnelMan::~TunnelMan(){}
