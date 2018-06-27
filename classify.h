
#ifndef _CLASSIFY_H
#define _CLASSIFY_H

//#include "crosslist.h"

#include "dfa.h"
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;
/*
typedef struct{
	set<state_t> statesSet;	//状态集合
	unsigned int num;	//状态数目
	unsigned int id;	//状态集编号
}recurrent_class;
*/
typedef list< set<state_t> > myRecuList;

//根据转移概率矩阵M（十字联表存储），求给定状态s的前向集合forward_set
state_set compute_forset(CrossList M,state_t s);
//根据转移概率矩阵M（十字联表存储），求给定状态s的前向集合backward_set
state_set compute_backset(CrossList M,state_t s);
//根据转移概率矩阵求过渡状态集合和闭合态的集合
bool State_classification(CrossList M,set<state_t>& transSet,myRecuList& reculist);
//对过渡状态集合以及闭合态集合进行统计
bool State_statistic(CrossList M,set<state_t> transSet,myRecuList reculist);
//通过乘以单位矩阵的方法交换转移概率矩阵中的状态，保证同一个集合内的元素处于相邻的位置(M2是交换后的矩阵,state_order保存交换后的状态顺序)
bool State_swap(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order);
//通过交换指针的方法交换转移概率矩阵中的状态，保证同一个集合内的元素处于相邻的位置(M2是交换后的矩阵,state_order保存交换后的状态顺序,T是从M2中复制出来的过渡状态矩阵)
bool State_swap2(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order,CrossList& T);
//将过渡状态方阵T从M2中复制出来
bool Copy_T(CrossList M2,set<state_t> transSet, CrossList &T);
//计算(I-T)的逆矩阵TI
bool inv_T(CrossList T,CrossList& TI);
//计算矩阵M2的稳态矩阵SM,M2是已经排好序的矩阵，reculist是M2的闭态集，T是M2的过渡矩阵，TI是(I-T)的逆
bool steady_matrix(CrossList M2, set<state_t> transSet,myRecuList reculist, CrossList TI, CrossList& SM);
//给定初始状态转移概率矩阵M，求解其在稳态时各个闭态的概率排序，从大至小存至final_order中
bool final_steady_order(CrossList& M,vector<unsigned int>& final_order,vector<double>& steady_prob);
#endif























