#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"

#include <string>
#include <sstream>

const int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH / 2;
const int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH / 2;
const int NEW_BORDER_Y = VIEW_HEIGHT - SPRITE_HEIGHT;
const int LEFT_WHITE_LANE = LEFT_EDGE + ROAD_WIDTH / 3;
const int RIGHT_WHITE_LANE = RIGHT_EDGE - ROAD_WIDTH / 3;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    virtual ~StudentWorld();
    virtual GhostRacer* getRacerPTR() const { return theGhostRacer; }
    void setHitHumanPed() { hitHumanPed = true; }
    void decSoulsToSave() { soulsToSave--; }
    bool doSprayEffects(Actor* spray);
    bool doesOverlap(Actor* one, Actor* two);
    void checkCollision(Actor* cab, bool& front, bool& back);
    void addActor(Actor* newActor) { allActors.push_back(newActor); }

private:
    bool shouldAddActor(int randParaOne, int randParaTwo);
    void shouldAddZombCab();
    bool addCab(int leftSide, int middle, int rightSide);  
    int actorInWhichLane(Actor* object);

    GhostRacer* theGhostRacer;
    std::vector<Actor*> allActors;
    double lastWBLValueY;
    int m_bonusPTS;
    int soulsToSave;
    bool hitHumanPed;
};

#endif // STUDENTWORLD_H_
