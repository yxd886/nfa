#ifndef HASHAA_H
#define HASHAA_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#define NULLKEY 0 // 0为无记录标志
#define N 10  // 数据元素个数



typedef char* KeyType;// 设关键字域为char*
typedef struct{
	char key[20];
	char value[100];
}ElemType; // 数据元素类型



// 开放定址哈希表的存储结构

typedef struct{
	ElemType *elem; // 数据元素存储基址，动态分配数组
	int count; // 当前数据元素个数
	int sizeindex; // hashsize[sizeindex]为当前容量
}HashTable;


#define SUCCESS 1
#define UNSUCCESS 0
#define DUPLICATE -1


// 构造一个空的哈希表
int InitHashTable(HashTable *H);

//  销毁哈希表H
void DestroyHashTable(HashTable *H);
// 一个简单的哈希函数(m为表长，全局变量)

unsigned int Hash(char *str);

// 开放定址法处理冲突
void collision(int *p,int d);
// 算法9.17
// 在开放定址哈希表H中查找关键码为K的元素,若查找成功,以p指示待查数据
// 元素在表中位置,并返回SUCCESS;否则,以p指示插入位置,并返回UNSUCCESS
// c用以计冲突次数，其初值置零，供建表插入时参考。
int SearchHash(HashTable H,KeyType K,int *p,int *c);
int InsertHash(HashTable *,ElemType); // 对函数的声明
// 重建哈希表
void RecreateHashTable(HashTable *H);
// 算法9.18
// 查找不成功时插入数据元素e到开放定址哈希表H中，并返回1；
// 若冲突次数过大，则重建哈希表。
int InsertHash(HashTable *H,ElemType e);
// 按哈希地址的顺序遍历哈希表
void TraverseHash(HashTable H,void(*Vi)(int,ElemType));
// 在开放定址哈希表H中查找关键码为K的元素,若查找成功,以p指示待查数据
// 元素在表中位置,并返回SUCCESS;否则,返回UNSUCCESS
int Find(HashTable H,KeyType K,int *p);
void print(int p,ElemType r);



#endif
