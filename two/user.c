//	 The below commented functions are for your reference. If you want 
//	 to use it, uncomment these functions.

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#define MALE   0
#define FEMALE 1

#define DAD MALE
#define MOM FEMALE

#define INIT         0
#define ADDMEMBER    1
#define GETDISTANCE  2
#define COUNTMEMBER  3

#define COUPLE  0
#define PARENT  1
#define CHILD   2

#define NAME_LEN_MAX 19
#define DEFAULT_CHILD 64
#define DEFAULT_MAP 1024

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

struct member{
	
	char name[NAME_LEN_MAX];
	int sex;
	int nChild;
	int sizeChild;

	struct member *couple;
	struct member *parents[2];
	struct member **child;
};

struct map{
	struct member **member;
	size_t size;
	size_t nMember;
};

int mstrcmp(const char a[], const char b[])
{
	int i;
	for (i = 0; a[i] != '\0'; ++i) if (a[i] != b[i]) return a[i] - b[i];
	return a[i] - b[i];
}

void mstrcpy(char dest[], const char src[])
{
	int i = 0;
	while (src[i] != '\0') { dest[i] = src[i]; i++; }
	dest[i] = src[i];
}

int mstrlen(const char a[])
{
	int i;
	for (i = 0; a[i] != '\0'; ++i);
	return i;
}

struct member *initialMember;
struct map *map;


static void addMemberMap(struct member* member)
{
	if(map->size == map->nMember){
		map->member = (struct member **)realloc(map->member,map->size*2);
		for(size_t i = 0; i < map->size; i++)
			map->member[i+map->size] = NULL;
		map->size *= 2;
	}

	map->member[map->nMember] = member;
	map->nMember += 1;
}

static struct member *findMemberMap(char *name)
{
	struct member *target = NULL;

	for(size_t i = 0; i < map->nMember; i++){
		if(!mstrcmp(name,map->member[i]->name)){
			target = map->member[i];
			break;
		}
	}
	return target;
}

static struct member* allocMember(const char* name, const int sex)
{
	struct member *member = (struct member*)malloc(sizeof(*member));
	if(!member){
		exit(1);
	}
	mstrcpy(member->name,name);
	member->sex = sex;
	member->parents[DAD] = NULL;
	member->parents[MOM] = NULL;

	member->child = (struct member**)malloc(
			sizeof(*member->child)*DEFAULT_CHILD);
	member->nChild = 0;
	member->sizeChild = DEFAULT_CHILD;

	for(int i = 0; i < DEFAULT_CHILD; i++){
		member->child[i] = NULL;
	}
	addMemberMap(member);

	return member;
}

/* 예외 : 성별이 같은 구성원 간에는 배우자가 될 수 없고, 이미 배우자가 있는 구성원은 배우자를 추가할 수 없다.
*/



static inline bool  condCouple(struct member *male, struct member *female)
{
	return ((male->sex != female->sex) && (male->couple) && (female->couple));
} 

static inline bool  condParent(struct member *parent, struct member *child)
{
	return (!child->parents[parent->sex]);
}
static inline bool condChild(struct member *child)
{
	return true;
}

static void __insertChild(struct member*parent, struct member* child)
{
	if(parent->nChild == parent->sizeChild){
		parent->child = (struct member **)realloc(parent->child,parent->sizeChild*2);
		
		for(size_t i = 0; i < parent->sizeChild; i++)
			parent->child[i+parent->sizeChild] = NULL;
		
		parent->sizeChild *= 2;
	}

	parent->child[parent->nChild] = child;
	parent->nChild ++;
	/* 의심 */
	if(child->parents[parent->sex]){
		printf("Something wrong %d %s %s\n",__LINE__,parent->name, child->name);
	}
	child->parents[parent->sex] = parent;
}

static void insertChild(struct member *par, struct member *ent)
{

	struct member *male = (par->sex) ? par:ent;
	struct member *female = (!par->sex) ? par : ent;
	size_t nChild = male->nChild + female->nChild;
	/*
	if(nChild > male->sizeChild){
		male->child = (struct member **)realloc(male->child,male->sizeChild*2);
		for(size_t i = 0; i < male->sizeChild; i++)
			male->child[i+male->sizeChild] = NULL;
		male->sizeChild *= 2;
	}

	if(nChild > female->sizeChild){
		female->child = (struct member **)realloc(female->child,female->sizeChild*2);
		for(size_t i = 0; i < female->sizeChild; i++)
			female->child[i+ female->sizeChild] = NULL;
		female->sizeChild *= 2;
	}

	for(size_t i = 0; i < female->nChild; i++ ){
		struct member *child = female->child[i];
		child->parents[DAD] = male;
		male->child[i+male->nChild];
		male->nChild ++;
	}

	for(size_t i = 0; i < male->nChild; i++ ){
		struct member *child = female->child[i];
		child->parents[MOM] = female;
		female->child[i+female->nChild];
		female->nChild ++;
	}
*/
	for(size_t i = 0 ; i < male->nChild; i++){
		__insertChild(female,male->child[i]);
	}

	for(size_t i = 0 ; i < female->nChild; i++){
		__insertChild(male,female->child[i]);
	}
}

static void connectCouple(struct member *male ,struct member *female)
{
	male->couple = female;
	female->couple = male;

	insertChild(male,female);
}

static void connectParent(struct member* parent, struct member *child)
{
	struct member *otherParent = child->parents[!parent->sex];
	
	if(otherParent){
		insertChild(parent,otherParent);
	}
	else {
		__insertChild(parent,child);
	}
}


static void connectChild(struct member * parent, struct member *child)
{
	__insertChild(parent,child);
}

void init(char initialMemberName[], int initialMemberSex)
{
	initialMember = allocMember(initialMemberName,initialMemberSex);
	
	map = (struct map*)malloc(sizeof(*map));
	if(!map){
		exit(1);
	}
	map->member = (struct member**)malloc(sizeof(*map->member)*DEFAULT_MAP);
	if(!map->member){
		exit(1);
	}
	for(int i = 0; i < DEFAULT_MAP; i++){
		map->member[i] = NULL;
	}
	map->size = DEFAULT_MAP;
	map->nMember = 0;
}

bool addMember(char newMemberName[], int newMemberSex, int relationship, char existingMemberName[])
{
	bool ret = false;
	struct member *newMember = allocMember(newMemberName,newMemberSex);
	struct member *existingMember = findMemberMap(existingMemberName);
	switch(relationship){

		case COUPLE:
			ret = condCouple(newMember,existingMember);
			if(ret){
				connectCouple(newMember,existingMember);
			}
			break;

		case PARENT:
			ret = condParent(newMember,existingMember);
			if(ret){
				connectParent(newMember,existingMember);
			}
			break;

		case CHILD:
			ret = condChild(newMember);
			if(ret){
				connectChild(existingMember,newMember);
			}
			break;
		defualt:
			printf("FUCK\n");
	}

	return ret;
}

int getDistance(char nameA[], char nameB[])
{
	return -1;
}

int countMember(char name[], int dist)
{
	return -1;
}


