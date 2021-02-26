#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED

#include "GraphObject.h"

const double PI = 3.14159;
const double MAX_SHIFT_PER_TICK = 4.0;

class StudentWorld;

class Actor : public GraphObject
{
public:
    Actor(StudentWorld* sw, int imageID, double x, double y, int dir, double size, int depth);
    virtual void doSomething() = 0;
    bool isDead() const { return isDeceased; }
    double getVspeed() const { return m_Vspeed; }
    virtual bool isWhiteBL() { return false; }
    virtual bool beSprayedIfAppropriate() { return false; }
    virtual bool isCollisionAvoidanceWorthy() const { return false; }
protected:
    void moveDependents(double hspeed);
    bool doesOverlap(Actor* object);
    void setDead() { isDeceased = true; }
    StudentWorld* world() const { return m_world; }
    void setVspeed(double speed) { m_Vspeed = speed; }  
private:
    double m_Vspeed;
    bool isDeceased;
    StudentWorld* m_world;
};

class BorderLine : public Actor
{
public:
    BorderLine(StudentWorld* sw, int imageID, double x, double y, bool isWhite = false);
    virtual void doSomething();
    virtual bool isWhiteBL() { return m_white; }
private:
    bool m_white;
};

class Agent : public Actor
{
public:
    Agent(StudentWorld* sw, int imageID, double x, double y, int dir, double size, int hp);
    virtual bool isCollisionAvoidanceWorthy() const { return true; }
    int getHP() const { return m_health; }
    void setHP(int hp)  { m_health = hp; }
    virtual int scoreOnDeath() { return 0; }
    virtual bool takeDamageAndPossiblyDie(int hp);
    virtual int soundWhenHurt() const { return SOUND_NONE; }
    virtual int soundWhenDie() const { return SOUND_NONE; }
private:
    int m_health;
};

class GhostRacer : public Agent
{
public:
    GhostRacer(StudentWorld* sw, double x, double y);
    virtual void doSomething();
    virtual int soundWhenHurt() const { return SOUND_NONE; }
    virtual int soundWhenDie() const { return SOUND_PLAYER_DIE; }
    int getNumSprays() const { return m_sprays; }
    void setSprays(int amt) { m_sprays = amt; }
    void spin();
private:
    int m_sprays;
};

class Pedestrian : public Agent
{
public:
    Pedestrian(StudentWorld* sw, int imageID, double x, double y, double size);
    virtual int soundWhenHurt() const { return SOUND_PED_HURT; }
    virtual int soundWhenDie() const { return SOUND_PED_DIE; }
protected:
    int getHspeed() const { return m_Hspeed; }
    void setHspeed(int s) { m_Hspeed = s; }
    void moveAndPossiblyPickPlan();
private:
    double m_Hspeed;
    int mvtPlanDist;
};

class HumanPedestrian : public Pedestrian
{
public:
    HumanPedestrian(StudentWorld* sw, double x, double y);
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
    virtual bool takeDamageAndPossiblyDie(int hp);
};

class ZombiePedestrian : public Pedestrian
{
public:
    ZombiePedestrian(StudentWorld* sw, double x, double y);
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
    virtual int soundWhenHurt() const { return SOUND_PED_HURT; }
    virtual int soundWhenDie() const { return SOUND_PED_DIE; }
    virtual int scoreOnDeath() { return 150; }
private:
    int m_gruntTicks;
};

class ZombieCab : public Agent
{
public:
    ZombieCab(StudentWorld* sw, double x, double y, int startSpeed);
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
    virtual int soundWhenHurt() const { return SOUND_VEHICLE_HURT; }
    virtual int soundWhenDie() const { return SOUND_VEHICLE_DIE; }
    virtual int scoreOnDeath() { return 200; }
private:
    bool hasDamagedRacer;
    double m_Hspeed;
    int mvtPlanDist;
};

class Spray : public Actor
{
public:
    Spray(StudentWorld* sw, double x, double y, int dir);
    virtual void doSomething();
private:
    int m_distTraveled;
};

class GhostRacerActivatedObject : public Actor
{
public:
    GhostRacerActivatedObject(StudentWorld* sw, int imageID, double x, double y, int dir, double size);
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
    virtual void doActivity(GhostRacer* gr) = 0;
    virtual bool isSprayable() const = 0;
};

class OilSlick : public GhostRacerActivatedObject
{
public:
    OilSlick(StudentWorld* sw, double x, double y);
    virtual void doActivity(GhostRacer* gr);
    virtual bool isSprayable() const { return false; }
};

class HealingGoodie : public GhostRacerActivatedObject
{
public:
    HealingGoodie(StudentWorld* sw, double x, double y);
    virtual void doActivity(GhostRacer* gr);
    virtual bool isSprayable() const { return true; }
};

class HolyWaterGoodie : public GhostRacerActivatedObject
{
public:
    HolyWaterGoodie(StudentWorld* sw, double x, double y);
    virtual void doActivity(GhostRacer* gr);
    virtual bool isSprayable() const { return true; }
};

class SoulGoodie : public GhostRacerActivatedObject
{
public:
    SoulGoodie(StudentWorld* sw, double x, double y);
    virtual void doActivity(GhostRacer* gr);
    virtual bool isSprayable() const { return false; }
};

#endif // ACTOR_INCLUDED