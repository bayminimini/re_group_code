#ifndef __CROSSLIST_H               
#define __CROSSLIST_H

#include "stdinc.h"

typedef struct OLNode{
  unsigned int i,j;
  double e;
  struct OLNode *right,*down;
}OLNode, *OLink;

typedef struct{
  OLink *Rhead,*Chead;
  unsigned int mu,nu,tu;
}CrossList;

//����DFA״̬ת�ƾ���
void input_matrix(CrossList* M);
//�ͷ�ʮ������
void free_matrix(CrossList* M);
//��ʾʮ������
void display_matrix(CrossList M);
//������ʮ�������ʾ��ϡ�����ĳ˷�
void multiply_matrix(CrossList M1,CrossList M2,CrossList& M3);
//������M���ϵ�λ����I����������M��
bool add_matrixI(CrossList& M);
//��һ��ת�Ƹ��ʾ�������ȶ�����������
bool compute_order(CrossList& M);
//��ת�ƾ���M1���Ƹ�ת�ƾ���M2
bool copy_matrix(CrossList M1,CrossList& M2);
//�ж����������Ƿ����
bool equal_matrix(CrossList M1,CrossList M2);
//������M�������ļ�filename��(�������������ֵ����1��ʼ)
bool output_matrix(CrossList M,char* filename);
//���ļ�filename�еľ����뵽M�У��ļ��еľ�������ֵ���Ǵ�1��ʼ��
bool input_matrix(CrossList& M,char* filename);

#endif /*__CROSSLIST_H*/
