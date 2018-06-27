#ifndef __CROSSLIST_C               
#define __CROSSLIST_C


#include "crosslist.h"

void input_matrix(CrossList* M)
{
	//printf("debug 0: in input_matrix\n");
	unsigned int k;	
	OLink	p,q,temp;
	FILE* infile=fopen("trans_matrix","r");
	//printf("debug 1: open file\n");
	fscanf(infile,"%u%u%u",&M->mu,&M->nu,&M->tu);
	//printf("debug 2: M->mu=%u,M->nu=%u,M->tu=%u\n",M->mu,M->nu,M->tu);
	//创建行链表头数组
	M->Rhead = (OLink*)malloc((M->mu)*sizeof(OLink));
	if(!M->Rhead)
		exit(-1);
	//创建列链表头数组
	M->Chead = (OLink*)malloc((M->nu)*sizeof(OLink));
	if(!M->Chead)
		exit(-1);
	for(k=0;k<M->mu;k++)
		M->Rhead[k]=NULL;
	for(k=0;k<M->nu;k++)
		M->Chead[k]=NULL;
	//printf("debug 3: allocate the M->Rhead M->Chead\n");
	//读取数据，并插入到十字联表中
	for(k=0;k<M->tu;k++)
	{
		p = (OLink)malloc(sizeof(OLNode));
		if(NULL == p)
		{
			printf("allocate memory failed!\n");
			exit(-1);
		}		
		fscanf(infile,"%u%u%lf",&(p->i),&(p->j),&(p->e));
		//printf("debug 4: p->i=%u,p->j=%u,p->e=%0.15lf\n",p->i,p->j,p->e);
		//插入行		
		if(NULL==M->Chead[p->i])
		{
			M->Chead[p->i]=p;
			p->right=NULL;
		}
		else
		{
			if(p->j<M->Chead[p->i]->j)
			{	
				temp = M->Chead[p->i];
				M->Chead[p->i]=p;
				p->right = temp;
			}
			else
			{	
				for(q=M->Chead[p->i]; q->right&&q->right->j<p->j;q=q->right);
				p->right=q->right;
				q->right=p;
			}
		}
		
		//插入列
		if(NULL==M->Rhead[p->j])
		{
			M->Rhead[p->j]=p;
			p->down=NULL;
		}
		else
		{
			if(p->i<M->Rhead[p->j]->i)
			{	
				temp = M->Rhead[p->j];
				M->Rhead[p->j]=p;
				p->down = temp;
			}
			else
			{
				for(q=M->Rhead[(p->j)];q->down&&q->down->i<p->i;q=q->down);
				p->down=q->down;
				q->down=p;
			}
		}
	}
	fclose(infile);
	//按行打印出十字联表
/*	for(k=0;k<M->mu;k++)
	{
		q=M->Chead[k];
		while(q)
		{
			printf("row= %u 	,col= %u	,e= %0.15lf\n",q->i,q->j,q->e);
			q=q->right;
		}
	}
*/
}

void free_matrix(CrossList* M)
{	//这个free函数只释放M的Rhead以及Chead所指向的空间，并不释放M本身的空间
	unsigned int i;	
	OLink p,q;
	if(!M->Chead||!M->Rhead)
		return;
	else
	{
		if(M->Rhead)
		{for(i=0;i<M->nu;i++)	M->Rhead[i]=NULL;}
		if(M->Chead)	//按行释放节点
		{
			for(i=0;i<M->mu;i++)
			{
				p=M->Chead[i];
				while(p)
				{q=p;	p=p->right;	free(q);}
			}	
		}
		free(M->Rhead);
		free(M->Chead);
	}

/*	unsigned int i;
	if(M)
	{
		for(i=0;i<M->mu;i++)
		{if(M->Chead[i])   free(M->Chead[i]);	M->Chead[i]=NULL;}	
		free(M->Chead);
		for(i=0;i<M->nu;i++)
		//{if(M->Rhead[i])   free(M->Rhead[i]);	M->Rhead[i]=NULL;}
			M->Rhead[i]=NULL;
		free(M->Rhead);
		M->mu=M->nu=M->tu=0;	M->Chead=M->Rhead=NULL;
		//free(M);
	}
*/
/*	if(M)
	{
		if(NULL!=M->Rhead)
		{ free(M->Rhead);  M->Rhead=NULL;}
		if(NULL!=M->Chead)
		{ free(M->Chead);  M->Chead=NULL;}
		M->mu = M->nu = M->tu =0;
	}
*/
}

void display_matrix(CrossList M)
{
	unsigned int k;
	OLink q;
	printf("M.mu=%u,M.nu=%u,M.tu=%u\n",M.mu,M.nu,M.tu);
	for(k=0;k<M.mu;k++)
	{
		q=M.Chead[k];
		while(q)
		{
			printf("%u	%u	%0.15lf\n",q->i+1,q->j+1,q->e);
			q=q->right;
		}
	}	
}

void multiply_matrix(CrossList M1,CrossList M2,CrossList& M3)
{
	unsigned int i,j,k;
	double e;
	OLink p,q,temp,q1,q2;
	//为M3申请空间并初始化
	M3.mu = M1.mu;
	M3.nu = M2.nu;
	M3.tu = 0;
	//创建行链表头数组
	M3.Chead = (OLink*)malloc((M3.mu)*sizeof(OLink));
	if(!M3.Chead)
		exit(-1);	
	//创建列链表头数组
	M3.Rhead = (OLink*)malloc((M3.nu)*sizeof(OLink));
	if(!M3.Rhead)
		exit(-1);
	
	for(k=0;k<M3.mu;k++)
		M3.Chead[k]=NULL;
	for(k=0;k<M3.nu;k++)
		M3.Rhead[k]=NULL;
/*	printf("debug: display M1 for mutiply\n");
	display_matrix(*M1);
	printf("debug: the distribution of M1");
	for(i=0;i<M1->mu;i++)
	{
		p=M1->Chead[i];
		printf("p=M1->Chead[%u]	  ",i);
		while(p)
		{printf("j=%u	",p->j);	p=p->right;}
		printf("\n");	
	}
	printf("debug: display M2 for mutiply\n");
	display_matrix(*M2);
	printf("debug: the distribution of M2");
	for(j=0;j<M2->nu;j++)
	{
		q=M2->Rhead[j];
		printf("q=Rhead[%u]	",j);
		while(q)
		{printf("i=%u	",q->i);	q=q->down;}
		printf("\n");
	}
*/
	for(i=0;i<M1.mu;i++)	//for1
	{
		for(j=0;j<M2.nu;j++)	//for2
		{
			e=0;
			p = M1.Chead[i];
			q = M2.Rhead[j];
			while(p&&q)
			{
				//printf("debug2 : p->j=%u,q->i=%u\n",p->j,q->i);
				if(p->j==q->i)
				{
					//printf("debug:  computing %0.15lf * %0.15lf .......\n",p->e,q->e);
					e = e + p->e*q->e;
					//printf("debug:	e=%0.15f\n",e);							
					p = p->right;
					q = q->down;
				}
				else if(p->j<q->i)
					p = p->right;
				else
					q = q->down;
			}
			//printf("debug: M3[%u][%u]=%0.15lf\n",i,j,e);
			//if(e!=0)	//向M3 中插入新元素
			//×××××××××××注意此处的判断×××××××*****
			if(e>=0.000000000000001 || e<=-0.000000000000001)	//15位
			{
				//printf("debug:	start insert...\n");
				//printf("debug:	insert i=%u  j=%u  e=%0.15lf\n",i,j,e);
				p = (OLink)malloc(sizeof(OLNode));
				if(NULL == p)
				{
					printf("allocate memory failed!\n");
					exit(-1);
				}		
				p->i = i;
				p->j = j;
				p->e = e;	
				
				//printf("debug:	insert row...\n");
				//插入行		
				if(NULL==M3.Chead[i]||p->j<M3.Chead[i]->j)
				{
					p->right = M3.Chead[i];
					M3.Chead[i]=p;
				}
				else
				{
					q1=q2=M3.Chead[i];
					while(q2&&q2->j<p->j)
					{q1=q2;	q2=q2->right;}
					p->right=q2;
					q1->right=p;
				}
				//printf("debug:	insert row okay\n");
				//printf("debug:	insert col....\n");
				//插入列
				if(NULL==M3.Rhead[j]||p->i<M3.Rhead[j]->i)
				{
					p->down = M3.Rhead[j];
					M3.Rhead[j]=p;
				}
				else
				{
					q1=q2=M3.Rhead[j];
					while(q2&&q2->i<p->i)
					{q1=q2;	q2=q2->down;}
					p->down=q2;	
					q1->down=p;
				}
				M3.tu++;
				//printf("debug:	insert col okay\n");
				//printf("debug:  after this insert , the matrix is :\n");
				//display_matrix(*M3);
			}//if
		}//for2
	}//for1
}
/*
bool compute_order(CrossList& M)
{	//求一个转移概率矩阵迭代稳定后的排序情况(第一行的数据不再发生变化)
	CrossList M2;	
	unsigned int k;
	OLink p,q;
	for(int i=1;i<=100;i++)	//最多迭代一百次就停止
	{
		multiply_matrix(M,M,&M2);
		printf("the %dst iterative :.....\n",i);
		display_matrix(M2);
		p = M.Chead[0];
		q = M2.Chead[0];
		//判断M和M2第0行是否相等或者只相差误差10的-15次方
		while(p&&q)
		{
			if(p->j==q->j && p->e-q->e>=-0.000000000000001 && p->e-q->e<=0.000000000000001)
			{
				p=p->right;	q=q->right;	continue;
			}
			else if(p->j<q->j && p->e<=0.000000000000001)
			{
				p=p->right;	continue;
			}
			else if(q->j<p->j && q->e<=0.000000000000001)
			{
				q=q->right;	continue;
			}
			else
				break;
		}
		if(NULL==p && NULL==q)	//M和M2第0行相等或者只相差误差10的-15次方
		{//将M2输出到文件final_matrix中
			printf("\ntotal iterative: %d\n final_matrix:\n",i);
			display_matrix(M2);
			//将稳定后的转移概率矩阵导出到文件final_matrix中
			FILE *outfile=fopen("final_matrix","w");
			fprintf(outfile,"%u	%u	%u\n",M2.mu,M2.nu,M2.tu);
			for(k=0;k<M2.mu;k++)
			{
				q=M.Chead[k];
				while(q)
				{
					fprintf(outfile,"%u	%u	%0.15lf\n",q->i,q->j,q->e);
					q=q->right;
				}
			}
			fclose(outfile);
			return true;
		}
		else if(NULL==p && NULL!=q)
		{
			while(q)
			{
				if(q->e<=0.00000000000001)	//14位
				{q=q->right;	continue;}
				else
					break;
			}
			if(NULL==q)
			{
				//将M2输出到文件final_matrix中
				printf("\ntotal iterative: %d\n final_matrix:\n",i);
				display_matrix(M2);
				//将稳定后的转移概率矩阵导出到文件final_matrix中
				FILE *outfile=fopen("final_matrix","w");
				fprintf(outfile,"%u	%u	%u\n",M2.mu,M2.nu,M2.tu);
				for(k=0;k<M2.mu;k++)
				{
					q=M2.Chead[k];
					while(q)
					{
						fprintf(outfile,"%u	%u	%0.15lf\n",q->i,q->j,q->e);
						q=q->right;
					}
				}
				fclose(outfile);
				return true;
			}
		}
		else if(NULL==q && NULL!=p)
		{
			while(p)
			{
				if(p->e<=0.00000000000001)	//14位精度
				{p=p->right;	continue;}
				else
					break;
			}
			if(NULL==p)
			{
				//将M2输出到文件final_matrix中
				printf("\ntotal iterative: %d\n final_matrix:\n",i);
				display_matrix(M2);
				//将稳定后的转移概率矩阵导出到文件final_matrix中
				FILE *outfile=fopen("final_matrix","w");
				fprintf(outfile,"%u	%u	%u\n",M2.mu,M2.nu,M2.tu);
				for(k=0;k<M2.mu;k++)
				{
					q=M2.Chead[k];
					while(q)
					{
						fprintf(outfile,"%u	%u	%0.15lf\n",q->i,q->j,q->e);
						q=q->right;
					}
				}
				fclose(outfile);
				return true;
			}
		}
		else ;	//做下一次矩阵相乘迭代
		error*****这样删除M有问题***************
		for(k=0;k<M.mu;k++)
			free(M.Chead[k]);
		free(M.Chead);	
		for(k=0;k<M.nu;k++)
			free(M.Rhead[k]);		
		free(M.Rhead);
		M.mu = M2.mu;	M.nu = M2.nu;	M.tu = M2.tu;
		M.Rhead = M2.Rhead;	M.Chead = M2.Chead;
		M2.mu = 0;	M2.nu = 0;	M2.tu = 0;
		M2.Rhead = NULL;	M2.Chead = NULL;	
	}//end for 100
}
*/
//将转移矩阵M1复制给转移矩阵M2
bool copy_matrix(CrossList M1,CrossList& M2)
{
	unsigned int k;	
	//free_matrix(&M2);	//首先将M2清空
	OLink q1,q2,temp;
	M2.mu=M1.mu;	M2.nu=M1.nu;	M2.tu=M1.tu;
	//创建列链表头数组
	M2.Rhead = (OLink*)malloc((M2.mu)*sizeof(OLink));
	if(!M2.Rhead)
	{
		printf("mallocate memory in copy_matrix failed!\n");
		exit(-1);	
	}
	for(k=0;k<M2.mu;k++)
		M2.Rhead[k]=NULL;
	//创建行联表头数组
	M2.Chead = (OLink*)malloc((M2.nu)*sizeof(OLink));
	if(!M2.Chead)
	{
		printf("mallocate memory in copy_matrix failed!\n");
		exit(-1);	
	}
	for(k=0;k<M2.nu;k++)
		M2.Chead[k]=NULL;
	
	for(k=0;k<M1.mu;k++)	
	{
		q1=M1.Chead[k];
		while(q1)
		{
			q2=(OLink)malloc(sizeof(OLNode));
			q2->i=q1->i;
			q2->j=q1->j;
			q2->e=q1->e;		
			//插入行
			if(NULL==M2.Chead[q2->i])
			{
				M2.Chead[q2->i]=q2;
				q2->right=NULL;
			}
			else
			{
				if(q2->j<M2.Chead[q2->i]->j)
				{
					temp = M2.Chead[q2->i];
					M2.Chead[q2->i]=q2;
					q2->right=temp;
				}
				else
				{
					for(temp=M2.Chead[q2->i];temp->right&&temp->right->j<q2->j;temp=temp->right);
					q2->right=temp->right;
					temp->right=q2;
				}
			}
			//插入列
			if(NULL==M2.Rhead[q2->j])
			{
				M2.Rhead[q2->j]=q2;
				q2->down=NULL;
			}
			else
			{
				if(q2->i<M2.Rhead[q2->j]->i)
				{
					temp=M2.Rhead[q2->j];
					M2.Rhead[q2->j]=q2;
					q2->down=temp;
				}
				else
				{
					for(temp=M2.Rhead[q2->j];temp->down&&temp->down->i<q2->i;temp=temp->down);
					q2->down=temp->down;
					temp->down=q2;
				}
			}
			q1=q1->right;
		}//end while
	}//end for

	return true;
}

//将矩阵M加上单位矩阵I，结果存放在M中
bool add_matrixI(CrossList& M)
{
	OLink tempp,p1,p2;
	unsigned int i;
	if(M.mu!=M.nu)
		return false;
	for(i=0;i<M.mu;i++)
	{
		if(M.Chead[i]==NULL||M.Chead[i]->j>i)
		{
			tempp = (OLink)malloc(sizeof(OLNode));	
			tempp->i = tempp->j = i;	tempp->e = 1;	//插入行
			tempp->right=M.Chead[i];	M.Chead[i]=tempp;
			if(M.Rhead[i]==NULL||M.Rhead[i]->i>i)	//插入列
			{tempp->down = M.Rhead[i];	M.Rhead[i]=tempp;}
			else
			{
				p1=p2=M.Rhead[i];	
				while(p2&&p2->i<i)
				{p1=p2;	p2=p2->down;}
				p1->down=tempp;
				tempp->down=p2;
			}
			M.tu++;
		}
		else
		{
			p1=p2=M.Chead[i];
			while(p2&&p2->j<i)
			{p1=p2;	p2=p2->right;}
			if(p2&&p2->j==i)
			{p2->e=p2->e+1;	continue;}
			else
			{
				tempp = (OLink)malloc(sizeof(OLNode));
				tempp->i = tempp->j =i;	 tempp->e=1;	//插入行
				tempp->right=p2;	p1->right=tempp;
				if(M.Rhead[i]==NULL||M.Rhead[i]->i>i)	//插入列
				{tempp->down = M.Rhead[i];	M.Rhead[i]=tempp;}
				else
				{
					p1=p2=M.Rhead[i];
					while(p2&&p2->i<i)
					{p1=p2;	p2=p2->down;}
					p1->down=tempp;
					tempp->down=p2;
				}
				M.tu++;
			}
		}
	}
	return true;
}

//判断两个矩阵是否相等
bool equal_matrix(CrossList M1,CrossList M2)
{
	unsigned int i;
	OLink p,q;
	if(M1.mu!=M2.mu || M1.nu!=M2.nu || M1.tu!=M2.tu)
		return false;
	for(i=0;i<M1.mu;i++)
	{
		p=M1.Chead[i];	q=M2.Chead[i];
		while(p&&q)
		{
			if(p->e-q->e>=0.000000000000001 || q->e-p->e>=0.000000000000001)
				return false;
			p=p->right;	q=q->right;
		}
		if(p||q)
		   return false;
	}
	return true;
}

//将矩阵M导出到文件filename中(注意导出的矩阵行列坐标都从1开始)
bool output_matrix(CrossList M,char* filename)
{
	int k;
	OLink q;
	FILE *outfile=fopen(filename,"w");
	if(!outfile)
	{printf("\nopen file %s error!",filename);  exit(-1);}
	fprintf(outfile,"%u	%u	%u\n",M.mu,M.nu,M.tu);
	for(k=0;k<M.mu;k++)
	{
		q=M.Chead[k];
		while(q)
		{
			fprintf(outfile,"%u	%u	%0.15lf\n",q->i+1,q->j+1,q->e);
			q=q->right;
		}
	}
	fclose(outfile);
	return true;
}

//将文件filename中的矩阵导入到M中，文件中的矩阵行列值都是从1开始的
bool input_matrix(CrossList& M,char* filename)
{
	unsigned int k,num=0;	
	OLink	p,q,temp;
	//free_matrix(&M);	//首先M占用的空间
	FILE* infile=fopen(filename,"r");
	if(!infile)	{printf("\nopen infile %s error!",filename); exit(-1);}
	//fscanf(infile,"%u%u%u",&M->mu,&M->nu,&M->tu);
	fscanf(infile,"%u%u%u",&M.mu,&M.nu,&M.tu);
	//printf("matrix in %s mu=%u,nu=%u,tu=%u\n",filename,M.mu,M.nu,M.tu);
	//创建行链表头数组
	M.Chead = (OLink*)malloc((M.mu)*sizeof(OLink));
	if(!M.Chead)	exit(-1);
	//创建列链表头数组
	M.Rhead = (OLink*)malloc((M.nu)*sizeof(OLink));
	if(!M.Rhead)	exit(-1);
	for(k=0;k<M.mu;k++)	M.Chead[k]=NULL;
	for(k=0;k<M.nu;k++)	M.Rhead[k]=NULL;
	//读取数据，并插入到十字联表中	
	p = (OLink)malloc(sizeof(OLNode));
	if(NULL == p)  {  printf("allocate memory failed!\n");   exit(-1); }	
	while(fscanf(infile,"%u%u%lf",&(p->i),&(p->j),&(p->e))!=EOF)	//注意feof函数在读完文件所有内容后再读取下一个字符时才会返回非0
	{
		p->i=p->i-1;	p->j=p->j-1;	//行列值各减去1
		//插入行		
		if(NULL==M.Chead[p->i])
		{
			M.Chead[p->i]=p;
			p->right=NULL;
		}
		else
		{
			if(p->j<M.Chead[p->i]->j)
			{	
				temp = M.Chead[p->i];
				M.Chead[p->i]=p;
				p->right = temp;
			}
			else
			{	
				temp = q = M.Chead[p->i];
				while(q&&q->j<p->j)
				{temp=q;	q=q->right;}
				temp->right=p;	p->right=q;
			}
		}
		
		//插入列
		if(NULL==M.Rhead[p->j])
		{
			M.Rhead[p->j]=p;
			p->down=NULL;
		}
		else
		{
			if(p->i<M.Rhead[p->j]->i)
			{	
				temp = M.Rhead[p->j];
				M.Rhead[p->j]=p;
				p->down = temp;
			}
			else
			{
				temp = q = M.Rhead[p->j];
				while(q&&q->i<p->i)
				{temp=q;	q=q->down;}
				temp->down=p;	p->down=q;	
			}
		}
		num++;
		//printf("num = %d   p->i=%u   p->j=%u   p->e=%0.15lf\n",num,p->i,p->j,p->e);
		p = (OLink)malloc(sizeof(OLNode));
		if(NULL == p)  {  printf("allocate memory failed!\n");   exit(-1); }	
	}
	M.tu=num;
	fclose(infile);
	//按行打印出十字联表
/*	for(k=0;k<M->mu;k++)
	{
		q=M->Chead[k];
		while(q)
		{
			printf("row= %u 	,col= %u	,e= %0.15lf\n",q->i,q->j,q->e);
			q=q->right;
		}
	}
*/
}

#endif /*__CROSSLIST_C*/
