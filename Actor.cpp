#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include "GameWorld.h"
#include <cmath>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp


Actor::Actor(StudentWorld* sw, int imageID, double x, double y, int dir, double size, int depth)
	:GraphObject(imageID, x, y, dir, size, depth), isDeceased(false)
{
	m_world = sw;
}

BorderLine::BorderLine(StudentWorld* sw, int imageID, double x, double y, bool isWhite)
	:Actor(sw, imageID, x, y, 0, 2.0, 2)
{
	m_white = isWhite;
	setVspeed(-4);
}

void BorderLine::doSomething()
{
	moveDependents(0);
}

Agent::Agent(StudentWorld* sw, int imageID, double x, double y, int dir, double size, int hp)
	: Actor(sw, imageID, x, y, dir, size, 0)
{
	setHP(hp);
}

bool Agent::takeDamageAndPossiblyDie(int hp)
{
	setHP(getHP() - hp);
	if (getHP() <= 0)
	{
		world()->playSound(soundWhenDie());
		setDead();
		world()->increaseScore(scoreOnDeath());
		return true;
	}
	else
	{
		world()->playSound(soundWhenHurt());
		return false;
	}
}

GhostRacer::GhostRacer(StudentWorld* sw, double x, double y)
	: Agent(sw, IID_GHOST_RACER, 128.0, 32.0, 90, 4.0, 100)
{
	m_sprays = 10;
	setVspeed(0);
}

void GhostRacer::doSomething()
{
	if (isDead())
		return;

	if (getX() < LEFT_EDGE) //If ghost racer is left of the road:
	{
		setDirection(82);
		world()->playSound(SOUND_VEHICLE_CRASH);
		takeDamageAndPossiblyDie(10);
	}

	if (getX() > RIGHT_EDGE) //If ghost racer is right of the road:
	{
		setDirection(98);
		world()->playSound(SOUND_VEHICLE_CRASH);
		takeDamageAndPossiblyDie(10);
	}

	int ch;
	if (world()->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_SPACE:
			if (getNumSprays() > 0)
			{
				double delta_y;
				double delta_x;
				getPositionInThisDirection(getDirection(), SPRITE_HEIGHT, delta_x, delta_y);
				world()->addActor(new Spray(world(), delta_x, delta_y, getDirection()));
				setSprays(getNumSprays() - 1);
				world()->playSound(SOUND_PLAYER_SPRAY);
			}
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() < 114)
				setDirection(getDirection() + 8);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() > 66)
				setDirection(getDirection() - 8);
			break;
		case KEY_PRESS_UP:
			if (getVspeed() < 5)
				setVspeed(getVspeed() + 1);
			break;
		case KEY_PRESS_DOWN:
			if (getVspeed() > -1)
				setVspeed(getVspeed() - 1);
			break;
		}
	}

	double delta_x = cos(getDirection() * PI / 180) * MAX_SHIFT_PER_TICK;
	double cur_x = getX();
	double cur_y = getY();
	moveTo(cur_x + delta_x, cur_y);
}

void GhostRacer::spin()
{
	//Randomize degree and sign
	int randomDegree = randInt(5, 20);
	int randomSign = randInt(1, 2);

	if (randomSign == 1)
		randomDegree = randomDegree * -1;

	if (randomDegree > 0)
	{
		if (getDirection() + randomDegree <= 120)
		{
			setDirection(getDirection() + randomDegree);
		}
		else
			setDirection(120);
	}

	else if (randomDegree < 0)
	{
		if (getDirection() + randomDegree >= 60)
		{
			setDirection(getDirection() + randomDegree);
		}
		else
			setDirection(60);
	}
}

ZombieCab::ZombieCab(StudentWorld* sw, double x, double y, int startSpeed)
	: Agent(sw, IID_ZOMBIE_CAB, x, y, 90, 4.0, 3)
{
	hasDamagedRacer = false;
	setVspeed(world()->getRacerPTR()->getVspeed() + startSpeed);
	m_Hspeed = 0;
	mvtPlanDist = 0;
	setHP(3);
}

void ZombieCab::doSomething()
{
	if (isDead())
		return;

	if (doesOverlap(world()->getRacerPTR())) //If the cab overlaps with the racer, collide, etc.
	{
		if (!hasDamagedRacer)
		{
			world()->playSound(SOUND_VEHICLE_CRASH);
			world()->getRacerPTR()->takeDamageAndPossiblyDie(20);
			if (getX() <= world()->getRacerPTR()->getX())
			{
				m_Hspeed = -5;
				setDirection(120 + randInt(0, 19));
			}
			else if (getX() > world()->getRacerPTR()->getX())
			{
				m_Hspeed = 5;
				setDirection(60 - randInt(0, 19));
			}
			hasDamagedRacer = true;
		}
	}

	moveDependents(m_Hspeed);
	
	bool front = false;
	bool back = false;
	world()->checkCollision(this, front, back); //Check for actors in front and behind the cab

	if (getVspeed() > world()->getRacerPTR()->getVspeed() && front)
	{
		setVspeed(getVspeed() - 0.5);
		return;
	}

	if (getVspeed() <= world()->getRacerPTR()->getVspeed() && back)
	{
		setVspeed(getVspeed() + 0.5);
		return;
	}

	mvtPlanDist--;
	if (mvtPlanDist > 0)
		return;

	mvtPlanDist = randInt(4, 32);
	setVspeed(getVspeed() + randInt(-2, 2));
}

bool ZombieCab::beSprayedIfAppropriate()
{
	takeDamageAndPossiblyDie(1);
	if (isDead())
	{
		int randomInteger = randInt(1, 5);
		if (randomInteger == 1)
			world()->addActor(new OilSlick(world(), getX(), getY())); //Chance of adding an oil slick on death
	}
	return true;
}

Pedestrian::Pedestrian(StudentWorld* sw, int imageID, double x, double y, double size)
	:Agent(sw, imageID, x, y, 0, size, 2)
{
	setVspeed(-4);
	m_Hspeed = 0;
	mvtPlanDist = 0;
	setHP(2);
}

void Pedestrian::moveAndPossiblyPickPlan()
{
	moveDependents(getHspeed());

	mvtPlanDist--;
	if (mvtPlanDist > 0)
		return;

	int randomHspeed = 0;

	do
	{
		randomHspeed = randInt(-3, 3);
	} while (randomHspeed == 0);

	setHspeed(randomHspeed);

	mvtPlanDist = randInt(4, 32);

	if (getHspeed() < 0)
		setDirection(180);
	else
		setDirection(0);
}

HumanPedestrian::HumanPedestrian(StudentWorld* sw, double x, double y)
	:Pedestrian(sw, IID_HUMAN_PED, x, y, 2.0)
{
}

void HumanPedestrian::doSomething()
{
	if (isDead())
		return;

	if (doesOverlap(world()->getRacerPTR())) //Tell StudentWorld a human has been hit if it overlaps with the racer
	{
		world()->setHitHumanPed();
		return;
	}

	moveAndPossiblyPickPlan();
}

bool HumanPedestrian::beSprayedIfAppropriate()
{
	setHspeed(getHspeed() * -1);
	if (getDirection() == 180)
		setDirection(0);
	else
		setDirection(180);

	return true;
}

bool HumanPedestrian::takeDamageAndPossiblyDie(int hp)
{
	return false;
}

ZombiePedestrian::ZombiePedestrian(StudentWorld* sw, double x, double y)
	:Pedestrian(sw, IID_ZOMBIE_PED, x, y, 3.0)
{
	m_gruntTicks = 0;
}

void ZombiePedestrian::doSomething()
{
	if (isDead())
		return;

	if (doesOverlap(world()->getRacerPTR()))
	{
		world()->getRacerPTR()->takeDamageAndPossiblyDie(5);
		takeDamageAndPossiblyDie(2);
		return;
	}

	double xDistToGr = getX() - world()->getRacerPTR()->getX();
	double yDistToGr = getY() - world()->getRacerPTR()->getY();
	
	if (abs(xDistToGr) < 30 && yDistToGr > 0) //If the zombie ped is close enough to the racer, attack 
	{
		setDirection(270);
		if (xDistToGr < 0)
			setHspeed(1);
		else if (xDistToGr > 0)
			setHspeed(-1);
		else
			setHspeed(0);
		m_gruntTicks--;

		if (m_gruntTicks <= 0)
		{
			m_gruntTicks = 20;
			world()->playSound(SOUND_ZOMBIE_ATTACK);
		}
	}

	moveAndPossiblyPickPlan();
}

bool ZombiePedestrian::beSprayedIfAppropriate()
{
	int randomInteger = randInt(1, 5);
	takeDamageAndPossiblyDie(1);

	if (isDead() && !doesOverlap(world()->getRacerPTR())) //Chance of adding a new healing goodie if it doesn't overlap with the racer
	{
		if (randomInteger == 1)
			world()->addActor(new HealingGoodie(world(), getX(), getY()));
	}

	return true;
}

Spray::Spray(StudentWorld* sw, double x, double y, int dir)
	:Actor(sw, IID_HOLY_WATER_PROJECTILE, x, y, dir, 1.0, 1)
{
	m_distTraveled = 0;
}

void Spray::doSomething()
{
	if (isDead())
		return;

	if (world()->doSprayEffects(this)) //Check if it overlaps with a collision actor
	{
		setDead();
		return;
	}

	moveForward(SPRITE_HEIGHT);
	m_distTraveled = m_distTraveled + SPRITE_HEIGHT;
	

	if (m_distTraveled > 160)
		setDead();
}

GhostRacerActivatedObject::GhostRacerActivatedObject(StudentWorld* sw, int imageID, double x, double y, int dir, double size)
	:Actor(sw, imageID, x, y, dir, size, 2)
{
	setVspeed(-4);
}

bool GhostRacerActivatedObject::beSprayedIfAppropriate()
{
	if (!isSprayable())
		return false;
	else
	{
		setDead(); //Kill objects that are sprayable
		return true;
	}
}


void GhostRacerActivatedObject::doSomething()
{
	moveDependents(0);
	doActivity(world()->getRacerPTR());
}

OilSlick::OilSlick(StudentWorld* sw, double x, double y)
	:GhostRacerActivatedObject(sw, IID_OIL_SLICK, x, y, 0, randInt(2,5))
{
}

void OilSlick::doActivity(GhostRacer* gr)
{	
	if (doesOverlap(gr))
	{
		world()->playSound(SOUND_OIL_SLICK);
		gr->spin();
	}
}

HealingGoodie::HealingGoodie(StudentWorld* sw, double x, double y)
	:GhostRacerActivatedObject(sw, IID_HEAL_GOODIE, x, y, 0, 1.0)
{
}

void HealingGoodie::doActivity(GhostRacer* gr)
{
	if (doesOverlap(gr))
	{
		gr->setHP(gr->getHP() + 10);
		setDead();
		world()->playSound(SOUND_GOT_GOODIE);
		world()->increaseScore(250);
	}
}


HolyWaterGoodie::HolyWaterGoodie(StudentWorld* sw, double x, double y)
	:GhostRacerActivatedObject(sw, IID_HOLY_WATER_GOODIE, x, y, 90, 2.0)
{
}

void HolyWaterGoodie::doActivity(GhostRacer* gr)
{
	if (doesOverlap(gr))
	{
		gr->setSprays(gr->getNumSprays() + 10);
		setDead();
		world()->playSound(SOUND_GOT_GOODIE);
		world()->increaseScore(50);
	}
}

SoulGoodie::SoulGoodie(StudentWorld* sw, double x, double y)
	:GhostRacerActivatedObject(sw, IID_SOUL_GOODIE, x, y, 0, 4.0 )
{
}

void SoulGoodie::doActivity(GhostRacer* gr)
{	
	if (doesOverlap(gr))
	{
		world()->decSoulsToSave();
		setDead();
		world()->playSound(SOUND_GOT_SOUL);
		world()->increaseScore(100);
	}
	setDirection(getDirection() - 10);
}

void Actor::moveDependents(double hspeed) //Moves actors dependent on ghost racer's speed
{
	double vert_speed = getVspeed() - (world()->getRacerPTR()->getVspeed());
	double horiz_speed = hspeed;
	double new_y = getY() + vert_speed;
	double new_x = getX() + horiz_speed;
	moveTo(new_x, new_y);

	if (getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
	{
		setDead();
		return;
	}
}

bool Actor::doesOverlap(Actor* object)
{
	double delta_x = abs(object->getX() - getX());
	double delta_y = abs(object->getY() - getY());
	double radius_sum = object->getRadius() + getRadius();

	if (delta_x < radius_sum * 0.25 && delta_y < radius_sum * 0.6)
		return true;
	else
		return false;
}