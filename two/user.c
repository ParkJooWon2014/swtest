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
#define DEFUALT_CHILD 64
#define DEFUALT_MAP 550
#define INT_MAX 500
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

	bool visited;
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
		memset(map->member+map->size,0,sizeof(struct member*)*map->size);
		map->size *= 2;
	}

	map->member[map->nMember] = member;
	map->nMember ++;
}

static struct member *findMemberMap(char *name)
{
	struct member *target = NULL;

	for(size_t i = 0; i < map->nMember; i++){
		if(!mstrcmp(map->member[i]->name,name)){
			target = map->member[i];
			break;
		}
	}
	return target;
}

static int findMemberMapIndex(char *name)
{
	int index = -1 ;
	for(int i = 0; i < map->nMember; i++){
		if(!mstrcmp(map->member[i]->name,name)){
			index = i;
			break;
		}
	}
	return index;
}


static struct member* allocMember(const char* name, const int sex)
{
	struct member *member = (struct member*)malloc(sizeof(*member));
	if(!member){
		return NULL;
	}

	mstrcpy(member->name,name);
	member->sex = sex;
	member->parents[DAD] = NULL;
	member->parents[MOM] = NULL;

	member->child = (struct member**)malloc(
			sizeof(*member->child)*DEFUALT_CHILD);
	member->nChild = 0;
	member->sizeChild = DEFUALT_CHILD;
	member->visited = false;
	for(int i = 0; i < DEFUALT_CHILD; i++){
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

//	if(child->parents[parent->sex]){
//		("Something wrong %d %s %s\n",__LINE__,parent->name, child->name);
//	}
	child->parents[parent->sex] = parent;
}

static void insertChild(struct member *par, struct member *ent)
{

	struct member *male = (par->sex) ? par:ent;
	struct member *female = (!par->sex) ? par : ent;
	size_t nChild = male->nChild + female->nChild;
	
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

static int choose(int *distance,size_t nMember, bool *visited)
{
	int min = INT_MAX;
	int minpos = -1;
	for(size_t i = 0; i < nMember; i++){
		if(distance[i] < min && !visited[i]){
			min = distance[i];
			minpos = i;
		}
	}
	return minpos;
}

static void updateDistance(struct member *mem, int *distance)
{
	int coupleIndex = -1;
	int momIndex = -1;
	int dadIndex = -1;

	if(mem->couple)
		coupleIndex = findMemberMapIndex(mem->couple->name);
	if(mem->parents[MOM])
		momIndex = findMemberMapIndex(mem->parents[MOM]->name);
	if(mem->parents[DAD])
		dadIndex = findMemberMapIndex(mem->parents[DAD]->name);
	
	for(int i = 0; i < map->nMember ; i++){
		if(i == coupleIndex){
			distance[i] = 0;
		}
		else if(i == momIndex || i == dadIndex){
			distance[i] = 1;
		}
		else {
			distance[i] = INT_MAX;
		}
	}
}

void init(char initialMemberName[], int initialMemberSex)
{	
	map = (struct map*)malloc(sizeof(*map));
	if(!map){
		return;
	}
	map->member = (struct member**)malloc(sizeof(struct member*)*DEFUALT_MAP);
	if(!map->member){
		return;
	}
	for(int i = 0; i < DEFUALT_MAP; i++){
		map->member[i] = NULL;
	}
	map->size = DEFUALT_MAP;
	map->nMember = 0;

	initialMember = allocMember(initialMemberName,initialMemberSex);
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
			;
	}

	return ret;
}

int getDistance(char nameA[], char nameB[])
{
	int src = findMemberMapIndex(nameA);
	struct member *dst;
	struct member *tmp = NULL;
	struct member *current = NULL;
	int alt = 0;
	bool *visited = (bool*)malloc(sizeof(bool)*map->nMember);
	int *dist = (int*)malloc(sizeof(int)*map->nMember);
	int *queue = (int*)malloc(sizeof(*queue)*map->nMember);
	int head =0 , tail = 0;
	int u;
	for(size_t i = 0; i < map->nMember; i++){
		visited[i] = false;
		dist[i] = INT_MAX;
	}

	dist[src] = 0;
	visited[src] = true;
	queue[head] = src;
	tail++;

	while(head == tail){
		current = map->member[queue[head]];
		head++;
		updateDistance(current,dist);
		u = choose(dist,map->nMember,visited);
	}


	return u;
}

int countMember(char name[], int dist)
{
	return -1;
}


