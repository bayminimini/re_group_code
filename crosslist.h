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

//导入DFA状态转移矩阵
void input_matrix(CrossList* M);
//释放十字联表
void free_matrix(CrossList* M);
//显示十字联表
void display_matrix(CrossList M);
//求两个十字联表表示的稀疏矩阵的乘法
void multiply_matrix(CrossList M1,CrossList M2,CrossList& M3);
//将矩阵M加上单位矩阵I，结果存放在M中
bool add_matrixI(CrossList& M);
//求一个转移概率矩阵迭代稳定后的排序情况
bool compute_order(CrossList& M);
//将转移矩阵M1复制给转移矩阵M2
bool copy_matrix(CrossList M1,CrossList& M2);
//判断两个矩阵是否相等
bool equal_matrix(CrossList M1,CrossList M2);
//将矩阵M导出到文件filename中(导出矩阵的行列值都从1开始)
bool output_matrix(CrossList M,char* filename);
//将文件filename中的矩阵导入到M中，文件中的矩阵行列值都是从1开始的
bool input_matrix(CrossList& M,char* filename);

#endif /*__CROSSLIST_H*/
