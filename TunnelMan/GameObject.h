//
//  GameObject.h
//  TunnelMan
//
//  Created by James Wu on 8/4/22.
//  Copyright © 2022 CS32 Instructor. All rights reserved.
//

#ifndef GameObject_h
#define GameObject_h
#include "GraphObject.h"

class GameObject : public GraphObject {
public:
    //Default values of GraphObject constructor
    GameObject(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0)
    : GraphObject(imageID, startX, startY, dir, size, depth){
        
    }
    
    virtual void doSomething(){}
    
    virtual ~GameObject(){}
    
};


#endif /* GameObject_h */
