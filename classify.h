
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
	set<state_t> statesSet;	//״̬����
	unsigned int num;	//״̬��Ŀ
	unsigned int id;	//״̬�����
}recurrent_class;
*/
typedef list< set<state_t> > myRecuList;

//����ת�Ƹ��ʾ���M��ʮ������洢���������״̬s��ǰ�򼯺�forward_set
state_set compute_forset(CrossList M,state_t s);
//����ת�Ƹ��ʾ���M��ʮ������洢���������״̬s��ǰ�򼯺�backward_set
state_set compute_backset(CrossList M,state_t s);
//����ת�Ƹ��ʾ��������״̬���Ϻͱպ�̬�ļ���
bool State_classification(CrossList M,set<state_t>& transSet,myRecuList& reculist);
//�Թ���״̬�����Լ��պ�̬���Ͻ���ͳ��
bool State_statistic(CrossList M,set<state_t> transSet,myRecuList reculist);
//ͨ�����Ե�λ����ķ�������ת�Ƹ��ʾ����е�״̬����֤ͬһ�������ڵ�Ԫ�ش������ڵ�λ��(M2�ǽ�����ľ���,state_order���潻�����״̬˳��)
bool State_swap(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order);
//ͨ������ָ��ķ�������ת�Ƹ��ʾ����е�״̬����֤ͬһ�������ڵ�Ԫ�ش������ڵ�λ��(M2�ǽ�����ľ���,state_order���潻�����״̬˳��,T�Ǵ�M2�и��Ƴ����Ĺ���״̬����)
bool State_swap2(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order,CrossList& T);
//������״̬����T��M2�и��Ƴ���
bool Copy_T(CrossList M2,set<state_t> transSet, CrossList &T);
//����(I-T)�������TI
bool inv_T(CrossList T,CrossList& TI);
//�������M2����̬����SM,M2���Ѿ��ź���ľ���reculist��M2�ı�̬����T��M2�Ĺ��ɾ���TI��(I-T)����
bool steady_matrix(CrossList M2, set<state_t> transSet,myRecuList reculist, CrossList TI, CrossList& SM);
//������ʼ״̬ת�Ƹ��ʾ���M�����������̬ʱ������̬�ĸ������򣬴Ӵ���С����final_order��
bool final_steady_order(CrossList& M,vector<unsigned int>& final_order,vector<double>& steady_prob);
#endif























