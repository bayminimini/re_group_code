/*
 * Copyright (c) 2007 Michela Becchi and Washington University in St. Louis.
 * All rights reserved
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. The name of the author or Washington University may not be used
 *       to endorse or promote products derived from this source code
 *       without specific prior written permission.
 *    4. Conditions of any other entities that contributed to this are also
 *       met. If a copyright notice is present from another entity, it must
 *       be maintained in redistributions of the source code.
 *
 * THIS INTELLECTUAL PROPERTY (WHICH MAY INCLUDE BUT IS NOT LIMITED TO SOFTWARE,
 * FIRMWARE, VHDL, etc) IS PROVIDED BY  THE AUTHOR AND WASHINGTON UNIVERSITY
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR WASHINGTON UNIVERSITY
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS INTELLECTUAL PROPERTY, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * */

/*
 * File:   parser.c
 * Author: Michela Becchi
 * Email:  mbecchi@cse.wustl.edu
 * Organization: Applied Research Laboratory
 */

#include "parser.h"
#include <limits.h>


regex_parser::regex_parser(bool i_mod, bool m_mod){
	i_modifier=i_mod; 
	m_modifier=m_mod;
	NFA::ignore_case=i_mod;
}

regex_parser::~regex_parser(){;}

NFA *regex_parser::parse(char **array, int m, int n, int* re_len){
	int i=0;
	int j=0;
	// NFA
	NFA *nfa=new NFA(); 
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	char *re = allocate_char_array(1000);
	
	for(i=0; i<m; i++){
		if(re_len[i]==0)
			continue;
		for(j=0; j<re_len[i]; j++)
			re[j] = *((char*)array + n*i + j);
		re[j]='\0';
		if (DEBUG) fprintf(stdout,"\n%d) processing regex:: <%s> ...\n",i,re);
		parse_re(nfa,re);
		free(re);
		re=allocate_char_array(1000);
	}
	
	if (DEBUG) fprintf(stdout, "\nAll RegEx processed\n");
	if (re!=NULL) free(re);

	//handle -m modifier
	if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){
		non_anchored->add_transition('\n',anchored);
		non_anchored->add_transition('\r',anchored);
	}
	
	//delete non_anchored, if necessary
	if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){
		nfa->get_epsilon()->remove(non_anchored);
		delete non_anchored;
	}else{
		non_anchored->add_any(non_anchored);
	}
	
	return nfa->get_first();
}



NFA *regex_parser::parse(FILE *file, int from, int to){
	rewind(file);
	char *re=allocate_char_array(1000);
	int i=0;
	int j=0;
	unsigned int c=fgetc(file);
	
	// NFA
	NFA *nfa=new NFA(); 
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	
	//parsing the RegEx and putting them in a NFA
	while(c!=EOF){
		if (c=='\n' || c=='\r'){
			if(i!=0){
				re[i]='\0';
				if (re[0]!='#'){
					j++;
					if (j>=from && (to==-1 || j<=to)){
						if (DEBUG) fprintf(stdout,"\n%d) processing regex:: <%s> ...\n",j,re);
						parse_re(nfa, re);
					}
				} 
				i=0;
				free(re);
				re=allocate_char_array(1000);
			}
		}else{
			re[i++]=c;
		}	
		c=fgetc(file);
	} //end while
	
	if(i!=0){
		re[i]='\0';
		if (re[0]!= '#'){
			j++;
			if (j>=from && (to==-1 || j<=to)){
				if (DEBUG) fprintf(stdout,"\n%d) processing regex:: <%s> ...\n",j,re);
				parse_re(nfa,re);
			}
		}
		free(re);
	}
	if (DEBUG) fprintf(stdout, "\nAll RegEx processed\n");
	
	if (re!=NULL) free(re);
	
	//handle -m modifier
	if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){
		non_anchored->add_transition('\n',anchored);
		non_anchored->add_transition('\r',anchored);
	}
	
	//delete non_anchored, if necessary
	if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){
		nfa->get_epsilon()->remove(non_anchored);
		delete non_anchored;
	}else{
		non_anchored->add_any(non_anchored);
	}
	
	return nfa->get_first();
	
}

unsigned num_regex(FILE *file){
	rewind(file);
	int re_0;
	int i=0;
	int j=0;
	unsigned int c=fgetc(file);
	
	//parsing the RegEx and putting them in a NFA
	while(c!=EOF){
		if (c=='\n' || c=='\r'){
			if(i!=0){
				if (re_0!='#') j++;
				i=0;
			}
		}else{
			if (i==0){
				re_0=c;
				i++;
			}
		}	
		c=fgetc(file);
	} //end while
	
	if(i!=0){
		if (re_0 != '#') j++;
	}
	
	return j;	
}

dfa_set *regex_parser::parse_to_dfa(FILE *file){ //This is not Yu's grouping algorithm
	dfa_set *dfas=new dfa_set();
	list <pair <unsigned,unsigned> > *queue = new list <pair <unsigned,unsigned> >();
	queue->push_back(pair <unsigned,unsigned>(1,num_regex(file)));
	while (!queue->empty()){
		int min=(queue->front()).first;
		int max=(queue->front()).second;
		queue->pop_front();
		NFA *nfa=parse(file,min,max);
		nfa->remove_epsilon();
		nfa->reduce();
		DFA *dfa=nfa->nfa2dfa();
		delete nfa;
		if (dfa!=NULL){
			if (VERBOSE) printf("DFA created for regex %ld to %ld\n",min,max);
			dfa->minimize();
			dfas->insert(dfa);
		}
		else{
			if (min==max) fatal("Could not create DFA on single Regex");
			queue->push_back(pair<unsigned,unsigned>(min,(min+max)/2));
			queue->push_back(pair<unsigned,unsigned>((min+max)/2+1,max));
		}
	}
	delete queue;
	return dfas;
}

bool regex_parser::read_re_to_array(FILE *file, char **res, int re_num){
	int re_i,pos_j;
	unsigned int c;
	
	re_i=0; pos_j=0;  
	rewind(file);
	c = fgetc(file);
	while(c!=EOF){
		if(c=='\n' || c=='\r'){
			if(pos_j!=0){
				res[re_i][pos_j]='\0';
				//fprintf(stdout,"DEBUG: res[%d][%d]=%c\n\n",re_i,pos_j,res[re_i][pos_j]);
				if(res[re_i][0]!='#'){
					re_i++;
				}
				pos_j=0;
			}	
		}else{
			if(pos_j==MAX_PATTERN_LEN-1){
				fprintf(stderr,"regex pattern length is more than 256\n");
				exit(0);
			}
			res[re_i][pos_j++]=c;
			//fprintf(stdout,"DEBUG: res[%d][%d]=%c\n",re_i,pos_j-1,c);
		}
		c=fgetc(file);
	}
	if(pos_j!=0){
		res[re_i][pos_j]='\0';
		if(res[re_i][0]!='#'){
			re_i++;
		}
	}
	if (DEBUG) fprintf(stdout, "\nAll %d regex patterns have been read to array\n\n",re_i);
	if(re_i!=re_num) {fprintf(stderr,"Error in function read_re_to_array\n"); exit(0);}  //never happen

	return 1;
}

bool regex_parser::compute_intersection(char **res, int re_num, int **re_inter_matrix){
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	unsigned int *dfa_state_num;

	dfa_state_num = allocate_uint_array(re_num);
	if(NULL==dfa_state_num)	{fprintf(stderr,"allocate mem failed in function compute intersection\n"); exit(0);}
	for(int i=0; i<re_num; i++)
		re_inter_matrix[i][i]=0;
	for(int i=0; i<re_num; i++){
		parse_re(nfa,res[i]);
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
			non_anchored->add_transition('\n',anchored);
			non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL){
			dfa->minimize();
			if (DEBUG) printf("DFA created for regex %ld, state num %ld\n",i,dfa->size());
		}
		dfa_state_num[i] = dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
	}
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			//compile regex i and regex j together
			parse_re(nfa,res[i]);
			parse_re(nfa,res[j]);
			if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
			}
			if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
				nfa->get_epsilon()->remove(non_anchored);
				delete non_anchored;
			}else{
				non_anchored->add_any(non_anchored);
			}
			nfa = nfa->get_first();
			nfa->remove_epsilon();
			nfa->reduce();
			dfa=nfa->nfa2dfa();
			if (dfa!=NULL){
				dfa->minimize();
				if (DEBUG) printf("DFA created for regex %ld & %ld, state num %ld\n",i,j,dfa->size());
			}
			re_inter_matrix[i][j] = dfa->size() - dfa_state_num[i] - dfa_state_num[j];
			re_inter_matrix[j][i] = re_inter_matrix[i][j];
			if(DEBUG) printf("re_inter_matrix[%ld][%ld]=%d\n",i,j,re_inter_matrix[i][j]);

			if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
			if (dfa!=NULL) delete dfa; dfa=NULL;
		}
	}
	if(NULL!=dfa_state_num){
		free(dfa_state_num);	
		dfa_state_num=NULL;
	}

	//output the original interaction matrix
	FILE *outfile = fopen("orig_inter_matrix","w");
	for(int i=0; i<re_num; i++){
		for(int j=0; j<re_num; j++)
			fprintf(outfile,"%d ",re_inter_matrix[i][j]);
		fprintf(outfile,"\n");
	}
	fclose(outfile);
	//output the non-negative interaction matrix
	outfile = fopen("nonneg_inter_matrix","w");
	for(int i=0; i<re_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_inter_matrix[i][j]<0)	
				fprintf(outfile,"%d ",0);
			else	fprintf(outfile,"%d ",re_inter_matrix[i][j]);
		}
		fprintf(outfile,"\n");
	}
	fclose(outfile);
	return 1;
}

//compute the expansion power for each rule


bool regex_parser::yu_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int* avg_size){
	unsigned int cur_inter, min_inter, tmp_id, group_num, max_single_dfa_size, cur_dfa_size, grouped_re_num, tmp_v1, tmp_v2,tmp_v3;
	group_num=0; cur_inter=0; min_inter=65535; group_num=0; grouped_re_num=0;
	max_single_dfa_size = 5000; cur_dfa_size=0;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	for(int i=0; i<re_num; i++)
		re_groups[i]=-1;
	//choose a re which has least interactions with other res to launch the first group
	for(int i=0; i<re_num; i++){
		cur_inter=0;
		for(int j=0; j<re_num; j++){
			if(re_inter_matrix[i][j]>0)
				cur_inter++;
		}
		if(cur_inter<min_inter){
			min_inter = cur_inter;
			tmp_id = i;
		}
		if(DEBUG) fprintf(stdout,"interaction of re %d is %d\n",i,cur_inter);
	}
	re_groups[tmp_id] = group_num;	grouped_re_num++;
	if(DEBUG) fprintf(stdout,"add rule %d to group %d\n",tmp_id,group_num);
	//compile the first rule
	parse_re(nfa,res[tmp_id]);
	if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
			non_anchored->add_transition('\n',anchored);
			non_anchored->add_transition('\r',anchored);
	}
	if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
		nfa->get_epsilon()->remove(non_anchored);
		delete non_anchored;
	}else{
		non_anchored->add_any(non_anchored);
	}
	nfa = nfa->get_first();
	nfa->remove_epsilon();
	nfa->reduce();
	dfa=nfa->nfa2dfa();
	if (dfa!=NULL)
		dfa->minimize();
	cur_dfa_size= dfa->size();
	if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
	if (dfa!=NULL) delete dfa; dfa=NULL;
	if(cur_dfa_size>max_single_dfa_size){ fprintf(stderr,"dfa size of a single re exceeds the limitation!\n"); exit(0);}

	//set up the groups one by one
	while(grouped_re_num<re_num){ //keep grouping until all rules have been grouped
		while(cur_dfa_size<max_single_dfa_size && grouped_re_num<re_num){ //keep add new re until the current group exceeds the limitation
			//find a the re i which has the min interactions with the current group
			cur_inter=0; min_inter=65535;
			for(int i=0; i<re_num; i++){
				if(-1==re_groups[i]){// only for not grouped rules
					for(int j=0; j<re_num; j++){ //rule j is already in the current group
						if(re_groups[j]==group_num && re_inter_matrix[i][j]>0)
							cur_inter++;
					}
					if(cur_inter<min_inter){
						min_inter = cur_inter;
						tmp_id = i;
					}
					cur_inter=0;
				}
			}
			//try to add the re i to the current group
			parse_re(nfa,res[tmp_id]);
			for(int i=0; i<re_num; i++){
				if(re_groups[i]==group_num)
					parse_re(nfa,res[i]);
			}
			if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
			}
			if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
				nfa->get_epsilon()->remove(non_anchored);
				delete non_anchored;
			}else{
				non_anchored->add_any(non_anchored);
			}
			nfa = nfa->get_first();
			nfa->remove_epsilon();
			nfa->reduce();
			dfa=nfa->nfa2dfa();
			if (dfa!=NULL)
				dfa->minimize();
			cur_dfa_size = dfa->size();
			if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
			if (dfa!=NULL) delete dfa; dfa=NULL;
			
			if(cur_dfa_size > max_single_dfa_size) //generate a new group
				break;
			else{ //add the rule to the current group
				re_groups[tmp_id] = group_num;  grouped_re_num++;
				if(DEBUG) fprintf(stdout,"add rule %d to group %d\n",tmp_id,group_num);
			}
		}
		if(grouped_re_num == re_num) //all rules have been grouped
			break;
		//generate a new group
		parse_re(nfa,res[tmp_id]);
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			dfa->minimize();
		cur_dfa_size = dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		if(cur_dfa_size>max_single_dfa_size){ fprintf(stderr,"dfa size of a single re exceeds the limitation!\n"); exit(0);}
		group_num++;
		re_groups[tmp_id] = group_num; grouped_re_num++;
		if(DEBUG) fprintf(stdout,"add rule %d to group %d\n",tmp_id,group_num);
	}


	//print the grouping results
	fprintf(stdout,"\nGrouping results of FangYu's grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<=group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			;	//dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"\nThe upper limitation of state number for a single DFA is %d\n",max_single_dfa_size);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num+1,tmp_v2);
	*avg_size = tmp_v2/(group_num+1);
}

bool regex_parser::sa_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	double e_init,e_n; 
	unsigned int tmp_id1, tmp_id2, pre_map[3][2], tmp_v1, tmp_v2;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	long time_before, time_after;

	//srand((unsigned)time(NULL));
	//time_before = time(NULL);
	//Initialize the grouping, random group the rules into group_num groups.
	if(DEBUG) fprintf(stdout,"the initial random grouping: \n");
	for(int i=0; i<re_num; i++){
		re_groups[i] = rand()%group_num;
		if(DEBUG) fprintf(stdout,"rule %d in group %d  \n",i,re_groups[i]);
	}
	//compute the initial energe value, here it means total intersections in each group. 
	e_init=0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				e_init = e_init + re_inter_matrix[i][j];
		}
	}
	//the outer loop, decrease the temperature, i is the temperature
	for(double i=re_num; i>0; i--){
		//get the neighbourhood of re_groups, randomly select three patterns and randomly re-mapped them to other groups
		for(int j=0; j<3; j++){
			tmp_id1 = rand()%re_num; //the selected rule
			tmp_id2 = rand()%group_num; //the selected group
			pre_map[j][0]=tmp_id1;	//record the previous solution	
			pre_map[j][1]=re_groups[tmp_id1];

			re_groups[tmp_id1] = tmp_id2; //new group
		}
		//compute the current energy value e_n
		e_n=0;
		for(int k=1; k<re_num; k++){ //(any speedup in this loop?)
			for(int l=0; l<k; l++){
				if(re_groups[k] == re_groups[l])
					e_n = e_n + re_inter_matrix[k][l];	
			}
		}
		//accept or reject the new solution, depending on 
		if(e_n < e_init) //accept the lower energy solution
			e_init = e_n;
		else{ //accept the higher energy solution with some probability
			if(exp((e_init-e_n)/i) > (rand()%0x7fff)/(float)(0x7fff))
				e_init = e_n;
			else{ //reject the higher energy solution, keep the previous group state
				for(int m=0; m<3; m++){
					re_groups[pre_map[m][0]] = pre_map[m][1];
				}
			}
		}
	}
	//time_after = time(NULL);
	//if(DEBUG) fprintf(stdout,"time for the grouping procedure: %ld s\n",time_after-time_before);


	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of simulated annealing grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
				fprintf(stdout,"rule %d in group %d\n",j,i);
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa==NULL) printf("Max DFA size %ld exceeded during creation: the DFA was not generated\n",MAX_DFA_SIZE);
		else ;//dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"the total internal intersection value is %d \n",e_init);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);

	return true;
}

bool regex_parser::sa_hi_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	double e_init,e_n; 
	unsigned int tmp_id1, tmp_id2, pre_map[3][2], tmp_v1, tmp_v2;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	long time_before, time_after;

	//initilize the grouping with HI algorithm
	hi_group_algo(res, re_num, re_inter_matrix, re_groups, group_num, avg_size);
	if(DEBUG) fprintf(stdout,"\n\nthe initial random grouping: \n");
	for(int i=0; i<re_num; i++){
		if(DEBUG) fprintf(stdout,"rule %d in group %d  \n",i,re_groups[i]);
	}
	
	//compute the initial energe value, here it means total intersections in each group. 
	e_init=0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				e_init = e_init + re_inter_matrix[i][j];
		}
	}
	//the outer loop, decrease the temperature, i is the temperature
	for(double i=re_num; i>0; i--){
		//get the neighbourhood of re_groups, randomly select three patterns and randomly re-mapped them to other groups
		for(int j=0; j<3; j++){
			tmp_id1 = rand()%re_num; //the selected rule
			tmp_id2 = rand()%group_num; //the selected group
			pre_map[j][0]=tmp_id1;	//record the previous solution	
			pre_map[j][1]=re_groups[tmp_id1];

			re_groups[tmp_id1] = tmp_id2; //new group
		}
		//compute the current energy value e_n
		e_n=0;
		for(int k=1; k<re_num; k++){ //(any speedup in this loop?)
			for(int l=0; l<k; l++){
				if(re_groups[k] == re_groups[l])
					e_n = e_n + re_inter_matrix[k][l];	
			}
		}
		//accept or reject the new solution, depending on 
		if(e_n < e_init) //accept the lower energy solution
			e_init = e_n;
		else{ //accept the higher energy solution with some probability
			if(exp((e_init-e_n)/i) > (rand()%0x7fff)/(float)(0x7fff))
				e_init = e_n;
			else{ //reject the higher energy solution, keep the previous group state
				for(int m=0; m<3; m++){
					re_groups[pre_map[m][0]] = pre_map[m][1];
				}
			}
		}
	}
	//time_after = time(NULL);
	//if(DEBUG) fprintf(stdout,"time for the grouping procedure: %ld s\n",time_after-time_before);


	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of simulated annealing grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
				fprintf(stdout,"rule %d in group %d\n",j,i);
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa==NULL) printf("Max DFA size %ld exceeded during creation: the DFA was not generated\n",MAX_DFA_SIZE);
		else ;//dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"the total internal intersection value is %d \n",e_init);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);
	return true;

}

	


bool regex_parser::sa2_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	double e_init,e_n; 
	unsigned int tmp_id1, tmp_id2, pre_map[3][2], tmp_v1, tmp_v2, counter;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	
	//Initialize the grouping, random group the rules into group_num groups.
	srand((unsigned)time(NULL));
	for(int i=0; i<re_num; i++){
		re_groups[i] = rand()%group_num;
	}
	//compute the initial energe value, here it means total intersections in each group. 
	e_init=0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				e_init = e_init + re_inter_matrix[i][j];
		}
	}
	counter = 0; 
	//the outer loop, decrease the temperature, i is the temperature
	for(double i=re_num; i>0; i--){
		//get the neighbourhood of re_groups, randomly select three patterns and randomly re-mapped them to other groups
		for(int j=0; j<3; j++){
			tmp_id1 = rand()%re_num; //the selected rule
			tmp_id2 = rand()%group_num; //the selected group
			pre_map[j][0]=tmp_id1;	//record the previous solution	
			pre_map[j][1]=re_groups[tmp_id1];

			re_groups[tmp_id1] = tmp_id2; //new group
		}
		//compute the current energy value e_n
		e_n=0;
		for(int k=1; k<re_num; k++){
			for(int l=0; l<k; l++){
				if(re_groups[k] == re_groups[l])
					e_n = e_n + re_inter_matrix[k][l];	
			}
		}
		//accept or reject the new solution, depending on 
		if(e_n < e_init){ //accept the lower energy solution
			e_init = e_n;
			counter = 0;
		}
		else{ //accept the higher energy solution with some probability
			if(exp((e_init-e_n)/i) > (rand()%0x7fff)/(float)(0x7fff))
				e_init = e_n;
			else{ //reject the higher energy solution, keep the previous group state
				for(int m=0; m<3; m++){
					re_groups[pre_map[m][0]] = pre_map[m][1];
				}
			}
			counter++;
		}
		if(counter>3) //teminate the loop if no improvements for successive three times.
			break;
	}

	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of simulated annealing grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
				fprintf(stdout,"rule %d in group %d\n",j,i);
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);

	return true;
}

bool regex_parser::sa2_hi_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	double e_init,e_n; 
	unsigned int tmp_id1, tmp_id2, pre_map[3][2], tmp_v1, tmp_v2, *ep, *ep_order, *ep_group, counter;
	bool *ep_mark;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	
	ep = allocate_uint_array(re_num);
	ep_order = allocate_uint_array(re_num);
	ep_mark = allocate_bool_array(re_num);
	ep_group = allocate_uint_array(group_num);
	if(NULL==ep || NULL==ep_order || NULL==ep_group || NULL==ep_mark)	{fprintf(stdout,"allocate memory failed in function sa_hi_group_algo2\n"); exit(0);}
	srand((unsigned)time(NULL));
	//Initialize the grouping, with the heuristic initial grouping algorithm
	for(int i=0; i<re_num; i++){
		ep[i] = 0;
		for(int j=0; j<re_num; j++)
			ep[i] = ep[i] + re_inter_matrix[i][j];
	}
	for(int i=0; i<re_num; i++)
		ep_mark[i] = false;
	tmp_v1=0;	tmp_v2=UINT_MAX;
	for(int i=0; i<re_num; i++){
		for(int j=0; j<re_num; j++){
			if(false==ep_mark[j]){
				if(ep[j]>=tmp_v1 && ep[j]<=tmp_v2){ //这块判断有问题，还要改；如果有相等的就不能解决了。
					tmp_id1 = j;
					tmp_v1 = ep[j];
				}
			}
		}
		ep_order[i] = tmp_id1;
		ep_mark[tmp_id1]=true;
		tmp_v2 = tmp_v1;  //upper limitation for the next iteration
		tmp_v1 = 0;
	}
	for(int i=0; i<group_num; i++){
		ep_group[i] = ep[ep_order[i]];
		re_groups[ep_order[i]] = i;
		fprintf(stdout,"re_groups[%d]=%d",ep_order[i],i);
	}
	for(int i=group_num; i<re_num; i++){
		tmp_v1=UINT_MAX;
		for(int j=0; j<group_num; j++){
			if(ep_group[j]<tmp_v1){
				tmp_v1 = ep_group[j];
				tmp_id1 = j;
			}
		} 
		ep_group[tmp_id1] = ep_group[tmp_id1] + ep[ep_order[i]];
		re_groups[ep_order[i]] = tmp_id1;
		fprintf(stdout,"re_groups[%d]=%d",ep_order[i],tmp_id1);
	}
	

	//compute the initial energe value, here it means total intersections in each group. 
	e_init=0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				e_init = e_init + re_inter_matrix[i][j];
		}
	}
	counter = 0; 
	//the outer loop, decrease the temperature, i is the temperature
	for(double i=re_num; i>0; i--){
		//get the neighbourhood of re_groups, randomly select three patterns and randomly re-mapped them to other groups
		for(int j=0; j<3; j++){
			tmp_id1 = rand()%re_num; //the selected rule
			tmp_id2 = rand()%group_num; //the selected group
			pre_map[j][0]=tmp_id1;	//record the previous solution	
			pre_map[j][1]=re_groups[tmp_id1];

			re_groups[tmp_id1] = tmp_id2; //new group
		}
		//compute the current energy value e_n
		e_n=0;
		for(int k=1; k<re_num; k++){
			for(int l=0; l<k; l++){
				if(re_groups[k] == re_groups[l])
					e_n = e_n + re_inter_matrix[k][l];	
			}
		}
		//accept or reject the new solution, depending on 
		if(e_n < e_init){ //accept the lower energy solution
			e_init = e_n;
			counter = 0;
		}
		else{ //accept the higher energy solution with some probability
			if(exp((e_init-e_n)/i) > (rand()%0x7fff)/(float)(0x7fff))
				e_init = e_n;
			else{ //reject the higher energy solution, keep the previous group state
				for(int m=0; m<3; m++){
					re_groups[pre_map[m][0]] = pre_map[m][1];
				}
			}
			counter++;
		}
		if(counter>3) //teminate the loop if no improvements for successive three times.
			break;
	}

	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of simulated annealing grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
				fprintf(stdout,"rule %d in group %d\n",j,i);
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);

	return true;
}


bool regex_parser::ga_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	int population_size, generation_num, tmp_v,tmp_v1,tmp_v2, cross[5], mut_num, parent1_no, parent2_no;
	unsigned char **population, *offspring;
	unsigned int tmp_rand, *fit_value, best_fit, best_fit_no, best_gene_no, tmp_fit_off, worst_fit, worst_fit_no;
	double sum_prob, total_fit, tmp_prob;
	
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;

	//Initialization
	population_size = 50 ;
	generation_num = 100;
	population = (unsigned char**)allocate_array(population_size,sizeof(unsigned char*));
	if(NULL==population) {fprintf(stderr,"allocate failed in function ga_group_algo\n"); exit(0);}
	for(int i=0; i<population_size; i++){
		population[i] = (unsigned char*)allocate_array(re_num,sizeof(unsigned char));
		if(NULL==population[i]) {fprintf(stderr,"allocate failed in function ga_group_algo\n"); exit(0);}
	}
	srand((unsigned)time(NULL));
	for(int i=0; i<population_size; i++){
		for(int j=0; j<re_num; j++){
			population[i][j] = rand()%group_num;
		}
	}
	fit_value = allocate_uint_array(population_size);
	if(NULL==fit_value) {fprintf(stderr,"allocate failed in function ga_group_algo\n"); exit(0);}
	offspring = (unsigned char*)allocate_array(re_num,sizeof(unsigned char));
	if(NULL==offspring)  {fprintf(stderr,"allocate failed in function ga_group_algo\n"); exit(0);}
	total_fit =0;
	for(int j=0; j<population_size; j++){ //Assess the fitness of each individual in the current generation
		fit_value[j]=0;
		for(int k=1; k<re_num; k++){
			for(int l=0; l<k; l++){
				if(population[j][k]!=population[j][l])
					fit_value[j] = fit_value[j] + re_inter_matrix[k][l];
			}
		}
		fprintf(stdout,"DEBUG: fit_value[%d]:%d\n",j,fit_value[j]);
		total_fit = total_fit + fit_value[j];
	}	
	
	//loop for generation_num generations
	for(int i=0; i<generation_num; i++){ //for each generation
		//Selection & Crossover
//		for(int m=0; m<population_size; m++){ //loop for m times, each time selects two parents with pvoportional selection method, then excute crossover
											  //to generate the offsprings
		//select parent1 & parent2
		sum_prob = 0;	tmp_prob = (double)(rand()%0x7fff)/(double)(0x7fff);
		for(int n=0; n<population_size; n++){  //select parent1
			sum_prob = sum_prob + (double)fit_value[n]/total_fit;
			if(tmp_prob < sum_prob){
				parent1_no = n;
				break;
			}
		}
		parent2_no = parent1_no;
		while(parent2_no==parent1_no){
			sum_prob = 0;	tmp_prob = (double)(rand()%0x7fff)/(double)(0x7fff);
			for(int n=0; n<population_size; n++){  //select parent2
				sum_prob = sum_prob + (double)fit_value[n]/total_fit;
				if(tmp_prob < sum_prob){
					parent2_no = n;
					break;
				}
			}
		}
		//corssover with five different cut point, change three parts, generate the offspring
		for(int s=0; s<5; s++) //generate cross point
			cross[s]=rand()%re_num;
		while(cross[1]==cross[0])	cross[1]=rand()%re_num;
		while(cross[2]==cross[1]||cross[2]==cross[0])	cross[2]=rand()%re_num;
		while(cross[3]==cross[2]||cross[3]==cross[1]||cross[3]==cross[0])	cross[3]=rand()%re_num;
		while(cross[4]==cross[3]||cross[4]==cross[2]||cross[4]==cross[1]||cross[4]==cross[0])	cross[4]=rand()%re_num;
		for(int s=0; s<4; s++){ //sort the cross point
			for(int t=0; t<4-s; t++){
				if(cross[t]>cross[t+1]){
					tmp_v = cross[t];
					cross[t] = cross[t+1];
					cross[t+1] = tmp_v;
				}
			}
		}
		//Cross
		for(int s=0; s<re_num; s++)		offspring[s] = population[parent1_no][s];
		for(int s=cross[0]; s<cross[1]; s++)	offspring[s] = population[parent2_no][s];
		for(int s=cross[2]; s<cross[3]; s++)	offspring[s] = population[parent2_no][s];
		for(int s=cross[4]; s<re_num; s++)		offspring[s] = population[parent2_no][s];
		//Mutation, select mut_num positions, mut_num is a uniform random integer on interval [0,re_num/100]
		mut_num = re_num/100;
		for(int s=0; s<mut_num; s++){
			tmp_rand = rand()%re_num;
			offspring[tmp_rand] = rand()%group_num;
		}
		//Replacement, from Thang Nguyen Bu's method, if the offspring is worse than both parents, then replace the most inferior member of current generation
		//else, replace the more similar parent
		//	first, compute the fit_value of the offspring
		tmp_fit_off=0;
		for(int s=1; s<re_num; s++){
			for(int t=0; t<s; t++){
				if(offspring[s]!=offspring[t])
					tmp_fit_off = tmp_fit_off + re_inter_matrix[s][t];
			}	
		}
		if(tmp_fit_off<fit_value[parent1_no] && tmp_fit_off<fit_value[parent2_no]){ //replace the most inferior member
			worst_fit=0xffffffff;
			for(int s=0; s<population_size; s++){
				if(fit_value[s]<worst_fit){worst_fit = fit_value[s];  worst_fit_no = s;}
			}
			fprintf(stdout,"DEBUG1: worst_fit_no:%d, worst_fit:%d\n",worst_fit_no,worst_fit);
			for(int s=0; s<re_num; s++){
				population[worst_fit_no][s] = offspring[s];  //update the individual
			}
			total_fit = total_fit + tmp_fit_off - fit_value[worst_fit_no]; //update the fit value
			fit_value[worst_fit_no] = tmp_fit_off; 
			if(DEBUG)
				fprintf(stdout,"Generation:%d, parent1:%d, parent2:%d, replace most inferior member:%d\n",i,parent1_no,parent2_no,worst_fit_no);	
		}
		else{ //replace the more similar parent
			if(cross[1]-cross[0]+cross[3]-cross[2]+re_num-cross[4]<re_num/2){ //replace parent1
				for(int s=0; s<re_num; s++)	population[parent1_no][s] = offspring[s];  //update the individual								
				total_fit = total_fit + tmp_fit_off -fit_value[parent1_no]; //update the fit value
				fit_value[parent1_no] = tmp_fit_off;  
				if(DEBUG) fprintf(stdout,"Generation:%d, parent1:%d, parent2:%d, replace parent1\n",i,parent1_no,parent2_no);										
			}else{
				for(int s=0; s<re_num; s++)	population[parent2_no][s] = offspring[s];  //update the individual
				total_fit = total_fit + tmp_fit_off - fit_value[parent2_no]; //update the fit value
				fit_value[parent2_no] = tmp_fit_off;  
				if(DEBUG) fprintf(stdout,"Generation:%d, parent1:%d, parent2:%d, replace parent2\n",i,parent1_no,parent2_no);										
			}
		}
		if(DEBUG){
			best_fit=0;
			for(int s=0; s<population_size; s++){
				if(fit_value[s]>best_fit){best_fit=fit_value[s];  best_fit_no = s;}
			}
			fprintf(stdout,"DEBUG2:	best_fit_no:%d, best fit:%d\n",best_fit_no,best_fit);
		}
//		}
	}

	for(int i=0; i<re_num; i++)
		re_groups[i] = population[best_fit_no][i];
	for(int s=0; s<population_size; s++){
		if(NULL!=population[s]){free(population[s]); population[s]=NULL;}
	}
	free(population);	population=NULL;
	if(NULL!=fit_value) {free(fit_value); fit_value=NULL;}
	if(NULL!=offspring) {free(offspring); offspring=NULL;}
	
	//print the grouping results
	fprintf(stdout,"\nGrouping results of simulated annealing grouping algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);
}

bool regex_parser::osg_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	int **cut_inc_mat, tmp_sum1, tmp_sum2, tmp_v, moved_re_num, last_inc, max_inc, rule_id, group_id, org_group_id;
	int tmp_v1, tmp_v2, pass_num, internal_inter;
	bool *b_moved;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	//Initialization
	b_moved = allocate_bool_array(re_num);
	if(NULL==b_moved) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	cut_inc_mat = allocate_int_matrix(re_num);
	if(NULL==cut_inc_mat) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	for(int i=0; i<re_num; i++){
		cut_inc_mat[i] = allocate_int_array(group_num);
		if(NULL==cut_inc_mat[i]) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	}
	srand((unsigned)time(NULL));
	for(int i=0; i<re_num; i++){	//initialize the groups
		re_groups[i] = rand()%group_num;
		//if(DEBUG)	fprintf(stdout,"re_groups[%d]=%d\n",i,re_groups[i]);
	}
	//if (DEBUG) fprintf(stdout, "\nDEBUG1 in osg re_num=%d, group_num=%d\n",re_num,group_num);
	//compute the cutsize increasement for each rule to each group(the increasement value maybe positive, zero or negative)
	for(int i=0; i<re_num; i++){
		tmp_v = re_groups[i];
		for(int j=0; j<group_num; j++){	//move rule i to group j
			tmp_sum1=0;	 tmp_sum2=0;
			if(tmp_v==j){ //rule i is in group j
				cut_inc_mat[i][j] =0;
				continue;
			}
			for(int k=0; k<re_num; k++){
				if(re_groups[k]==tmp_v)
					tmp_sum1 = tmp_sum1 + re_inter_matrix[i][k];
				else if(re_groups[k]==j)
					tmp_sum2 = tmp_sum2 + re_inter_matrix[i][k];
			}
			cut_inc_mat[i][j] = tmp_sum1 - tmp_sum2;
			//if(DEBUG) fprintf(stdout,"cut_int_mat[%d][%d]=%d\n",i,j,cut_inc_mat[i][j]);
		}
	}
	last_inc=1; pass_num=0;
	while(last_inc>0){ //outer loop, each iteration is called a pass, keep running no more improvement is achieved in the last pass
		last_inc = 0; 	moved_re_num =0;
		for(int i=0; i<re_num; i++) //reset the b_moved marks
			b_moved[i]=false;
		if(DEBUG)	fprintf(stdout,"\n\n********the %dst pass*****\n\n",pass_num);

		for(int i=0; i<re_num; i++){ //inner loop, keep running until all rules have been moved or no unmoved cutsize increase is positive
			max_inc =0;
			for(int j=0; j<re_num; j++){ //find the max cutsize increase (j is rule id, k is group id)
				if(false==b_moved[j]){
					for(int k=0; k<group_num; k++){ 
						if(cut_inc_mat[j][k]>max_inc){
							max_inc = cut_inc_mat[j][k];
							rule_id = j; group_id = k;
						}
					}
				}
			}
			if(0==max_inc)
				break;	//go to next pass
			
			//move, mark and update
			org_group_id = re_groups[rule_id];
			re_groups[rule_id] = group_id;
			//if(DEBUG)	fprintf(stdout,"move rule %d to group %d\n",rule_id,group_id);
			b_moved[rule_id] = true;
			last_inc = last_inc + max_inc;
			//to avoid recompute all the cutsize increase values,  through detailed analysis, we found the following rules
			for(int l=0; l<re_num; l++){
				if(re_groups[l] == org_group_id){
					for(int m=0; m<group_num; m++){
						if(m==org_group_id)
							cut_inc_mat[l][m]=0;
						else if(m==group_id)
							cut_inc_mat[l][m] = cut_inc_mat[l][m] - 2*re_inter_matrix[l][rule_id];
						else
							cut_inc_mat[l][m] = cut_inc_mat[l][m] - re_inter_matrix[l][rule_id];
					}
				}
				else if(re_groups[l] == group_id){
					for(int m=0; m<group_num; m++){
						if(m==org_group_id)
							cut_inc_mat[l][m] = cut_inc_mat[l][m] + 2*re_inter_matrix[l][rule_id];
						else if(m==group_id)
							cut_inc_mat[l][m]=0;
						else
							cut_inc_mat[l][m] = cut_inc_mat[l][m] + re_inter_matrix[l][rule_id];
					}
				}
				else{
					cut_inc_mat[l][org_group_id] = cut_inc_mat[l][org_group_id] + re_inter_matrix[l][rule_id];
					cut_inc_mat[l][group_id] = cut_inc_mat[l][group_id] - re_inter_matrix[l][rule_id];
				}
			}
		}
		pass_num++;
	}
	//if (DEBUG) fprintf(stdout, "\nDEBUG3 in osg\n");
	//free resources
	for(int i=0; i<re_num; i++){
		if(NULL!=cut_inc_mat[i])	{free(cut_inc_mat[i]); cut_inc_mat[i]=NULL;}
	}
	if(NULL!=cut_inc_mat)	{fprintf(stdout,"cut_inc_mat is not NULL\n");	free(cut_inc_mat);	cut_inc_mat=NULL;}
	if(NULL!=b_moved)	{free(b_moved);	b_moved=NULL;}

	//if (DEBUG) fprintf(stdout, "\nDEBUG4 in osg\n");
	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of osg algorithm\n");
	for(int i=0; i<group_num; i++){
		fprintf(stdout,"\ngroup %d\nrules:	",i);
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i)
				fprintf(stdout,"%d	",j);
		}
		fprintf(stdout,"\n");
	}
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			; //dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	internal_inter =0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				internal_inter = internal_inter + re_inter_matrix[i][j];
		}
	}
	fprintf(stdout,"the total internal intersection value is %d\n",internal_inter);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);

	return true;
	
}

bool regex_parser::osg_hi_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	int **cut_inc_mat, tmp_sum1, tmp_sum2, tmp_v, moved_re_num, last_inc, max_inc, rule_id, group_id, org_group_id;
	int tmp_v1, tmp_v2, pass_num, internal_inter;
	bool *b_moved;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;
	//Initialization
	b_moved = allocate_bool_array(re_num);
	if(NULL==b_moved) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	cut_inc_mat = allocate_int_matrix(re_num);
	if(NULL==cut_inc_mat) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	for(int i=0; i<re_num; i++){
		cut_inc_mat[i] = allocate_int_array(group_num);
		if(NULL==cut_inc_mat[i]) {fprintf(stderr,"allocate failed in function ikl_group_algo\n"); exit(0);}
	}
	srand((unsigned)time(NULL));

	//initialize the groups with HI algorithm
	hi_group_algo(res, re_num, re_inter_matrix, re_groups, group_num, avg_size);
	if(DEBUG) fprintf(stdout,"\n\nthe initial random grouping: \n");
	for(int i=0; i<re_num; i++){
		if(DEBUG) fprintf(stdout,"rule %d in group %d  \n",i,re_groups[i]);
	}
	//if (DEBUG) fprintf(stdout, "\nDEBUG1 in osg re_num=%d, group_num=%d\n",re_num,group_num);
	//compute the cutsize increasement for each rule to each group(the increasement value maybe positive, zero or negative)
	for(int i=0; i<re_num; i++){
		tmp_v = re_groups[i];
		for(int j=0; j<group_num; j++){	//move rule i to group j
			tmp_sum1=0;	 tmp_sum2=0;
			if(tmp_v==j){ //rule i is in group j
				cut_inc_mat[i][j] =0;
				continue;
			}
			for(int k=0; k<re_num; k++){
				if(re_groups[k]==tmp_v)
					tmp_sum1 = tmp_sum1 + re_inter_matrix[i][k];
				else if(re_groups[k]==j)
					tmp_sum2 = tmp_sum2 + re_inter_matrix[i][k];
			}
			cut_inc_mat[i][j] = tmp_sum1 - tmp_sum2;
			//if(DEBUG) fprintf(stdout,"cut_int_mat[%d][%d]=%d\n",i,j,cut_inc_mat[i][j]);
		}
	}
	last_inc=1; pass_num=0;
	while(last_inc>0){ //outer loop, each iteration is called a pass, keep running no more improvement is achieved in the last pass
		last_inc = 0; 	moved_re_num =0;
		for(int i=0; i<re_num; i++) //reset the b_moved marks
			b_moved[i]=false;
		if(DEBUG)	fprintf(stdout,"\n\n********the %dst pass*****\n\n",pass_num);

		for(int i=0; i<re_num; i++){ //inner loop, keep running until all rules have been moved or no unmoved cutsize increase is positive
			max_inc =0;
			for(int j=0; j<re_num; j++){ //find the max cutsize increase (j is rule id, k is group id)
				if(false==b_moved[j]){
					for(int k=0; k<group_num; k++){ 
						if(cut_inc_mat[j][k]>max_inc){
							max_inc = cut_inc_mat[j][k];
							rule_id = j; group_id = k;
						}
					}
				}
			}
			if(0==max_inc)
				break;	//go to next pass
			
			//move, mark and update
			org_group_id = re_groups[rule_id];
			re_groups[rule_id] = group_id;
			//if(DEBUG)	fprintf(stdout,"move rule %d to group %d\n",rule_id,group_id);
			b_moved[rule_id] = true;
			last_inc = last_inc + max_inc;
			//to avoid recompute all the cutsize increase values,  through detailed analysis, we found the following rules
			for(int l=0; l<re_num; l++){
				if(re_groups[l] == org_group_id){
					for(int m=0; m<group_num; m++){
						if(m==org_group_id)
							cut_inc_mat[l][m]=0;
						else if(m==group_id)
							cut_inc_mat[l][m] = cut_inc_mat[l][m] - 2*re_inter_matrix[l][rule_id];
						else
							cut_inc_mat[l][m] = cut_inc_mat[l][m] - re_inter_matrix[l][rule_id];
					}
				}
				else if(re_groups[l] == group_id){
					for(int m=0; m<group_num; m++){
						if(m==org_group_id)
							cut_inc_mat[l][m] = cut_inc_mat[l][m] + 2*re_inter_matrix[l][rule_id];
						else if(m==group_id)
							cut_inc_mat[l][m]=0;
						else
							cut_inc_mat[l][m] = cut_inc_mat[l][m] + re_inter_matrix[l][rule_id];
					}
				}
				else{
					cut_inc_mat[l][org_group_id] = cut_inc_mat[l][org_group_id] + re_inter_matrix[l][rule_id];
					cut_inc_mat[l][group_id] = cut_inc_mat[l][group_id] - re_inter_matrix[l][rule_id];
				}
			}
		}
		pass_num++;
	}
	//if (DEBUG) fprintf(stdout, "\nDEBUG3 in osg\n");
	//free resources
	for(int i=0; i<re_num; i++){
		if(NULL!=cut_inc_mat[i])	{free(cut_inc_mat[i]); cut_inc_mat[i]=NULL;}
	}
	if(NULL!=cut_inc_mat)	{fprintf(stdout,"cut_inc_mat is not NULL\n");	free(cut_inc_mat);	cut_inc_mat=NULL;}
	if(NULL!=b_moved)	{free(b_moved);	b_moved=NULL;}

	//if (DEBUG) fprintf(stdout, "\nDEBUG4 in osg\n");
	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of osg algorithm\n");
	for(int i=0; i<group_num; i++){
		fprintf(stdout,"\ngroup %d\nrules:	",i);
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i)
				fprintf(stdout,"%d	",j);
		}
		fprintf(stdout,"\n");
	}
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			; //dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	internal_inter =0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				internal_inter = internal_inter + re_inter_matrix[i][j];
		}
	}
	fprintf(stdout,"the total internal intersection value is %d\n",internal_inter);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);

	return true;
	
}


bool regex_parser::rand_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	int tmp_v1, tmp_v2, internal_inter;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;

	srand((unsigned)time(NULL));
	for(int i=0; i<re_num; i++){	//initialize the groups
		re_groups[i] = rand()%group_num;
	}

	//printf the grouping resutls
	fprintf(stdout,"\nGrouping results of rand algorithm\n");
	for(int i=0; i<group_num; i++){
		fprintf(stdout,"\ngroup %d\nrules:	",i);
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i)
				fprintf(stdout,"%d	",j);
		}
		fprintf(stdout,"\n");
	}
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			; //dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	internal_inter =0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				internal_inter = internal_inter + re_inter_matrix[i][j];
		}
	}
	fprintf(stdout,"the total internal intersection value is %d\n",internal_inter);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);
	
	return true;
}

bool regex_parser::hi_group_algo(char **res, int re_num, int **re_inter_matrix, int* re_groups, int group_num, int* avg_size){
	double *ep;
	unsigned int tmp_id1, tmp_id2, tmp_v1, tmp_v2, *ep_order, *ep_group, *S, counter, internal_inter;
	bool *ep_mark;
	NFA *nfa=new NFA();
	NFA *non_anchored = nfa->add_epsilon(); // for .* RegEx
	NFA *anchored = nfa->add_epsilon(); // for anchored RegEx (^)
	DFA *dfa=NULL;

	ep = allocate_double_array(re_num);
	ep_order = allocate_uint_array(re_num);
	ep_mark = allocate_bool_array(re_num);
	ep_group = allocate_uint_array(group_num);
	S = allocate_uint_array(re_num);
	if(NULL==ep || NULL==ep_order || NULL==ep_group || NULL==ep_mark || NULL==S)	
		{fprintf(stdout,"allocate memory failed in function sa_hi_group_algo2\n"); exit(0);}

	//compute the state number for each rule
	for(int i=0; i<re_num; i++){
		parse_re(nfa,res[i]);
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			dfa->minimize();
		S[i] = dfa->size();

		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
	}

	//compute the expansion power for each rule
	for(int i=0; i<re_num; i++){
		ep[i]=0;
		for(int j=0; j<re_num; j++)
			ep[i] = ep[i] + (double)re_inter_matrix[i][j]/(double)S[j];
	}

	//sort the ep
	for(int i=0; i<re_num; i++)
		ep_mark[i] = false;
	tmp_v1=0;	tmp_v2=UINT_MAX;
	for(int i=0; i<re_num; i++){
		for(int j=0; j<re_num; j++){
			if(false==ep_mark[j]){
				if(ep[j]>=tmp_v1 && ep[j]<=tmp_v2){ //
					tmp_id1 = j;
					tmp_v1 = ep[j];
				}
			}
		}
		ep_order[i] = tmp_id1;
		ep_mark[tmp_id1]=true;
		tmp_v2 = tmp_v1;  //upper limitation for the next iteration
		tmp_v1 = 0;
	}

	//grouping
	for(int i=0; i<group_num; i++){
		ep_group[i] = ep[ep_order[i]];
		re_groups[ep_order[i]] = i;
		fprintf(stdout,"re_groups[%d]=%d",ep_order[i],i);
	}
	for(int i=group_num; i<re_num; i++){
		tmp_v1=UINT_MAX;
		for(int j=0; j<group_num; j++){
			if(ep_group[j]<tmp_v1){
				tmp_v1 = ep_group[j];
				tmp_id1 = j;
			}
		} 
		ep_group[tmp_id1] = ep_group[tmp_id1] + ep[ep_order[i]];
		re_groups[ep_order[i]] = tmp_id1;
		fprintf(stdout,"re_groups[%d]=%d\n",ep_order[i],tmp_id1);
	}
/*
	//printf grouping results
	fprintf(stdout,"\nGrouping results of hi algorithm\n");
	tmp_v1=0;	tmp_v2=0;
	for(int i=0; i<group_num; i++){
		for(int j=0; j<re_num; j++){
			if(re_groups[j]==i){
				parse_re(nfa,res[j]);
				tmp_v1++;
			}
		}
		if (m_modifier && (!anchored->get_epsilon()->empty() || !anchored->get_transitions()->empty())){//handle -m modifier
				non_anchored->add_transition('\n',anchored);
				non_anchored->add_transition('\r',anchored);
		}
		if(non_anchored->get_epsilon()->empty() && non_anchored->get_transitions()->empty()){//delete non_anchored, if necessary
			nfa->get_epsilon()->remove(non_anchored);
			delete non_anchored;
		}else{
			non_anchored->add_any(non_anchored);
		}
		nfa = nfa->get_first();
		nfa->remove_epsilon();
		nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa!=NULL)
			; //dfa->minimize();
		fprintf(stdout,"Group %d, rule num: %d, dfa state num: %d\n",i,tmp_v1,dfa->size());
		tmp_v2 = tmp_v2 + dfa->size();
		if (nfa!=NULL) delete nfa; nfa=new NFA(); non_anchored = nfa->add_epsilon(); anchored = nfa->add_epsilon();
		if (dfa!=NULL) delete dfa; dfa=NULL;
		tmp_v1=0;
	}
	internal_inter =0;
	for(int i=1; i<re_num; i++){
		for(int j=0; j<i; j++){
			if(re_groups[i]==re_groups[j])
				internal_inter = internal_inter + re_inter_matrix[i][j];
		}
	}
	fprintf(stdout,"the total internal intersection value is %d\n",internal_inter);
	fprintf(stdout,"The upper limitation of a single DFA size is %d\n",MAX_DFA_SIZE);
	fprintf(stdout,"In total, %d rules are divided into %d groups, total dfa state num %d\n",re_num,group_num,tmp_v2);
	*avg_size = tmp_v2/(group_num);
*/	
	return true;	
}


dfa_set *regex_parser::parse_to_multi_dfas(FILE *file, int group_num){ //Grouping algorithms added by Chengcheng Xu, 20161020
	char** res; //regex rules
	int re_num, avg_size;
	int **re_inter_matrix; //record the intersection values among res
	int *re_groups;
	long time_start, time_comp_inter, time_read_inter, time_yu, time_sa, time_sa_hi, time_ga, time_osg, time_osg_hi, time_rand, total_inter_value;

	re_num = num_regex(file);
	re_groups = allocate_int_array(re_num);
	if(NULL==re_groups) {{fprintf(stderr,"allocate failed in function read_re_to_array\n"); exit(0);}}
	//step 1. read the regexs from file to array
	if (DEBUG) fprintf(stdout, "\nread patterns from file to array\n");
	res = allocate_string_array(re_num);
	if(NULL==res) {fprintf(stderr,"allocate failed in function read_re_to_array\n"); exit(0);}
	for(int i=0; i<re_num; i++){
		res[i] = allocate_char_array(MAX_PATTERN_LEN); //default max re len 512
		if(NULL==res[i]) {fprintf(stderr,"allocate failed in function read_re_to_array\n"); exit(0);}
	}
	read_re_to_array(file, res, re_num);	

	//step 2. compute the intersection values
	re_inter_matrix = allocate_int_matrix(re_num);
	if(NULL==re_inter_matrix) {fprintf(stderr,"allocate failed in function read_re_to_array\n"); exit(0);}
	for(int i=0; i<re_num; i++){
		re_inter_matrix[i] = allocate_int_array(re_num);
		if(NULL==re_inter_matrix[i]) {fprintf(stderr,"allocate failed in function read_re_to_array\n"); exit(0);}
	}
	srand((unsigned)time(NULL));
	time_start = time(NULL);
	//compute_intersection(res,re_num,re_inter_matrix);
	time_comp_inter = time(NULL); 
	if(DEBUG) fprintf(stdout,"time for compute the interaction matrix: %ld s\n",time_comp_inter-time_start);

	//or read the intersection matrix from the file nonneg_inter_matrix
	FILE *infile = fopen("nonneg_inter_matrix","r");
	if(NULL==infile) {fprintf(stderr,"open file nonneg_inter_matrix error!\n"); exit(0);}
	for(int i=0; i<re_num; i++){
		for(int j=0; j<re_num; j++){
			fscanf(infile,"%d",&re_inter_matrix[i][j]);
			total_inter_value = total_inter_value + re_inter_matrix[i][j];
		}
	}
	fclose(infile);
	
	time_read_inter = time(NULL);
	if(DEBUG) fprintf(stdout,"the nonnegative interaction matrix has been read from file\n");
	if(DEBUG) fprintf(stdout,"total intersection value among all rules is %ld \n",total_inter_value);
	
	//step 2. 1 Yu's grouping algorithm
/*	yu_group_algo(res,re_num,re_inter_matrix,re_groups,&avg_size);
	time_yu = time(NULL);
	if(DEBUG) fprintf(stdout,"time for yu's grouping algorithm: %ld s",time_yu-time_read_inter);
*/

	//step 2.2 Simulated annealing algorithm
/*	sa_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	//sa2_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_sa = time(NULL);
	if(DEBUG) fprintf(stdout,"time for sa grouping algorithm: %ld s",time_sa-time_read_inter);
*/
	//step 2.2.1 simulated annealing + heuristic inilization
/*	sa_hi_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_sa_hi = time(NULL);
	if(DEBUG) fprintf(stdout,"time for sa grouping algorithm: %ld s",time_sa_hi-time_read_inter);
*/
/*
	//step 2.3 Genetic Algorithm
	ga_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_ga = time(NULL);
	if(DEBUG) fprintf(stdout,"time for ga grouping algorithm: %ld s\n",time_ga-time_read_inter); 
*/

	//step 2.4 one-step greedy (OSG) algorithm
/*	osg_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_osg = time(NULL);
	if(DEBUG) fprintf(stdout,"time for osg grouping algorithm: %ld s\n",time_osg-time_read_inter);
*/
	//step 2.4.1 one-step greedy (OSG) + HI
	osg_hi_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_osg_hi = time(NULL);
	if(DEBUG) fprintf(stdout,"time for osg grouping algorithm: %ld s\n",time_osg_hi-time_read_inter);

	//step 2.5 random grouping algorithm
/*	rand_group_algo(res,re_num,re_inter_matrix,re_groups,group_num,&avg_size);
	time_rand = time(NULL);
	if(DEBUG) fprintf(stdout,"time for random grouping algorithm: %ld s\n",time_rand - time_read_inter);
*/	
	for(int i=0; i<re_num; i++){
		if(res[i]!=NULL)
			free(res[i]);
	}
	if(res!=NULL) free(res);
	

	return NULL;
}


void *regex_parser::parse_re(NFA* nfa, const char *re){
	int ptr=0;
	bool tilde_re=false;
	NFA *non_anchored = *(nfa->get_epsilon()->begin());
	NFA *anchored = *(++nfa->get_epsilon()->begin());

	//check whether the text must match at the beginning of the regular expression
	if (re[ptr]==TILDE){
		tilde_re=true;
		ptr++;
	}
	NFA *fa=parse_re(re,&ptr,false);	
	fa->get_last()->accept();
	if (!tilde_re){ 
		non_anchored->add_epsilon(fa->get_first());
	}else{
		anchored->add_epsilon(fa->get_first());
	}
}

NFA *regex_parser::parse_re(const char *re, int *ptr, bool bracket){
	NFA *fa=new NFA();
	NFA *to_link=NULL;
	bool open_b=bracket;
	bool close_b=false;
	while((*ptr)<strlen(re)){
		if(re[(*ptr)]==ESCAPE){			
			int_set *chars=new int_set(CSIZE);
			(*ptr)=process_escape(re, (*ptr)+1,chars);
			if((*ptr)==strlen(re)||!is_repetition(re[(*ptr)])){   //the end of pattern or not repetition
				fa=fa->add_transition(chars);
			}else{    //repetition???
				to_link=new NFA();
				to_link=to_link->add_transition(chars);
			}	
			delete chars;
		}else if (!is_special(re[(*ptr)]) && ((*ptr)==(strlen(re)-1)||!is_repetition(re[(*ptr)+1]))){
			fa=fa->add_transition(re[(*ptr)++]);
		}else if(!is_special(re[(*ptr)])){   //for conditions that next symbol is repetitions
			to_link=new NFA();
			to_link=to_link->add_transition(re[(*ptr)++]);
		}else if (re[(*ptr)]==ANY && ((*ptr)==(strlen(re)-1)||!is_repetition(re[(*ptr)+1]))){
			fa=fa->add_any();   //any with no repetitions
			(*ptr)++;
		}else if(re[(*ptr)]==ANY){   //any with repetitions
			to_link=new NFA();
			to_link=to_link->add_any();
			(*ptr)++;
		}else if (re[(*ptr)]==STAR){
			(*ptr)++;
			if (close_b)
				return fa->make_rep(0,_INFINITY);
			else{	
				to_link=to_link->make_rep(0,_INFINITY);
				fa=fa->link(to_link);			
			}
		}else if (re[(*ptr)]==OPT){
			(*ptr)++;
			if (close_b)
				return fa->make_rep(0,1);
			else{	
				to_link=to_link->make_rep(0,1);
				fa=fa->link(to_link);			
			}
		}else if (re[(*ptr)]==PLUS){
			(*ptr)++;
			if (close_b){
				return fa->make_rep(1,_INFINITY);
			}else{
				to_link=to_link->make_rep(1,_INFINITY);
				fa=fa->link(to_link);			
			}
		}else if(re[(*ptr)]==OPEN_QBRACKET){
			if ((*ptr)==(strlen(re)-1))
				fatal("regex_parser:: parse_re: { in last position.");
			else{
				int lb=0; int ub=_INFINITY;	
				(*ptr)=process_quantifier(re,(*ptr)+1,&lb,&ub);
				if (close_b)
					return fa->make_rep(lb,ub);
				else{	
					to_link=to_link->make_rep(lb,ub);
					fa=fa->link(to_link);			
				}	
			}	
		}else if(re[(*ptr)]==OPEN_SBRACKET){
			if ((*ptr)==(strlen(re)-1))
				fatal("regex_parser:: parse_re: [ in last position.");
			else	
				(*ptr)=process_range(&fa,&to_link,re,(*ptr)+1);
		}else if(re[(*ptr)]==OR){
			(*ptr)++;
			fa=fa->make_or(parse_re(re,ptr,false));
		}else if(re[(*ptr)]==OPEN_RBRACKET){
			(*ptr)++;
			fa=fa->get_last()->link(parse_re(re,ptr,true));
		}else if(re[(*ptr)]==CLOSE_RBRACKET){
			if (open_b){
				close_b=true;
				(*ptr)++;
				if ((*ptr)==strlen(re) || !is_repetition(re[(*ptr)]))
					return fa;
			}
			//fatal("parse:: parse_re : close ) without opening it.");
			else{
				return fa;			
			}	
		}
	}
	return fa->get_first();
}

int regex_parser::process_escape(const char *re, int ptr, int_set *chars){
	if (ptr==strlen(re)){
		return (++ptr);
		//fatal("regex_parser:: process_escape: \\ in last position.");
	}
	char c=re[ptr];
	int next;
	if(is_x(c)){	//hex
		if(ptr>strlen(re)-3)	//ptr>strlen(re)-3 right??  why not ptr>strlen(re)-2    by xu;    answer: right, every expression has a end symbol, which is not displayed but can be read
 			fatal("regex_parser::process_escape: invalid hex escape sequence.");
		else if (!is_hex_digit(re[ptr+1]) || !is_hex_digit(re[ptr+2]))
			fatal("regex_parser::process_escape: invalid hex escape sequence.");
		else{
			char tmp[5];
			tmp[0]='0';tmp[1]=c;tmp[2]=re[ptr+1];tmp[3]=re[ptr+2]; tmp[4]='\0';
			sscanf(tmp,"0x%x", &next);
			chars->insert(next);
			ptr=ptr+3;
		}
	}else if (is_oct_digit(c)){		//octal, \ddd denotes Three octal
		if(ptr>strlen(re)-3)
			{next=escaped(c);ptr++;chars->insert(next);} //normal escape sequence
		else if (!is_oct_digit(re[ptr+1]) || !is_oct_digit(re[ptr+2]))
			{next=escaped(c);ptr++;chars->insert(next);} //normal escape sequence
		else{
			//really an octal sequence!
			char tmp[5];
			tmp[0]='0';tmp[1]=c;tmp[2]=re[ptr+1];tmp[3]=re[ptr+2]; tmp[4]='\0';
			sscanf(tmp,"0%o", &next);
			chars->insert(next);
			ptr=ptr+3;
		}
	}else if(c=='s'){
		chars->insert('\t');
		chars->insert('\n');
		chars->insert('\r');
		chars->insert('\x0C');    //  \f  , form feed
		chars->insert('\x20');    //  space
		ptr++;
	}else if(c=='S'){
		chars->insert('\t');
		chars->insert('\n');
		chars->insert('\r');
		chars->insert('\x0C');    
		chars->insert('\x20');	  
		chars->negate();
		ptr++;
	}else if(c=='d'){
		chars->insert('0');chars->insert('1');chars->insert('2');
		chars->insert('3');chars->insert('4');chars->insert('5');
		chars->insert('6');chars->insert('7');chars->insert('8');
		chars->insert('9');
		ptr++;
	}else if(c=='D'){
		chars->insert('0');chars->insert('1');chars->insert('2');
		chars->insert('3');chars->insert('4');chars->insert('5');
		chars->insert('6');chars->insert('7');chars->insert('8');
		chars->insert('9');
		chars->negate();
		ptr++;
	}else if(c=='w'){
		chars->insert('_');
		chars->insert('0');chars->insert('1');chars->insert('2');
		chars->insert('3');chars->insert('4');chars->insert('5');
		chars->insert('6');chars->insert('7');chars->insert('8');
		chars->insert('9');
		chars->insert('a');chars->insert('b');chars->insert('c');
		chars->insert('d');chars->insert('e');chars->insert('f');
		chars->insert('g');chars->insert('h');chars->insert('i');
		chars->insert('j');chars->insert('k');chars->insert('l');
		chars->insert('m');chars->insert('n');chars->insert('o');
		chars->insert('p');chars->insert('q');chars->insert('r');
		chars->insert('s');chars->insert('t');chars->insert('u');
		chars->insert('v');chars->insert('w');chars->insert('x');
		chars->insert('y');chars->insert('z');
		chars->insert('A');chars->insert('B');chars->insert('C');
		chars->insert('D');chars->insert('E');chars->insert('F');
		chars->insert('G');chars->insert('H');chars->insert('I');
		chars->insert('J');chars->insert('K');chars->insert('L');
		chars->insert('M');chars->insert('N');chars->insert('O');
		chars->insert('P');chars->insert('Q');chars->insert('R');
		chars->insert('S');chars->insert('T');chars->insert('U');
		chars->insert('V');chars->insert('W');chars->insert('X');
		chars->insert('Y');chars->insert('Z');
		ptr++;
	}else if(c=='W'){
		chars->insert('_');
		chars->insert('0');chars->insert('1');chars->insert('2');
		chars->insert('3');chars->insert('4');chars->insert('5');
		chars->insert('6');chars->insert('7');chars->insert('8');
		chars->insert('9');
		chars->insert('a');chars->insert('b');chars->insert('c');
		chars->insert('d');chars->insert('e');chars->insert('f');
		chars->insert('g');chars->insert('h');chars->insert('i');
		chars->insert('j');chars->insert('k');chars->insert('l');
		chars->insert('m');chars->insert('n');chars->insert('o');
		chars->insert('p');chars->insert('q');chars->insert('r');
		chars->insert('s');chars->insert('t');chars->insert('u');
		chars->insert('v');chars->insert('w');chars->insert('x');
		chars->insert('y');chars->insert('z');
		chars->insert('A');chars->insert('B');chars->insert('C');
		chars->insert('D');chars->insert('E');chars->insert('F');
		chars->insert('G');chars->insert('H');chars->insert('I');
		chars->insert('J');chars->insert('K');chars->insert('L');
		chars->insert('M');chars->insert('N');chars->insert('O');
		chars->insert('P');chars->insert('Q');chars->insert('R');
		chars->insert('S');chars->insert('T');chars->insert('U');
		chars->insert('V');chars->insert('W');chars->insert('X');
		chars->insert('Y');chars->insert('Z');
		chars->negate();
		ptr++;										
	}else{
		next=escaped(c);
		chars->insert(next);
		ptr++;
	}
	return ptr;
}

int regex_parser::process_quantifier(const char *re, int ptr, int *lb_p, int *ub_p){
	int lb=0;
	int ub=_INFINITY;
	int res=sscanf(re+ptr,"%d",&lb);
	if (res!=1) fatal("regex_parser:: process_quantifier: wrong quantified expression.");
	while(ptr!=strlen(re) && re[ptr]!=COMMA && re[ptr]!=CLOSE_QBRACKET)
		ptr++;
	if (ptr==strlen(re) || (re[ptr]==COMMA && ptr==strlen(re)-1))	
		fatal("regex_parser:: process_quantifier: wrong quantified expression.");
	if(re[ptr]==CLOSE_QBRACKET){
		ub=lb;
	}else{
		ptr++;
		if(re[ptr]!=CLOSE_QBRACKET){	
			res=sscanf(re+ptr,"%d}",&ub);
			if (res!=1) fatal("regex_parser:: process_quantifier: wrong quantified expression.");
		}
	}
	while(re[ptr]!=CLOSE_QBRACKET)
		ptr++;
	(*lb_p)=lb;
	(*ub_p)=ub;
	ptr++;
	return ptr;
}

int regex_parser::process_range(NFA **fa, NFA **to_link, const char *re, int ptr){
	if (re[ptr]==CLOSE_SBRACKET)
		fatal("regex_parser:: process_range: empty range.");
	bool negate=false;
	int from=CSIZE+1;
	int_set *range=new int_set(CSIZE);
	if(re[ptr]==TILDE){
		negate=true;
		ptr++;
	}
	while(ptr!=strlen(re)-1 && re[ptr]!=CLOSE_SBRACKET){
		symbol_t to = re[ptr];
		//if (is_special(to) && to!=ESCAPE)
		//	fatal("regex_parser:: process_range: invalid character.");
		if (to==ESCAPE){
			int_set *chars=new int_set(CSIZE);
			ptr=process_escape(re,ptr+1,chars);
			to=chars->head();
			delete chars;
		}
		else
			ptr++;
		if (from==(CSIZE+1)) from=to;	
		if (ptr!=strlen(re)-1 && re[ptr]==MINUS_RANGE){ 
			ptr++;	
		}else{
			if (from>to)
				fatal("regex_parser:: process_range: invalid range.");
			for(symbol_t i=from;i<=to;i++){
				range->insert(i);
				if (i==255) break;
			}
			from=CSIZE+1;	
		}			
	}	
	if (re[ptr]!=CLOSE_SBRACKET)
		fatal("regex_parser:: process_range: range not closed.");
	ptr++;	
	if (i_modifier){
		int_set *is=new int_set(CSIZE);
		for (unsigned v=range->head();v!=UNDEF;v=range->suc(v)){
			if (v>='A' && v<='Z') is->insert(v+('a'-'A')); 
			if (v>='a' && v<='z') is->insert(v-('a'-'A'));
		}
		range->add(is);
		delete is;
	}
	if (negate) range->negate();
	if(ptr==strlen(re)||!is_repetition(re[ptr])){
		(*fa)=(*fa)->add_transition(range);
	}else{
		(*to_link)=new NFA();
		(*to_link)=(*to_link)->add_transition(range);
	}
	delete range;
	return ptr;
}
