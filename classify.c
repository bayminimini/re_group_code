
#ifndef _CLASSIFY_C
#define _CLASSIFY_C

//#include "stdinc.h"
#include "classify.h"
#include <time.h>

//����ת�Ƹ��ʾ���M��ʮ������洢���������״̬s��ǰ�򼯺�forward_set
state_set compute_forset(CrossList M,state_t s)
{
	//set��state_t��::iterator si;
	state_set::iterator si;
	state_set RS,FS0,FS1;	//��Щset�����ڵ�����ʱ��ʹ��
	RS.clear();
	FS0.clear();	FS1.clear();
	FS0.insert(s);
	while(!FS0.empty())
	{
		for(si=FS0.begin();si!=FS0.end();si++)
		{//��FS0��ÿ��Ԫ�أ�������һ��ת��״̬���ϣ����뵽FS1��
			OLNode* temp = M.Chead[*si];
			while(temp)
			{
				FS1.insert(temp->j);
				temp = temp->right;
			} 
		}
		FS0.clear();	//��FS0���
		//��FS1������ɾ��RS�е�Ԫ��,����ŵ�FS0��
		set_difference(FS1.begin(),FS1.end(),RS.begin(),RS.end(),insert_iterator<set<state_t> >(FS0,FS0.begin()));
		FS1.clear();
		for(si=FS0.begin();si!=FS0.end();si++)
			RS.insert(*si);
	}
/*	cout<<endl<<"״̬"<<s<<"��ǰ�򼯺���:	"<<endl;
	for(si=RS.begin();si!=RS.end();si++)
		cout<<*si<<'	';
	cout<<endl;
*/	return RS;
}


//����ת�Ƹ��ʾ���M��ʮ������洢���������״̬s��ǰ�򼯺�backward_set
state_set compute_backset(CrossList M,state_t s)
{
	state_set::iterator si;
	state_set RS,FS0,FS1;	//��Щset�����ڵ�����ʱ��ʹ��
	RS.clear();
	FS0.clear();	FS1.clear();
	FS0.insert(s);
	while(!FS0.empty())
	{
		for(si=FS0.begin();si!=FS0.end();si++)
		{//��FS0��ÿ��Ԫ�أ�������һ��ת��״̬���ϣ����뵽FS1��
			OLNode* temp = M.Rhead[*si];
			while(temp)
			{
				FS1.insert(temp->i);
				temp = temp->down;
			} 
		}
		FS0.clear();	//��FS0���
		//��FS1������ɾ��RS�е�Ԫ��,����ŵ�FS0��
		set_difference(FS1.begin(),FS1.end(),RS.begin(),RS.end(),insert_iterator<set<state_t> >(FS0,FS0.begin()));
		FS1.clear();
		for(si=FS0.begin();si!=FS0.end();si++)
			RS.insert(*si);
	}
/*	cout<<endl<<"״̬"<<s<<"�ĺ��򼯺���:	"<<endl;
	for(si=RS.begin();si!=RS.end();si++)
		cout<<*si<<'	';
	cout<<endl;
*/	return RS;
}


//����ת�Ƹ��ʾ��������״̬���Ϻͱպ�̬�ļ���
bool State_classification(CrossList M,set<state_t>& transSet,myRecuList& reculist)
{
	set<state_t> myset;
	state_set FS,BS,TS;
	state_t s;
	int i;
	set<state_t>::iterator it;
	for(i=0;i<M.mu;i++)
		myset.insert(i);
	//srand(0);
	while(!myset.empty())
	{
		//s = rand()%(myset.size());
		it = myset.begin();
		s = *it;
		FS = compute_forset(M,s);
		BS = compute_backset(M,s);
		if(BS.size()>=FS.size())  //FS ������BS �Ӽ���
		{		
			set_intersection(FS.begin(),FS.end(),BS.begin(),BS.end(),insert_iterator<set<state_t> >(TS,TS.begin()));
			if(TS.size()==FS.size())	//FS��BS�Ӽ���
			{//��ʱFS��һ���ռ��ϣ�BS��FS�����Ľ����ǹ���״̬��
				reculist.push_back(FS);
				TS.clear();
				set_difference(BS.begin(),BS.end(),FS.begin(),FS.end(),insert_iterator<set<state_t> >(TS,TS.begin()));
				transSet.insert(TS.begin(),TS.end());
			}				
			else	//FS����BS �Ӽ���
			{//��ʱS �Լ�BS �ǹ���״̬
				transSet.insert(s);
				transSet.insert(BS.begin(),BS.end());
			}				
		}
		else	//FS �϶�����BS �Ӽ���
		{
			transSet.insert(s);
			transSet.insert(BS.begin(),BS.end());			
		}
		myset.erase(s);
		for(it=BS.begin();it!=BS.end();it++)
			myset.erase(*it);
		FS.clear();	BS.clear();	TS.clear();
	}
	return true;
}

//�Թ���״̬�����Լ��պ�̬���Ͻ���ͳ��
bool State_statistic(CrossList M,set<state_t> transSet,myRecuList reculist)
{
	set<state_t>::iterator it;
	list< set<state_t> >::iterator it2;
/*	cout<<endl<<endl<<"		the trans states :	"<<endl;
	for(it=transSet.begin();it!=transSet.end();it++)
		cout<<*it<<'	';
*/	cout<<endl<<endl<<"		the recurrent states :	"<<endl;
	int i=1;
	for(it2=reculist.begin();it2!=reculist.end();it2++)
	{
		cout<<"class "<<i<<endl;
		for(it=(*it2).begin();it!=(*it2).end();it++)
			cout<<*it<<'	';
		cout<<endl;
		i++;
	}	

	//�Թ���״̬���Ϻͱպ�̬���Ͻ���ͳ��
	cout<<endl<<endl<<"the total states "<<M.mu<<endl;
	cout<<"the states in transient class "<<transSet.size()<<endl;	
	unsigned int min,max;	
	min = M.mu;		
	max = 0;
	for(it2=reculist.begin();it2!=reculist.end();it2++)
	{
		if((*it2).size()<min)
			min = (*it2).size();
		if((*it2).size()>max)
			max = (*it2).size();
	}	
	cout<<"the recurrent class num is "<<reculist.size()<<endl;
	cout<<"the max recurrent class size is "<<max<<endl;
	cout<<"the min recurrent class size is "<<min<<endl;	
	cout<<"the average recurrent class size is "<<(M.mu-transSet.size())/(reculist.size())<<endl<<endl;	

	return true;
}
//ͨ�����Ե�λ����ķ��������ž���
bool State_swap(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order)
{	//M2Ϊ���յõ��İ��ռ��͹��ɼ������еľ���state_orderΪ����״̬��M2�е�λ��
	unsigned int i;	
	unsigned int j=1;
	vector<unsigned int>::iterator state_pt,state_it;
	set<state_t>::iterator it;
	list< set<state_t> >::iterator it2;
	state_t state1,state2;	//��¼���ε�λת������M3��λ��
	state_t last_state1,last_state2; //��¼��һ�εĵ�λת������M3��λ��
	last_state1 = last_state2 =0;
	CrossList M3,M4,M5;    //M3Ϊת������M4Ϊ�������
	OLink p,q;
	clock_t start,finish;
	unsigned int duration;
	copy_matrix(M,M2);	//���Ƚ�M���Ƹ�M2
	for(i=0;i<M2.mu;i++)	//��ʼ״̬˳��
		state_order.push_back(i);
	//����ת������M3����ʼʱΪ��λ���󣬺������ʹ�õ���Ҫ���е���
	M3.mu=M2.mu;	M3.nu=M2.nu;	M3.tu=M2.mu;
	//����M3������ͷ����
	M3.Rhead = (OLink*)malloc((M3.mu)*sizeof(OLink));
	if(!M3.Rhead)
	{
		printf("mallocate memory in State_swap failed!\n");
		exit(-1);	
	}
	for(i=0;i<M3.mu;i++)
		M3.Rhead[i]=NULL;
	//����M3������ͷ����
	M3.Chead = (OLink*)malloc((M3.nu)*sizeof(OLink));
	if(!M3.Chead)
	{
		printf("mallocate memory in State_swap failed!\n");
		exit(-1);	
	}
	for(i=0;i<M3.nu;i++)
		M3.Chead[i]=NULL;
	for(i=0;i<M3.mu;i++)  //��λ����
	{
		p=(OLink)malloc(sizeof(OLNode));
		if(!p)
		{
			printf("mallocate memory in State_swap failed!\n");
			exit(-1);	
		}	
		p->i=i;	p->j=i;	p->e=1;
		p->right=NULL;	p->down=NULL;
		M3.Chead[i]=p;	M3.Rhead[i]=p;
	}	
	
	state_pt = state_order.begin();	//state_ptָʾ��ǰ��������λ��
	start=clock();
	for(it2=reculist.begin();it2!=reculist.end();it2++)//for it2
	{
		for(it=(*it2).begin();it!=(*it2).end();it++)//for it
		{//itָ��ǰҪ�����ıպ�״̬��	
			//�����ҵ�itָ���״̬��state_order�е�λ��(����ɨ��state_order)
			for(state_it=state_pt;state_it!=state_order.end();state_it++)
			{
				if((*it)==(*state_it))
					break;
			}	
			if(state_it==state_pt)	//������Ԫ�ص�λ�þ���state_pt,��ʱ����Ҫ����
			{
				state_pt++;
				continue;	//�����˴�ѭ����������һ��ѭ��
			}
			printf("\nthe %ldst iterative swap...\n",j);
			//�����state_ptָ���״̬��state_itָ��״̬���н�������Ӧ�ı�M2����
			state1 = state_pt-state_order.begin();	//�õ�Ҫ����������λ��
			state2 = state_it-state_order.begin();
			
			//����ת������M3
			//����Ҫ��M3�ָ��ɵ�λ����
			if(last_state1==last_state2) ;	//��Ե�һ�Σ����ûָ�
			else
			{	//��M3�ָ��ɵ�λ����
				M3.Chead[last_state1]->j=last_state1; M3.Rhead[last_state1]=M3.Chead[last_state1];	 
				M3.Chead[last_state2]->j=last_state2; M3.Rhead[last_state2]=M3.Chead[last_state2];
			}
			last_state1 = state1;	last_state2 = state2;
			//�õ�ת������M3
			M3.Chead[state1]->j=state2;	M3.Rhead[state2]=M3.Chead[state1];
			M3.Chead[state2]->j=state1;	M3.Rhead[state1]=M3.Chead[state2];	
			
			multiply_matrix(M3,M2,M4);
			free_matrix(&M2);
			multiply_matrix(M4,M3,M2);	//�õ������Ľ��M2
			free_matrix(&M4);
					
			i=*state_it;	*state_it=*state_pt;	*state_pt=i;//����state_pt��state_it��ָ���Ԫ��
			state_pt++;	//����state_pt��λ��

			j++;
		}//end for it
	}//end for it2
	finish=clock();
	duration=(int)(finish-start)/CLOCKS_PER_SEC;
	printf("\n swap matrix cost time :%ld seconds\n",duration);
	//����ټ���һ�齻����ľ����Ƿ񰴱ռ�����
	state_pt = state_order.begin();
	for(it2=reculist.begin();it2!=reculist.end();it2++)
	{
		for(it=(*it2).begin();it!=(*it2).end();it++)
		{
			if((*it)!=(*state_pt))
			{	
				printf("CHECK ERROR: the final swaped matrix is wrong!");
				return false;
			}
			state_pt++;
		}
	}
	cout<<endl<<"the state order in the swaped matrix:"<<endl;
	for(state_pt=state_order.begin();state_pt!=state_order.end();state_pt++)
		cout<<*state_pt<<'	';
	//cout<<endl<<"the  final swaped matrix :"<<endl;
	//display_matrix(M2);
	return true;
}
//ͨ������ָ��ķ��������ž��󣬶����б任����ɾ����������Ҫ�������У�Ȼ���ٽ������зֱ����ʮ������;�б任ͬ��
bool State_swap2(CrossList M,CrossList& M2,set<state_t> transSet,myRecuList reculist,vector<unsigned int>& state_order,CrossList& T)
{	//M2Ϊ���յõ��İ��ռ��͹��ɼ������еľ���state_orderΪ����״̬��M2�е�λ��
	unsigned int i;	
	unsigned int j=1;
	vector<unsigned int>::iterator state_pt,state_it;
	set<state_t>::iterator it;
	list< set<state_t> >::iterator it2;
	state_t state1,state2;	//��¼������Ҫ����������״̬
	OLink link1,link2,p,q1,q2;
	clock_t start,finish;
	unsigned int duration,tempd;
	unsigned int recuSize=0;
	double tempe;
	copy_matrix(M,M2);	//���Ƚ�M���Ƹ�M2
	for(i=0;i<M2.mu;i++)	//��ʼ״̬˳��
		state_order.push_back(i);

	//cout<<endl<<"the  initial matrix :"<<endl;
	//display_matrix(M2);	
	
	state_pt = state_order.begin();	//state_ptָʾ��ǰ��������λ��
	start=clock();
	for(it2=reculist.begin();it2!=reculist.end();it2++)//for it2
	{
		for(it=(*it2).begin();it!=(*it2).end();it++)//for it
		{//itָ��ǰҪ�����ıպ�״̬��	
			//�����ҵ�itָ���״̬��state_order�е�λ��(����ɨ��state_order)
			//printf("\nthe %ldst iterative swap...\n",j);
			for(state_it=state_pt;state_it!=state_order.end();state_it++)
			{
				if((*it)==(*state_it))
					break;
			}	
			if(state_it==state_pt)	//������Ԫ�ص�λ�þ���state_pt,��ʱ����Ҫ����
			{
				state_pt++;
				continue;	//�����˴�ѭ����������һ��ѭ��
			}
			//�����state_ptָ���״̬��state_itָ��״̬���н�������Ӧ�ı�M2����
			state1 = state_pt-state_order.begin();	//�õ�Ҫ����������λ��(state1<state2)
			state2 = state_it-state_order.begin();
			//���������������������������������������������Ƚ����б任����������������������������������������������
			link1=M2.Chead[state1];
			M2.Chead[state1]=NULL;
			p=link1;
			while(p)	//��������ɾ��M2->Chead[state1]
			{
				if(M2.Rhead[p->j]->i==state1)
				{ 	
					M2.Rhead[p->j]=M2.Rhead[p->j]->down;
					p=p->right;
					continue;					
				}
				q1=q2=M2.Rhead[p->j];
				while(q2->i!=state1)
				{q1=q2;	q2=q2->down;}
				q1->down=q2->down;
				p=p->right;
			}		
			link2=M2.Chead[state2];
			M2.Chead[state2]=NULL;
			p=link2;
			while(p)	//��������ɾ��M2->Chead[state2]
			{
				if(M2.Rhead[p->j]->i==state2)
				{
					M2.Rhead[p->j]=M2.Rhead[p->j]->down;
					p=p->right;
					continue;
				}
				q1=q2=M2.Rhead[p->j];
				while(q2->i!=state2)
				{q1=q2;	q2=q2->down;}
				q1->down=q2->down;
				p=p->right;
			}
			//�޸������������ֵ
			p=link1;
			while(p)
			{p->i=state2;	p=p->right;}
			p=link2;
			while(p)
			{p->i=state1;	p=p->right;}				
			//������������뵽ʮ��������
			M2.Chead[state1]=link2;
			p=link2;
			while(p) //����link2
			{
				if(M2.Rhead[p->j]==NULL)
				{M2.Rhead[p->j]=p;	p->down=NULL;	p=p->right;	continue;}
				q1=q2=M2.Rhead[p->j];
				if(M2.Rhead[p->j]->i>state1)
				{p->down=M2.Rhead[p->j];   M2.Rhead[p->j]=p;   p=p->right;   continue;}
				q1=q2=M2.Rhead[p->j];
				while(q2&&q2->i<state1)
				{q1=q2;	q2=q2->down;}
				q1->down=p;	p->down=q2;
				p=p->right;
			}
			M2.Chead[state2]=link1;
			p=link1;
			while(p) //��������link1
			{
				if(M2.Rhead[p->j]==NULL)
				{M2.Rhead[p->j]=p;  p->down=NULL;   p=p->right;   continue;}
				q1=q2=M2.Rhead[p->j];
				if(M2.Rhead[p->j]->i>state2)
				{p->down=M2.Rhead[p->j];  M2.Rhead[p->j]=p;  p=p->right;  continue;}
				q1=q2=M2.Rhead[p->j];
				while(q2&&q2->i<state2)
				{q1=q2;  q2=q2->down;}
				q1->down=p;	p->down=q2;
				p=p->right;
			}
			//���������������������������������������������ٽ����б任��������������������������������������������������������
			link1=M2.Rhead[state1];
			M2.Rhead[state1]=NULL;
			p=link1;
			while(p)	//��������ɾ��M2->Rhead[state1]
			{
				if(M2.Chead[p->i]->j==state1)
				{
					M2.Chead[p->i]=M2.Chead[p->i]->right;
					p=p->down;
					continue;
				}				
				q1=q2=M2.Chead[p->i];
				while(q2->j!=state1)
				{q1=q2; q2=q2->right;}
				q1->right=q2->right;
				p=p->down;
			}		
			link2=M2.Rhead[state2];
			M2.Rhead[state2]=NULL;
			p=link2;
			while(p)	//��������ɾ��M2->Rhead[state2]
			{
				if(M2.Chead[p->i]->j==state2)
				{
					M2.Chead[p->i]=M2.Chead[p->i]->right;
					p=p->down;
					continue;
				}				
				q1=q2=M2.Chead[p->i];
				while(q2->j!=state2)
				{q1=q2; q2=q2->right;}
				q1->right=q2->right;
				p=p->down;
			}		
			//�޸������������ֵ
			p=link1;
			while(p)
			{p->j=state2;	p=p->down;}
			p=link2;
			while(p)
			{p->j=state1;	p=p->down;}				
			//������������뵽ʮ��������
			M2.Rhead[state1]=link2;	
			p=link2;
			while(p) //����link2
			{
				if(M2.Chead[p->i]==NULL)
				{M2.Chead[p->i]=p;	p->right=NULL;	p=p->down;	continue;}
				if(M2.Chead[p->i]->j>state1)
				{p->right=M2.Chead[p->i];  M2.Chead[p->i]=p;  p=p->down;   continue;}				
				q1=q2=M2.Chead[p->i];
				while(q2&&q2->j<state1)
				{q1=q2;	q2=q2->right;}
				q1->right=p;	p->right=q2;
				p=p->down;	
			}
			M2.Rhead[state2]=link1;
			p=link1;
			while(p)
			{
				if(M2.Chead[p->i]==NULL)
				{M2.Chead[p->i]=p;	p->right=NULL;	p=p->down;	continue;}
				if(M2.Chead[p->i]->j>state2)
				{p->right=M2.Chead[p->i];  M2.Chead[p->i]=p;  p=p->down;   continue;}
				q1=q2=M2.Chead[p->i];
				while(q2&&q2->j<state2)
				{q1=q2;	q2=q2->right;}
				q1->right=p;	p->right=q2;
				p=p->down;
			}

			i=*state_it;	*state_it=*state_pt;	*state_pt=i;//����state_pt��state_it��ָ���Ԫ��
			state_pt++;	//����state_pt��λ��

			j++;
		}//end for it
	}//end for it2
	
	finish=clock();
	duration=(int)(finish-start)/CLOCKS_PER_SEC;
	printf("\n swap matrix cost time :%ld seconds\n",duration);
	//cout<<endl<<"the state order in the swaped matrix:"<<endl;
	//for(state_pt=state_order.begin();state_pt!=state_order.end();state_pt++)
	//	cout<<*state_pt<<'	';
	//cout<<endl<<"the  final swaped matrix :"<<endl;
	//display_matrix(M2);
	Copy_T(M2,transSet,T);
	//cout<<endl<<"the transient matrix T:"<<endl;
	//display_matrix(T);
	return true;	
}

bool Copy_T(CrossList M2,set<state_t> transSet, CrossList &T)
{//��M2�еĹ���״̬�����Ƶ�T�У����Ƚ�M2���Ƹ�T���ٽ�C��Aɾ����
	unsigned int i,tempd,recuSize;
	OLink p;
	copy_matrix(M2,T);	
	recuSize = M2.mu - transSet.size();

	for(i=0;i<recuSize;i++)//ɾ��C
	{
		while(T.Chead[i]!=NULL)
		{	p=T.Chead[i];	T.Chead[i]=p->right;
			free(p);	T.tu--;
		}		
	}
	for(i=recuSize;i<T.mu;i++)//ɾ��A
	{
		while(T.Chead[i]!=NULL&&T.Chead[i]->j<recuSize)
		{
			p=T.Chead[i];	T.Chead[i]=p->right;
			free(p);	T.tu--;
		}
	}
	tempd = T.mu - recuSize;
	for(i=0;i<tempd;i++)
	{
		T.Chead[i]=T.Chead[i+recuSize];
		T.Rhead[i]=T.Rhead[i+recuSize];
	}
	for(i=tempd;i<T.mu;i++)
	{
		T.Chead[i]=NULL;
		T.Rhead[i]=NULL;
	}
	T.mu = T.nu = tempd;
	for(i=0;i<T.mu;i++)
	{	p=T.Chead[i];
		while(p!=NULL)
		{p->i=p->i-recuSize;	p->j=p->j-recuSize;	p=p->right;}
	}	
	
	return true;
}

//����(I-T)�������TI(ʹ�õ�������������)
bool inv_T(CrossList T,CrossList& TI)
{
	CrossList X;
	unsigned int i;
	X.mu=X.nu=T.mu;	X.tu=0;	//��������X
	X.Chead=(OLink*)malloc((X.mu)*sizeof(OLink));
	if(!X.Chead)
	{printf("mallocate memory in inv_T failed!\n");   exit(-1);}
	for(i=0;i<X.mu;i++)
		X.Chead[i]=NULL;
	X.Rhead=(OLink*)malloc((X.nu)*sizeof(OLink));
	if(!X.Rhead)
	{printf("mallocate memory in inv_T failed!\n");   exit(-1);}
	for(i=0;i<X.nu;i++)
		X.Rhead[i]=NULL;

	multiply_matrix(T,X,TI);
	add_matrixI(TI);
	while(!equal_matrix(TI,X))
	{
		free_matrix(&X);
		X.mu=TI.mu;	X.nu=TI.nu;	X.tu=TI.tu;
		X.Chead=TI.Chead;	X.Rhead=TI.Rhead;
		TI.tu=0;	TI.Chead=NULL;	TI.Rhead=NULL;
		multiply_matrix(T,X,TI);
		add_matrixI(TI);
	}
	//cout<<endl<<"the inverse of (I-T) matrix:"<<endl;
	//display_matrix(TI);
	return true;
}

//�������M2����̬����SM,M2���Ѿ��ź���ľ���T��M2�Ĺ��ɾ���TI��(I-T)����
bool steady_matrix(CrossList M2, set<state_t> transSet, myRecuList reculist, CrossList TI, CrossList& SM)
{
	unsigned int i,csize,j,tempd,icycle;
	list< set<state_t> >::iterator it2;
	CrossList CM,tempM,tempM2,M3,YM;
	OLink p,q;
	char filename[10];
	csize=0;
	copy_matrix(M2,M3);
	SM.mu=SM.nu=M3.mu;	SM.tu=0;
	SM.Chead=(OLink*)malloc((SM.mu)*sizeof(OLink));
	if(!SM.Chead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
	SM.Rhead=(OLink*)malloc((SM.nu)*sizeof(OLink));
	if(!SM.Rhead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
	for(i=0;i<SM.mu;i++)	SM.Chead[i]=SM.Rhead[i]=NULL;
	icycle=0;
	for(it2=reculist.begin();it2!=reculist.end();it2++)
	{
		icycle++;
		tempM.mu=tempM.nu=(*it2).size();   tempM.tu=0;
		tempM.Chead=(OLink*)malloc((tempM.mu)*sizeof(OLink));
		if(!tempM.Chead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
		for(i=0;i<tempM.mu;i++)		tempM.Chead[i]=NULL;
		tempM.Rhead=(OLink*)malloc((tempM.nu)*sizeof(OLink));
		if(!tempM.Rhead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
		for(i=0;i<tempM.nu;i++)		tempM.Rhead[i]=NULL;
		//~~~~~~~~~~~~~~~1.��Ci����̬����CM~~~~~~~~~~~~~~~~~~~~~~~~~
		for(i=0;i<tempM.mu;i++)	//���ռ���ת������Ci���Ƶ�tempM��
		{	
			tempM.Chead[i]=M3.Chead[csize+i];
			tempM.Rhead[i]=M3.Rhead[csize+i];
			p=q=M3.Rhead[csize+i];
			while(q && q->i<csize+(*it2).size())
			{p=q;	q=q->down; }
			M3.Rhead[csize+i]=q;
			p->down=NULL;
			M3.Chead[csize+i]=NULL;
		}
		for(i=0;i<tempM.mu;i++)	//����Ci��ÿ��Ԫ��i��jֵ
		{
			p=tempM.Chead[i];
			while(p)
			{p->i=i;  tempM.tu++;	M3.tu--;  p=p->right;}
		}
		for(i=0;i<tempM.nu;i++)
		{
			p=tempM.Rhead[i];
			while(p)
			{p->j=i;  p=p->down;}
		}
		//sprintf(filename,"Ci%d",icycle);
		//output_matrix(tempM,filename);
		//********************delete debug**************************
		//cout<<endl<<"the recurrent matrix:"<<endl;
		//display_matrix(tempM);	//delete********
		//cout<<endl<<"the initial matrix sub recurrent matrix i:"<<endl;
		//display_matrix(M3);
		//********************delete debug**************************	
		multiply_matrix(tempM,tempM,CM); //���ռ�Ci����̬����CM
		while(!equal_matrix(tempM,CM))
		{
			free_matrix(&tempM);
			tempM.mu=CM.mu;	tempM.nu=CM.nu;	tempM.tu=CM.tu;
			tempM.Chead=CM.Chead;	tempM.Rhead=CM.Rhead;
			CM.tu=0;	CM.Chead=NULL;	CM.Rhead=NULL;
			multiply_matrix(tempM,tempM,CM);
		}
		//sprintf(filename,"CM%d",icycle);
		//output_matrix(CM,filename);
		//********************delete debug**************************
		//cout<<endl<<"the steady matrix of recurrent matrix:"<<endl;			
		//display_matrix(CM);
		//free_matrix(&CM);	
		//********************delete debug**************************		
		free_matrix(&tempM);
		
		
		//~~~~~~~~~~~~~~~~~~~2.��Yi����̬����~~~~~~~~~~~~~~~~~~~~~~~~~
		tempM.mu=transSet.size();  tempM.nu=(*it2).size();  tempM.tu=0;
		tempM.Chead=(OLink*)malloc((tempM.mu)*sizeof(OLink));
		if(!tempM.Chead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
		for(i=0;i<tempM.mu;i++)		tempM.Chead[i]=NULL;
		tempM.Rhead=(OLink*)malloc((tempM.nu)*sizeof(OLink));
		if(!tempM.Rhead)	{printf("mallocate memory in steady_matrix failed!\n");   exit(-1);}
		for(i=0;i<tempM.nu;i++)		tempM.Rhead[i]=NULL;
		tempd=M3.mu-transSet.size();		
		for(i=0;i<transSet.size();i++) //��Ai���Ƶ�tempM��(��)
		{
			if(M3.Chead[tempd+i]==NULL)
			{tempM.Chead[i]=NULL;	continue;}
			if(M3.Chead[tempd+i]->j>=csize+(*it2).size())
			{tempM.Chead[i]=NULL;	continue;}
			tempM.Chead[i]=M3.Chead[tempd+i];
			p=q=M3.Chead[tempd+i];
			while(q&&q->j<csize+(*it2).size())	
			{p=q;  q=q->right;}
			p->right=NULL;
			M3.Chead[tempd+i]=q;							
		}
		for(i=0;i<tempM.nu;i++)	//��Ai���Ƶ�tempM��(��)
		{
			tempM.Rhead[i]=M3.Rhead[csize+i];
			M3.Rhead[csize+i]=NULL;
		}
		for(i=0;i<tempM.mu;i++) //����tempM�е�i��jֵ
		{
			p=tempM.Chead[i];
			while(p)
			{p->i=i;  tempM.tu++;  M3.tu--;  p=p->right;}
		}
		for(i=0;i<tempM.nu;i++)
		{
			p=tempM.Rhead[i];
			while(p)
			{p->j=i;  p=p->down;}
		}
		//\********************delete debug**************************
		//cout<<endl<<"the matrix Ai :"<<endl;
		//display_matrix(tempM);	//delete********
		//free_matrix(&tempM);
		//\********************delete debug**************************	

		multiply_matrix(TI,tempM,tempM2);
		multiply_matrix(tempM2,CM,YM);	
		//\********************delete debug**************************
		//cout<<endl<<"the matrix TI*Ai*Ci:"<<endl;
		//display_matrix(YM);
		//free_matrix(&CM);
		//free_matrix(&YM);
		//\********************delete debug**************************
		free_matrix(&tempM);
		free_matrix(&tempM2);
		//~~~~~~~~~~~~~~~~~~~3.��CM��YM����M2����̬����SM��ȥ��T����̬����0��~~~~~~~~~~~~~~~~~~~~~~~~~
		for(i=0;i<(*it2).size();i++) //��CM���Ƶ�SM��
		{
			SM.Chead[csize+i]=CM.Chead[i];	CM.Chead[i]=NULL;
			SM.Rhead[csize+i]=CM.Rhead[i];	CM.Rhead[i]=NULL;			
		}
		for(i=csize;i<csize+(*it2).size();i++) //��������ֵ
		{
			p=SM.Chead[i];
			while(p)
			{p->i=i;  SM.tu++;  p=p->right;}
			q=SM.Rhead[i];
			while(q)
			{q->j=i;  q=q->down;}
		}
		
		for(i=0;i<transSet.size();i++) //����YM����ֵ
		{
			p=YM.Chead[i];
			while(p)
			{p->i=i+tempd;  SM.tu++;  p=p->right;}
		}		
		for(i=0;i<(*it2).size();i++)
		{
			q=YM.Rhead[i];
			while(q)
			{q->j=i+csize;  q=q->down;}
		}
		for(i=0;i<transSet.size();i++) //��YM���Ƶ�SM��(��)
		{
			if(!SM.Chead[i+tempd])
				SM.Chead[i+tempd]=YM.Chead[i];
			else
			{
				p=SM.Chead[i+tempd];
				while(p->right)
					p=p->right;
				p->right = YM.Chead[i];
			}
			YM.Chead[i]=NULL;
		}
		for(i=0;i<(*it2).size();i++)	//��YM���Ƶ�SM��(��)
		{
			p=SM.Rhead[i+csize];
			if(p==NULL)
			{
				//printf("wrong in insert YM%u  col%u!!\n",icycle,i+1);
				/*exit(-1);*/
				SM.Rhead[i+csize]=YM.Rhead[i];
				YM.Rhead[i]=NULL;
				continue;
			}
			while(p->down)
				p=p->down;
			p->down=YM.Rhead[i];
			YM.Rhead[i]=NULL;
		}
		
		free_matrix(&CM);
		free_matrix(&YM);
		csize = csize+(*it2).size();
	}
	//cout<<endl<<"the steady matrix :"<<endl;
	//display_matrix(SM);
	return true;
}

//������ʼ״̬ת�Ƹ��ʾ���M�����������̬ʱ������̬�ĸ������򣬴Ӵ���С����final_order��
bool final_steady_order(CrossList& M,vector<unsigned int>& final_order,vector<double>& steady_prob)
{
	//�����̬���͹���״̬��
	cout<<endl<<"compute the transsient class and recurrient class......."<<endl;
	set<state_t> transSet,tempSet;
	myRecuList reculist;
	State_classification( M, transSet,  reculist);
	State_statistic( M, transSet, reculist);
	//��ת�Ƹ��ʾ�����������
	cout<<endl<<"re_order the initial Matrix......"<<endl;
	CrossList M2,T,TI,SM;
	vector<unsigned int> state_order;
	State_swap2(M,M2,transSet,reculist,state_order,T);
	//cout<<endl<<"output the recurrent matrix T:"<<endl;
	//output_matrix(T,"matrix_T");
	////����(I-T)�������
	////cout<<endl<<"compute the inverse matrix of T......"<<endl;
	////inv_T(T,TI);
	////cout<<endl<<"the inverse of recurrent matrix T,TI:"<<endl;
	////display_matrix(TI);
	free_matrix(&T);	//�ͷž���T��M
	free_matrix(&M);
	//�������õ�TI
	TI.Chead=TI.Rhead=NULL;
	cout<<endl<<"input the inverse of matrix (I-T)......"<<endl;
	input_matrix(TI,"matrix_TI");
	//������̬����SM
	cout<<endl<<"compute the steady matrix SM ......"<<endl;
	steady_matrix(M2, transSet, reculist, TI, SM);
	free_matrix(&TI);	//�ͷž���TI��M2
	free_matrix(&M2);
	//������̬��������
	unsigned int order0,i,j,k;
	vector<unsigned int>::iterator state_pt,state_it;
	vector<double>::iterator state_pp;
	double* temp_order;
	double emax;
	OLink p;
	cout<<endl<<"compute the final_order......"<<endl;
	state_pt=state_order.begin();
	while((*state_pt)!=0)	//����״̬0��λ��
		state_pt++;
	order0=state_pt-state_order.begin();
	p=SM.Chead[order0];	
	temp_order = (double*)malloc((SM.mu-transSet.size())*sizeof(double));
	for(i=0;i<SM.mu-transSet.size();i++) 
		temp_order[i]=-1;
	j=0;
	while(p) //��״̬0�����е�ת�Ƹ��ʸ��Ƶ�temp_order��
	{	
		temp_order[p->j]=p->e;
		p=p->right;	
		j++;
	}
	while(j!=0)
	{
		emax=0;	   k=-1;
		for(i=0;i<SM.mu-transSet.size();i++)
		{
			if(temp_order[i]-emax>=0.000000000000001)
			{ emax=temp_order[i];  k=i;}
		}
		temp_order[k]=-1; 
		if(k!=-1)
		{//*****ע������k��M2�е�k��λ�õ�״̬����Ҫ�ҵ�����state_order�д����״̬	
			//final_order.push_back(k);
			final_order.push_back(state_order.at(k));
			steady_prob.push_back(emax);
		}
		j--;
	}
	cout<<endl<<"the final order in recurrent states("<<final_order.size()<<") :"<<endl;
	free_matrix(&SM);	//�ͷž���SM	
	// ��״̬���ʵ�����������ļ�state_order��
	FILE*outfile=fopen("state_order","w");
	for(state_pt=final_order.begin(),state_pp=steady_prob.begin();
			state_pt!=final_order.end(),state_pp!=steady_prob.end();state_pt++,state_pp++)
	{
		printf("%u	%0.15lf\n",*state_pt,*state_pp);
		fprintf(outfile,"%u	%0.15lf\n",*state_pt,*state_pp);
	}
	fclose(outfile);
	return true;
}

#endif

