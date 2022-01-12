#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define MAX_MSTOCK 5
#define INT_MAX 1000001
#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({              \
    void *__mptr = (void *)(ptr);                   \
    ((type *)((uintptr_t)__mptr - (uintptr_t)offsetof(type, member))); })

#define LIST_POISON1    ((struct list *)0xdeadbeef)
#define LIST_POISON2    ((struct list *)0xbadacafe)


struct list{
	struct list *next, *prev;
};

static inline void INIT_LIST(struct list *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list *__next,
                  struct list *prev,
                  struct list *next)
{
    next->prev = __next;
    __next->next = next;
    __next->prev = prev;
    prev->next = __next;
}

static inline void list_add(struct list *__next, struct list *head)
{
    __list_add(__next, head, head->next);
}

static inline void list_add_tail(struct list *__next, struct list *head)
{
    __list_add(__next, head->prev, head);
}

static inline void __list_del(struct list * prev, struct list * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(struct list *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list *entry)
{
    __list_del_entry(entry);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

static inline void list_del_init(struct list *entry)
{
    __list_del_entry(entry);
    INIT_LIST(entry);
}

static inline int list_empty(const struct list *head)
{
    return head->next == head;
}

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_first_entry_or_null(ptr, type, member) ({ \
    struct list *head__ = (ptr); \
    struct list *pos__ = head__->next; \
    pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
	})

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, __typeof__(*(pos)), member)

#define list_for_each_entry(pos, head, member)              \
    for (pos = list_first_entry(head, __typeof__(*pos), member);    \
         &pos->member != (head);                    \
         pos = list_next_entry(pos, member))

enum {
	BUY = 0,
	SELL = 1,
};


struct stock{
	int mNumber;
	int mStock;
	int mQuantity;
	int mPrice;
	int type;

	struct list list;
};

struct profit{
	struct list list;
	int price;
	int number;
};

struct list minPriceList[MAX_MSTOCK];
struct list maxPriceList[MAX_MSTOCK];
struct list stockList;

static int count = 0;

void dump(struct stock *stock)
{
	char strType[5];
	if(stock->type == 0){
		strcpy(strType,"BUY");
	}else{
		strcpy(strType,"SELL");
	} 
	printf("%s : mNumber : %d  mStock: %d mQuantity : %d mPrice : %d \n",strType,stock->mNumber,
			stock->mStock,stock->mQuantity,stock->mPrice);
}

static struct stock* allocStock(int mNumber, int mStock, 
		int mQuantity, int mPrice, int type)
{
	struct stock *stock = (struct stock*)malloc(sizeof(*stock));
	if(!stock){
		fprintf(stderr,"Unable to alloc stock %d\n",__LINE__);
		return NULL;
	}

	stock->mNumber =mNumber;
	stock->mStock = mStock;
	stock->mQuantity = mQuantity;
	stock->mPrice = mPrice;
	stock->type = type;

	list_add_tail(&stock->list,&stockList);
	dump(stock);
	return stock;
}

static struct stock* findPriceMin(int type,int mStock)
{
	struct stock *target = NULL;
	struct stock *iter = NULL;
	int minPrice = INT_MAX;
	
	list_for_each_entry(iter,&stockList,list){
		if((iter->type == type) && (iter->mStock == mStock) &&
				(minPrice > iter->mPrice)){
			minPrice = iter->mPrice;
			target = iter;
		}
	}

	return target;
}

static struct stock* findPriceMax(int type,int mStock)
{
	struct stock *target = NULL;
	struct stock *iter = NULL;
	int maxPrice = 0;

	list_for_each_entry(iter,&stockList,list){
		if((iter->type == type) && (iter->mStock == mStock) &&
				(maxPrice < iter->mPrice)){
			maxPrice = iter->mPrice;
			target = iter;
		}
	}

	return target;
}

static int trade(struct stock *sell, struct stock *buy)
{
	int sub = 0;
	int ret = 0;
	sub = sell->mQuantity - buy->mQuantity;
	
	if(sub >= 0){
		sell->mQuantity = sub;
		buy->mQuantity = 0;
		list_del_init(&buy->list);
		ret ++;
		//free(buy);
	}
	if(sub <= 0){
		buy->mQuantity = (-1) *sub;
		sell->mQuantity = 0;
		list_del_init(&sell->list);
		ret --;
		//free(sell);
	}
	return ret;
}
static void addMinPriceList(int mStock,int nNumber, int price)
{
	struct profit *profit = (struct profit*)malloc(sizeof(*profit));
	if(!profit)
	{
		fprintf(stderr,"Unable to alloc stock %d\n",__LINE__);
	}
	profit->price = price;
	profit->number = nNumber;
	list_add_tail(&profit->list,minPriceList+mStock-1);
}

void addMaxPriceList(int mStock,int nNumber, int price)
{
	struct profit *profit = (struct profit*)malloc(sizeof(*profit));
	if(!profit)
	{
		fprintf(stderr,"Unable to alloc stock %d\n",__LINE__);
	}
	profit->price = price;
	profit->number = nNumber;
	list_add_tail(&profit->list,maxPriceList+mStock-1);
}

void init(void)
{
	INIT_LIST(&stockList);
	for(int i = 0 ; i < MAX_MSTOCK; i++){
		INIT_LIST(&maxPriceList[i]);
		INIT_LIST(&minPriceList[i]);
	}
}

int buy(int mNumber, int mStock, int mQuantity, int mPrice)
{
	int ret = -1;
	struct stock* buyStock = allocStock(mNumber,mStock,mQuantity,
			mPrice,BUY);

	while(ret < 0){
		struct stock *sellPriceMinStock = findPriceMin(SELL,mStock);

		if(!sellPriceMinStock)
			break;
		if(sellPriceMinStock->mPrice > buyStock->mPrice)
			break;
	
		count ++;

		addMinPriceList(mStock,count,sellPriceMinStock->mPrice);	
		addMaxPriceList(mStock,count,sellPriceMinStock->mPrice);	

		ret = trade(sellPriceMinStock,buyStock);
	}
	return buyStock->mQuantity;
}

int sell(int mNumber, int mStock, int mQuantity, int mPrice)
{
	int ret = 1;
	struct stock* sellStock = allocStock(mNumber,mStock,mQuantity,
			mPrice,SELL);

	while(ret > 0){
		struct stock *buyPriceMaxStock = findPriceMax(BUY,mStock);

		if(!buyPriceMaxStock)
			break;

		if(buyPriceMaxStock->mPrice < sellStock->mPrice)
			break;
		
		count ++;
		addMaxPriceList(mStock,count,buyPriceMaxStock->mPrice);
		addMinPriceList(mStock,count,buyPriceMaxStock->mPrice);
	
		ret = trade(sellStock,buyPriceMaxStock);
	}

	return sellStock->mQuantity;
}

void cancel(int mNumber)
{
	struct stock *iter = NULL;
	bool check = false;
	list_for_each_entry(iter,&stockList,list){
		if(iter->mNumber == mNumber){
			check = true;
			break;
		}
	}

	if(check){
		list_del_init(&iter->list);
	}
}

int bestProfit(int mStock)
{
	int bProfit = 0;
	struct profit *max = NULL , *min = NULL;
	list_for_each_entry(min,&minPriceList[mStock-1],list){
		list_for_each_entry(max,&maxPriceList[mStock-1],list){
			if(max->number < min->number){
				continue;
			}
			if(bProfit < max->price - min->price)
				bProfit = max->price - min->price;
		}
	};
	return bProfit;
}
