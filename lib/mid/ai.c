// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"

static void dojump(Enemy*,Player*,Lvl*);
static void walk(Enemy*,Player*,Lvl*);
static void patrol(Enemy*,Player*,Lvl*);
static void chase(Enemy*,Player*,Lvl*);
static void hunt(Enemy*,Player*,Lvl*);

void aijumper(Ai *ai, double jv){
	ai->update = dojump;
	ai->mv = (Point){0, jv};
	ai->awdst = 0;
}

void aiwalker(Ai *ai, double wv){
	ai->update = walk;
	ai->mv = (Point){wv, 0};
	ai->awdst = 0;
}

void aipatroller(Ai *ai, double wv){
	ai->update = patrol;
	ai->mv = (Point){wv, 0};
	ai->awdst = 0;
}

void aichaser(Ai *ai, double wv, double awdst){
	ai->update = chase;
	ai->mv = (Point){wv, 0};
	ai->awdst = awdst;
}

void aihunter(Ai *ai, double jv, double wv, double awdst){
	ai->update = hunt;
	ai->mv = (Point){wv, jv};
	ai->awdst = awdst;
}

static void dojump(Enemy *e, Player *p, Lvl *lvl){
	if(!e->b.fall)
		e->b.vel.y = -e->ai.mv.y;
}

static void walk(Enemy *e, Player *p, Lvl *lvl){

}

static void patrol(Enemy *e, Player *p, Lvl *lvl){

}

static void chase(Enemy *e, Player *p, Lvl *lvl){

}

static void hunt(Enemy *e, Player *p, Lvl *lvl){

}
