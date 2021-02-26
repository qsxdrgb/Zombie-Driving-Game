#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <unordered_set>

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    hitHumanPed = false;
}

/************************************************************
* Called by provided game framework each time the game starts.
* Constructs current level and populates with initial objects
*************************************************************/
int StudentWorld::init()
{
  
    theGhostRacer = new GhostRacer(this, 128, 32);   
    soulsToSave = 2 * getLevel() + 5;
    m_bonusPTS = 5000;
    //Construct yellow border lines
    for (int counter = 0; counter < (VIEW_HEIGHT / SPRITE_HEIGHT); counter++)
    {
        addActor(new BorderLine(this, IID_YELLOW_BORDER_LINE, LEFT_EDGE, counter * SPRITE_HEIGHT)); //Left edge
        addActor(new BorderLine(this, IID_YELLOW_BORDER_LINE, RIGHT_EDGE, counter * SPRITE_HEIGHT)); //Right edge
    }

    //Construct white border lines
    for (int counter = 0; counter < (VIEW_HEIGHT / (4 * SPRITE_HEIGHT)); counter++)
    {
        addActor(new BorderLine(this, IID_WHITE_BORDER_LINE, (LEFT_EDGE + ROAD_WIDTH / 3), counter * 4 * SPRITE_HEIGHT,true)); //Left edge
        addActor(new BorderLine(this, IID_WHITE_BORDER_LINE, (RIGHT_EDGE - ROAD_WIDTH / 3), counter * 4 * SPRITE_HEIGHT,true)); //Right edge
    }

    return GWSTATUS_CONTINUE_GAME;
}

/*****************************************
* Called 20x/sec, each move is a tick.
* Introduces new actors, deletes actors
* Called by provided game framework per tick
******************************************/
int StudentWorld::move()
{
    m_bonusPTS--;

    if (theGhostRacer->isDead())
    {
        //playSound(SOUND_PLAYER_DIE);
        decLives();
        return GWSTATUS_PLAYER_DIED;
    } 

    if (hitHumanPed)
    {
        decLives();
        hitHumanPed = false;
        return GWSTATUS_PLAYER_DIED;
    }

    if (soulsToSave == 0)
    {
        increaseScore(m_bonusPTS);
        return GWSTATUS_FINISHED_LEVEL;
    }

    //Destruct dead actors
    for (int i = 0; i < allActors.size(); i++)
    {
        if (allActors[i]->isDead())
        {
            delete allActors[i];
            allActors.erase(allActors.begin() + i);
            i--;
        }
    }

    /*******************
    * BORDER LINE CODE
    ********************/

    //Find last white border line
    for (int i = allActors.size() - 1; i >= 0; i--)
    {
        if (allActors[i]->isWhiteBL())
        {
            lastWBLValueY = allActors[i]->getY();
            break;
        }
    }

    double delta_y = NEW_BORDER_Y - lastWBLValueY;

    //Add new yellow border lines
    if (delta_y >= SPRITE_HEIGHT)
    {
        addActor(new BorderLine(this, IID_YELLOW_BORDER_LINE, LEFT_EDGE, NEW_BORDER_Y)); //Left edge
        addActor(new BorderLine(this, IID_YELLOW_BORDER_LINE, RIGHT_EDGE, NEW_BORDER_Y)); //Right edge
    }

    //Add new white border lines
    if (delta_y >= 4 * SPRITE_HEIGHT)
    {
        addActor(new BorderLine(this, IID_WHITE_BORDER_LINE, (LEFT_EDGE + ROAD_WIDTH / 3), NEW_BORDER_Y, true)); //Left edge
        addActor(new BorderLine(this, IID_WHITE_BORDER_LINE, (RIGHT_EDGE - ROAD_WIDTH / 3), NEW_BORDER_Y, true)); //Right edge 
    }

    theGhostRacer->doSomething();
    for (int i = 0; i < allActors.size(); i++)
    {
        allActors[i]->doSomething();
    }


    //Add oil slicks
    int ChanceOilSlick = max(150 - getLevel() * 10, 40);

    if (shouldAddActor(0 , ChanceOilSlick))
        addActor(new OilSlick(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));

    //Add holy water goodies
    int ChanceOfHolyWater = 100 + 10 * getLevel();

    if (shouldAddActor(0, ChanceOfHolyWater))
        addActor(new HolyWaterGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));

   //Add soul goodies
    int ChanceOfLostSoul = 100;

    if (shouldAddActor(0, ChanceOfLostSoul))
        addActor(new SoulGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));

    //Add human peds
    int ChanceHumanPed = max(200 - getLevel() * 10, 30);

    if (shouldAddActor(0, ChanceHumanPed))
        addActor(new HumanPedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));

    //Add zombie peds
    int ChanceZombiePed = max(100 - getLevel() * 10, 20);

    if (shouldAddActor(0, ChanceZombiePed))
        addActor(new ZombiePedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
    //Add zombie cabs

    int ChanceVehicle = max(100 - getLevel() * 10, 20);
    if (shouldAddActor(0, ChanceVehicle))
        shouldAddZombCab();

    /****************
    * GAME TEXT CODE 
    *****************/
    ostringstream gameText;
    gameText << "Score: " << getScore() << setw(6) << "Lvl: " << getLevel() << setw(14)
        << "Souls2Save: " << soulsToSave << setw(9) << "Lives: " << getLives() << setw(10) << "Health: "
        << theGhostRacer->getHP() << setw(10) << "Sprays: " << theGhostRacer->getNumSprays() << setw(9) << "Bonus: " << m_bonusPTS;
    setGameStatText(gameText.str());
    return GWSTATUS_CONTINUE_GAME;
}

/*********************************************************
* Called by provided game framework on level end/life loss
* Frees all actors currently in the game
***********************************************************/
void StudentWorld::cleanUp()
{   
    while (allActors.size() != 0)
    {
        delete (allActors.back());
        allActors.pop_back();
    }   
    delete theGhostRacer;   
}

bool StudentWorld::shouldAddActor(int randParaOne, int randParaTwo)
{
    if (randInt(randParaOne, randParaTwo) == 0)
        return true;
    else
        return false;
}

void StudentWorld::shouldAddZombCab()
{
    bool flag = false;
    //1 == left , 2 == mid, 3 == right
    unordered_set<int> randomizer;

    while (randomizer.size() < 3)
    {
        randomizer.insert(randInt(1, 3));
    }

    unordered_set<int>::iterator it = randomizer.begin();
    while (it != randomizer.end())
    { 
        switch (*it)
        {
        case 1:
            if(addCab(LEFT_EDGE, ROAD_CENTER - ROAD_WIDTH/3, LEFT_WHITE_LANE))
                flag = true;
            break;
        case 2:
            if (addCab(LEFT_WHITE_LANE, ROAD_CENTER, RIGHT_WHITE_LANE))
                flag = true;
            break;
        case 3:
            if (addCab(RIGHT_WHITE_LANE, ROAD_CENTER + ROAD_WIDTH /3, RIGHT_EDGE))
                flag = true;
            break;
        }

        if (flag)
            break; 
        it++;  
    }    
}

bool StudentWorld::doSprayEffects(Actor* spray)
{
    for (int i = 0; i < allActors.size(); i++)
    {
        if (doesOverlap(allActors[i], spray))
        {
            if (allActors[i]->beSprayedIfAppropriate())
                return true;
        }
    }
    return false;
}

bool StudentWorld::doesOverlap(Actor* one, Actor* two)
{
    double delta_x = abs(one->getX() - two->getX());
    double delta_y = abs(one->getY() - two->getY());
    double radius_sum = one->getRadius() + two->getRadius();

    if (delta_x < radius_sum * 0.25 && delta_y < radius_sum * 0.6)
        return true;
    else
        return false;
}

bool StudentWorld::addCab(int leftSide, int middle, int rightSide)
{
    bool hasAddedInLane = false;

    double minY = VIEW_HEIGHT / 3;

    for (int i = 0; i < allActors.size(); i++)
    {
        if (allActors[i]->isCollisionAvoidanceWorthy() && allActors[i]->getX() > leftSide && allActors[i]->getX() < rightSide)
        {
            if (allActors[i]->getY() < minY)
                minY = allActors[i]->getY();
        }
    }
    if (theGhostRacer->getX() > leftSide && theGhostRacer->getX() < rightSide)
        minY = theGhostRacer->getY();

    if (minY >= VIEW_HEIGHT / 3)
    {
        hasAddedInLane = true;
        addActor(new ZombieCab(this, middle, SPRITE_HEIGHT / 2, randInt(2, 4)));
    }

    if (hasAddedInLane)
        return true;

    double maxY = VIEW_HEIGHT * 2 / 3;

    for (int i = 0; i < allActors.size(); i++)
    {
        if (allActors[i]->isCollisionAvoidanceWorthy() && allActors[i]->getX() > leftSide && allActors[i]->getX() < rightSide)
        {
            if (allActors[i]->getY() > maxY)
                maxY = allActors[i]->getY();
        }
    }

    if (maxY <= VIEW_HEIGHT * 2 / 3)
    {
        hasAddedInLane = true;
        addActor(new ZombieCab(this, middle, VIEW_HEIGHT - SPRITE_HEIGHT / 2, randInt(-2, -4)));
    }

    return hasAddedInLane;
}

void StudentWorld::checkCollision(Actor* cab, bool& front, bool& back)
{
    int laneToCheck = actorInWhichLane(cab);

    for (int i = 0; i < allActors.size(); i++)
    {
        if (actorInWhichLane(allActors[i]) == laneToCheck && allActors[i]->isCollisionAvoidanceWorthy())
        {
            if (allActors[i]->getY() - cab->getY() > 0 && allActors[i]->getY() - cab->getY() < 96)
                front = true;
            if (cab->getY() - allActors[i]->getY() > 0 && cab->getY() - allActors[i]->getY() < 96)
                back = true;
        }
    }
}

//Return 1 for left lane, 2 for mid lane, 3 for right lane
int StudentWorld::actorInWhichLane(Actor* object)
{
    if (object->getX() < LEFT_WHITE_LANE)
        return 1;
    else if (object->getX() > RIGHT_WHITE_LANE)
        return 3;
    else
        return 2;
}


StudentWorld::~StudentWorld()
{
    cleanUp();
}
