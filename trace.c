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
 * File:   trace.c
 * Author: Michela Becchi
 * Email:  mbecchi@cse.wustl.edu
 * Organization: Applied Research Laboratory
 */

#include "trace.h"
#include "dheap.h"

#include <set>
#include <iterator>
#include <pcap.h>

#define SWAPLONG(y) ((((y)&0xff)<<24)|(((y)&0xff00)<<8)|(((y)&0xff0000)>>8)|(((y)>>24)&0xff))   
#define SWAPSHORT(y) ((((y)&0xff)<<8)|((u_short)((y)&0xff00)>>8))



using namespace std;

trace::trace(char *filename){
	tracefile=NULL;
	if (filename!=NULL) set_trace(filename);
	else tracename=NULL;
}
	
trace::~trace(){
	if (tracefile!=NULL) fclose(tracefile);
}

void trace::set_trace(char *filename){
	if (tracefile!=NULL) fclose(tracefile);
	tracename=filename;
	tracefile=fopen(tracename,"r");	//以只读的方式打开文本文件
	//tracefile=fopen(tracename,"rb");	//以只读的方式打开二进制文件
	if (tracefile==NULL) fatal("trace:: set_trace: error opening trace-file\n");
}
	
void trace::traverse(DFA *dfa, FILE *stream){
	if (tracefile==NULL) fatal("trace file is NULL!");
	rewind(tracefile);
	
	if (VERBOSE) fprintf(stderr, "\n=>trace::traverse DFA on file %s\n...",tracename);
	if (dfa->get_depth()==NULL) dfa->set_depth();
	unsigned int *dfa_depth=dfa->get_depth();
	
	//读取tcpdump格式的文件，并对文件进行遍历
	//首先校验文件头，判断是否是tcpdump格式的文件
	//.dmp文件开头是(D4 C3 B2 A1)
	if((unsigned char)fgetc(tracefile)!=212||(unsigned char)fgetc(tracefile)!=195
		||(unsigned char)fgetc(tracefile)!=178||(unsigned char)fgetc(tracefile)!=161)	
		fatal("trace:: traverse: the trace file is not tcpdump format\n");
	//检查文件头的主次版本号
	unsigned short major_v,minor_v;
	fread((char*)(&major_v),sizeof(unsigned short),1,tracefile);	//major_v=SWAPSHORT(major_v);
	fread((char*)(&minor_v),sizeof(unsigned short),1,tracefile);	//minor_v=SWAPSHORT(minor_v);
//	printf("\nthe version %hu.%hu\n",major_v,minor_v);
	if(major_v!=2||minor_v!=4)
		fatal("\ntrace:: traverse: the version is not 0204\n");
	//检查数据链路类型是否是以太网
	fseek(tracefile,12,SEEK_CUR);
	unsigned int ilink_type;
	fread((char*)(&ilink_type),sizeof(unsigned int),1,tracefile);	//ilink_type=SWAPLONG(ilink_type);
	//printf("\nthe link_type is %d\n",ilink_type);
	if(ilink_type!=1)
		fatal("\ntrace:: traverse: the link in tracefile is not Ethernet\n");
	//依次遍历各个数据包
	unsigned int eth_len, ip_len, pay_len, proc_pkt_num, total_pkt_num;
	unsigned char ip_head_len,ip_ver,trans_prot,tcp_head_len;
	unsigned short net_prot;

	state_t state=0;
	long inputs=0;
	proc_pkt_num=0;	 total_pkt_num=0;
	int c;
	unsigned int *stats=allocate_uint_array(dfa->size());
	for (int j=0;j<dfa->size();j++) stats[j]=0;
	linked_set *accepted_rules=new linked_set();

	while(feof(tracefile)==0)	//注意feof()只有再读取最后一个字符的下一个字符时才会返回非0,因此下面用fgetc辅助判断
	{
		if(fgetc(tracefile)==EOF)	{printf("end of file\n");   break;}	//到达文件尾，退出
		total_pkt_num++;
		fseek(tracefile,7,SEEK_CUR);	//跳过8个字节(上一句已经读过一个字节)，到当前包头的报文长度字段
		fread((char*)(&eth_len),sizeof(unsigned int),1,tracefile);	//eth_len=SWAPLONG(eth_len);
		printf("debug: packet%u eth_len %u\n",total_pkt_num,eth_len);
		fseek(tracefile,16,SEEK_CUR);	//跳过16个字节，到以太网的类型字段（<1500是802.3；>1500是Ethernet2）
		fread((char*)(&net_prot),sizeof(unsigned short),1,tracefile);	net_prot=SWAPSHORT(net_prot);
		if(net_prot==0x0800);	//printf("ip	");	//以太网E2+IP
		else  //不是以太网E2+IP就跳到下一个数据包
			{fseek(tracefile,eth_len-14,SEEK_CUR);	printf("net_prot %X not Ethernet2\n",net_prot); continue;}
		//printf("DEBUG1\n");
		fread((char*)(&ip_head_len),1,1,tracefile);
		ip_ver=(ip_head_len&0xf0)>>4;	ip_head_len=ip_head_len&0x0f;
		if(ip_ver!=4)
		{	
			//printf("not ipv4\n");	//不是ipv4,跳到下一个包
			fseek(tracefile,eth_len-15,SEEK_CUR);
			continue;
		}
		fseek(tracefile,8,SEEK_CUR);	//跳过8个字节，到传输层协议号
		fread((char*)(&trans_prot),1,1,tracefile);
		if(trans_prot==6)  //TCP
		{
			//printf("DEBUG2\n");
			//printf("tcp\n");
			fseek(tracefile,ip_head_len*4-10+12,SEEK_CUR);	//跳过剩余的IP头和部分TCP头，到TCP首部长度字段
			fread((char*)(&tcp_head_len),1,1,tracefile);
			tcp_head_len = (tcp_head_len&0xf0)>>4;   //注意tcp_head_len和ip_head_len都是以4字节为单位
			fseek(tracefile,tcp_head_len*4-13,SEEK_CUR);  //跳过TCP头,到负载
			pay_len = eth_len-14-ip_head_len*4-tcp_head_len*4 ;	//应用层负载长度
		}
		else if(trans_prot==17)  //UDP
		{	//printf("udp\n");	
			fseek(tracefile,ip_head_len*4-10+8,SEEK_CUR);  //跳过UDP头
			pay_len = eth_len-14-ip_head_len*4-8;
		}
		else	//不是TCP或UDP，跳到下一个包
		{fseek(tracefile,eth_len-14-10,SEEK_CUR);  /*printf("not tcp or udp\n");*/  continue;}
		//下面是对报文负载进行正则表达式匹配
		//printf("DEBUG3 pay_len: %d\n",pay_len);
		state=0;
		stats[0]++;
		proc_pkt_num++;
		for(int j=0;j<pay_len;j++)
		{
			c=fgetc(tracefile);
			state=dfa->get_next_state(state,(unsigned char)c);
			stats[state]++;
			if (!dfa->accepts(state)->empty()){
				//printf("DEBUG4\n");
				accepted_rules->add(dfa->accepts(state));
				/*if (DEBUG){
					char *label=NULL;  
					linked_set *acc=dfa->accepts(state);
					while(acc!=NULL && !acc->empty()){
						if (label==NULL){
							label=(char *)malloc(100);
							sprintf(label,"%d",acc->value());
						}else{
							char *tmp=(char *)malloc(5);
							sprintf(tmp,",%d",acc->value());
							label=strcat(label,tmp); 
							free(tmp);
						}
						acc=acc->succ();
					}
					fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
					free(label);
				}*/
			}
			inputs++;
			//printf("DEBUG5\n");
		}
		stats[state]--;	//每个报文最后一个字节对应的状态并没有被访问
	}//END while
	if(feof(tracefile)==0)	printf("\ndebug: the end of file\nS");
	printf("total packet num:%u    processed packet num:%u   processed byte num:%u\n",total_pkt_num,proc_pkt_num,inputs);
	//inputs = inputs + proc_pkt_num;	//注意这里有对inputs的一次校正
	fprintf(stream,"\ntraversal statistics:: [state #, depth, # traversals, %%time]\n");
	unsigned int num=0;
	for (int j=0;j<dfa->size();j++){
		if(stats[j]!=0){

			fprintf(stream,"[%ld, %ld, %ld, %f %%]\n",j,dfa_depth[j],stats[j],(float)stats[j]*100/inputs);
			num++;
		}
	}
	fprintf(stream,"%ld out of %ld states traversed (%f %%)\n",num,dfa->size(),(float)num*100/dfa->size());
	fprintf(stream,"rules matched: %ld\n",accepted_rules->size());	
	free(stats);		
	delete accepted_rules;
}

void trace::traverse_compressed(DFA *dfa, FILE *stream){
	if (tracefile==NULL) fatal("trace file is NULL!");
	rewind(tracefile);
	
	if (VERBOSE) fprintf(stderr, "\n=>trace::traverse compressed DFA on file %s\n...",tracename);
	
	if (dfa->get_depth()==NULL) dfa->set_depth();
	unsigned int *dfa_depth=dfa->get_depth();
	
	unsigned int accesses=0;
	unsigned int *stats=allocate_uint_array(dfa->size()); 	  //state traversals (including the ones due to default transitions)
	unsigned int *dfa_stats=allocate_uint_array(dfa->size()); //state traversals in the original DFA
	for (int j=0;j<dfa->size();j++){
		stats[j]=0;
		dfa_stats[j]=0;
	}
	dfa_stats[0]++;
	stats[0]++;
	
	linked_set *accepted_rules=new linked_set();
	state_t *fp=dfa->get_default_tx();
	int *is_fp=new int[dfa->size()];
	for (state_t s=0;s<dfa->size();s++) is_fp[s]=0;
	for (state_t s=0;s<dfa->size();s++) if(fp[s]!=NO_STATE) is_fp[fp[s]]=1;
	
	unsigned int inputs=0;
	state_t state=0;
	int c=fgetc(tracefile);
	while(c!=EOF){
		state=dfa->lookup(state,c,&stats,&accesses);
		dfa_stats[state]++;
		if (!dfa->accepts(state)->empty()){
			accepted_rules->add(dfa->accepts(state));
			if (DEBUG){
				char *label=NULL;  
				linked_set *acc=dfa->accepts(state);
				while(acc!=NULL && !acc->empty()){
					if (label==NULL){
						label=(char *)malloc(100);
						sprintf(label,"%d",acc->value());
					}else{
						char *tmp=(char *)malloc(5);
						sprintf(tmp,",%d",acc->value());
						label=strcat(label,tmp); 
						free(tmp);
					}
					acc=acc->succ();
				}
				fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
				free(label);
			}
		}
		inputs++;
		c=fgetc(tracefile);
	}
	fprintf(stream,"\ntraversal statistics:: [state #, depth, #traversal, (# traversals in DFA), %%time]\n");
	fprintf(stream,"                         - state_id*: target of default transition - not taken\n");
	fprintf(stream,"                         - *: target of default transition - taken \n");
	
	int num=0;
	int more_states=0;
	int max_depth=0;
	for (int j=0;j<dfa->size();j++){
		if(stats[j]!=0){
			if (dfa_depth[j]>max_depth) max_depth=dfa_depth[j];
			if (is_fp[j]){
				if (dfa_stats[j]==stats[j])
					fprintf(stream,"[%ld*, %ld, %ld, %f %%]\n",j,dfa_depth[j],stats[j],(float)stats[j]*100/accesses);
				else{
					fprintf(stream,"[%ld**, %ld, %ld, %ld, %f %%]\n",j,dfa_depth[j],stats[j],dfa_stats[j],(float)stats[j]*100/accesses);
					more_states++;
				}	
			}
			else fprintf(stream,"[%ld, %ld, %ld, %f %%]\n",j,dfa_depth[j],stats[j],(float)stats[j]*100/accesses);
			num++;
		}
	}
	fprintf(stream,"%ld out of %ld states traversed (%f %%)\n",num,dfa->size(),(float)num*100/dfa->size());
	fprintf(stream,"rules matched: %ld\n",accepted_rules->size());
	fprintf(stream,"fraction traversals %f\n",(float)accesses/inputs);
	fprintf(stream,"number of additional states %ld\n",more_states);
	fprintf(stream,"max depth reached %ld\n",max_depth);	
	free(stats);
	free(dfa_stats);
	delete [] is_fp;		
	delete accepted_rules;
}

/*
void trace::traverse(NFA *nfa, FILE *stream){
	if (tracefile==NULL) fatal("trace file is NULL!");
	rewind(tracefile);
	
	if (VERBOSE) fprintf(stderr, "\n=>trace::traverse NFA on file %s\n...",tracename);
	nfa->reset_state_id(); //reset state identifiers in breath-first order
	
	//statistics
	unsigned int *active=allocate_uint_array(nfa->size()); //active[x]=y if x states are active for y times
	unsigned int *stats=allocate_uint_array(nfa->size()); //stats[x]=y if state x was active y times
	for (int j=0;j<nfa->size();j++){
		stats[j]=0;
		active[j]=0;
	}
	
	//code
	nfa_set *nfa_state=new nfa_set();
	nfa_set *next_state=new nfa_set();
	linked_set *accepted_rules=new linked_set();
	
	nfa_set *closure = nfa->epsilon_closure(); 
	
	nfa_state->insert(closure->begin(),closure->end());
	delete closure;
	
	FOREACH_SET (nfa_state,it) stats[(*it)->get_id()]=1;
	active[nfa_state->size()]=1;
	
	int inputs=0;
	int c=fgetc(tracefile);	
	while(c!=EOF){
		FOREACH_SET(nfa_state,it){
			nfa_set *target=(*it)->get_transitions(c);
			if (target!=NULL){
				FOREACH_SET(target,it2){
					nfa_set *target_closure=(*it2)->epsilon_closure();	
					next_state->insert(target_closure->begin(),target_closure->end());
					delete target_closure;
				}
	            delete target;        	   
			}
		}
		delete nfa_state;
		nfa_state=next_state;
		next_state=new nfa_set();
		active[nfa_state->size()]++;
		FOREACH_SET (nfa_state,it) stats[(*it)->get_id()]++;
		
		linked_set *rules=new linked_set();
		
		FOREACH_SET (nfa_state,it) rules->add((*it)->get_accepting());
				
		if (!rules->empty()){
			accepted_rules->add(rules);
			if (DEBUG){
				char *label=(char *)malloc(100);
				sprintf(label,"%d",rules->value());
				linked_set *acc=rules->succ();
				while(acc!=NULL && !acc->empty()){
					char *tmp=(char *)malloc(5);
					sprintf(tmp,",%d",acc->value());
					label=strcat(label,tmp); free(tmp);
					acc=acc->succ();
				}
				fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
				free(label);
			}
		}
		delete rules;
		inputs++;
		c=fgetc(tracefile);
	}//end while (traversal)
	
	fprintf(stream,"\ntraversal statistics:: [state #,#traversal, %%time]\n");
	unsigned long num=0;
	for (int j=0;j<nfa->size();j++)
		if(stats[j]!=0){
			fprintf(stream,"[%ld,%ld,%f %%]\n",j,stats[j],(float)stats[j]*100/inputs);
			num++;
		}
	fprintf(stream,"%ld out of %ld states traversed (%f %%)\n",num,nfa->size(),(float)num*100/nfa->size());
	fprintf(stream,"\ntraversal statistics:: [size of active state vector #,frequency, %%time]\n");
	num=0;
	for (int j=0;j<nfa->size();j++)
		if(active[j]!=0){
			fprintf(stream,"[%ld,%ld,%f %%]\n",j,active[j],(float)active[j]*100/inputs);
			num+=j*active[j];
		}
	fprintf(stream,"average size of active state vector %f\n",(float)num/inputs);
	fprintf(stream,"rules matched: %ld\n",accepted_rules->size());	
	free(stats);		
	free(active);
	delete nfa_state;
	delete next_state;
	delete accepted_rules;
}
*/

void trace::traverse(NFA *nfa, FILE *stream){
	if (tracefile==NULL) fatal("trace file is NULL!");
	rewind(tracefile);
	
	if (VERBOSE) fprintf(stderr, "\n=>trace::traverse NFA on file %s\n...",tracename);
	nfa->reset_state_id(); //reset state identifiers in breath-first order
	
	//statistics
	unsigned int *active=allocate_uint_array(nfa->size()); //active[x]=y if x states are active for y times
	unsigned int *stats=allocate_uint_array(nfa->size()); //stats[x]=y if state x was active y times
	unsigned int *copy_nfa_stats=allocate_uint_array(nfa->size());
	state_t *nfa_access_order=allocate_uint_array(nfa->size());
	unsigned int *nfa_depth=allocate_uint_array(nfa->size());
	for (int j=0;j<nfa->size();j++){
		stats[j]=0;
		active[j]=0;
		copy_nfa_stats[j]=0;
		nfa_access_order[j]=NO_STATE;
		nfa_depth[j]=0;
	}
	
	//code
	//读取tcpdump格式的文件，并对文件进行遍历
	//首先校验文件头，判断是否是tcpdump格式的文件(A1,B2,C3,D4)
	//.dmp文件开头是(D4 C3 B2 A1),  pcap文件开头是(D4,C3,B2,A1)
//	printf("\nfile type %d %d %d %d\n",(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile));
	//for(D4,C3,B2,A1)
//	if((unsigned char)fgetc(tracefile)!=212||(unsigned char)fgetc(tracefile)!=195
//		||(unsigned char)fgetc(tracefile)!=178||(unsigned char)fgetc(tracefile)!=161)	
//		fatal("trace:: traverse: the trace file is not tcpdump format\n");
	//for (A1,B2,C3,D4)
	if((unsigned char)fgetc(tracefile)!=161||(unsigned char)fgetc(tracefile)!=178
		||(unsigned char)fgetc(tracefile)!=195||(unsigned char)fgetc(tracefile)!=212)	
		fatal("trace:: traverse: the trace file is not tcpdump format\n");
	//检查文件头的主次版本号
	unsigned short major_v,minor_v;
	fread((char*)(&major_v),2,1,tracefile);	
	major_v=SWAPSHORT(major_v);    //only for (A1,B2,C3,D4)
	fread((char*)(&minor_v),2,1,tracefile);	
	minor_v=SWAPSHORT(minor_v);   //only for (A1,B2,C3,D4)
	fprintf(stream,"\nthe version %hu.%hu\n",major_v,minor_v);
	if(major_v!=2||minor_v!=4)
		fatal("\ntrace:: traverse: the version is not 0204\n");
	//检查数据链路类型是否是以太网
	fseek(tracefile,12,SEEK_CUR);
	unsigned int ilink_type;
	fread((char*)(&ilink_type),4,1,tracefile);	
	ilink_type=SWAPLONG(ilink_type);     //only for (A1,B2,C3,D4)
	if(ilink_type!=1)
		fatal("\ntrace:: traverse: the link in tracefile is not Ethernet\n");
	//依次遍历各个数据包
	unsigned int eth_len, ip_len, pay_len, proc_pkt_num, total_pkt_num, c;
	unsigned char ip_head_len,ip_ver,trans_prot,tcp_head_len;
	unsigned short net_prot;
	unsigned int inputs=0;
	proc_pkt_num=0;	  total_pkt_num=0;
	
	nfa_set *nfa_state=new nfa_set();
	nfa_set *next_state=new nfa_set();
	linked_set *accepted_rules=new linked_set();
	nfa_list *nfa_queue=new nfa_list();

	nfa_set *closure = nfa->epsilon_closure();
	nfa_state->insert(closure->begin(),closure->end());

	while(feof(tracefile)==0)
	{
		if(fgetc(tracefile)==EOF)	{printf("end of file\n");  break;}  //?????,??
		total_pkt_num++;
		fseek(tracefile,7,SEEK_CUR);	//跳过8个字节(上一句已经读过一个字节)，到当前包头的报文长度字段
		fread((char*)(&eth_len),4,1,tracefile);	  
		eth_len=SWAPLONG(eth_len);   //only for (A1,B2,C3,D4)
		//printf("debug: packet%u eth_len %u\n",total_pkt_num,eth_len);
		fseek(tracefile,16,SEEK_CUR);	//跳过16个字节，到以太网的类型字段（<1500是802.3；>1500是Ethernet2）
		fread((char*)(&net_prot),2,1,tracefile);	
		net_prot=SWAPSHORT(net_prot);     //for both
		if(net_prot==0x0800);	//printf("ip	");	//以太网E2+IP
		else  //不是以太网E2+IP就跳到下一个数据包
			{fseek(tracefile,eth_len-14,SEEK_CUR);	/*printf("not Ethernet2\n");*/ continue;}
		fread((char*)(&ip_head_len),1,1,tracefile);
		ip_ver=(ip_head_len&0xf0)>>4;	ip_head_len=ip_head_len&0x0f;
		if(ip_ver!=4)
		{	
			//printf("not ipv4\n");	//不是ipv4,跳到下一个包
			fseek(tracefile,eth_len-15,SEEK_CUR);
			continue;
		}
		fseek(tracefile,8,SEEK_CUR);	//跳过8个字节，到传输层协议号
		fread((char*)(&trans_prot),1,1,tracefile);
		if(trans_prot==6)  //TCP
		{
			//printf("tcp\n");
			fseek(tracefile,ip_head_len*4-10+12,SEEK_CUR);	//跳过剩余的IP头和部分TCP头，到TCP首部长度字段
			fread((char*)(&tcp_head_len),1,1,tracefile);
			tcp_head_len = (tcp_head_len&0xf0)>>4;   //注意tcp_head_len和ip_head_len都是以4字节为单位
			fseek(tracefile,tcp_head_len*4-13,SEEK_CUR);  //跳过TCP头,到负载
			pay_len = eth_len-14-ip_head_len*4-tcp_head_len*4 ;	//应用层负载长度
		}
		else if(trans_prot==17)  //UDP
		{	//printf("udp\n");	
			fseek(tracefile,ip_head_len*4-10+8,SEEK_CUR);  //跳过UDP头
			pay_len = eth_len-14-ip_head_len*4-8;
		}
		else	//不是TCP或UDP，跳到下一个包
		{fseek(tracefile,eth_len-14-10,SEEK_CUR);  /*printf("not tcp or udp\n");*/  continue;}
		//printf("Debug pay_len: %u\n",pay_len);
		//下面是对报文负载进行正则表达式匹配
		proc_pkt_num++;
		delete nfa_state;	nfa_state=new nfa_set();
		nfa_state->insert(closure->begin(),closure->end());
		FOREACH_SET (nfa_state,it) stats[(*it)->get_id()]= stats[(*it)->get_id()] +1;
		active[nfa_state->size()]= active[nfa_state->size()] +1;
		delete next_state;	next_state=new nfa_set();

		for(int j=0;j<pay_len;j++)
		{
			c=fgetc(tracefile);
			FOREACH_SET(nfa_state,it){
				nfa_set *target=(*it)->get_transitions(c);
				if (target!=NULL){
					FOREACH_SET(target,it2){
						nfa_set *target_closure=(*it2)->epsilon_closure();	
						next_state->insert(target_closure->begin(),target_closure->end());
						delete target_closure;
					}
		            delete target;        	   
				}
			}
			delete nfa_state;
			nfa_state=next_state;
			next_state=new nfa_set();
			active[nfa_state->size()] = active[nfa_state->size()] +1;
			FOREACH_SET (nfa_state,it) 
				stats[(*it)->get_id()] = stats[(*it)->get_id()] +1;

			linked_set *rules=new linked_set();
			FOREACH_SET (nfa_state,it) rules->add((*it)->get_accepting());	
			if (!rules->empty()){
				accepted_rules->add(rules);
				if (DEBUG){
					char *label=(char *)malloc(100);
					sprintf(label,"%d",rules->value());
					linked_set *acc=rules->succ();
					while(acc!=NULL && !acc->empty()){
						char *tmp=(char *)malloc(5);
						sprintf(tmp,",%d",acc->value());
						label=strcat(label,tmp); free(tmp);
						acc=acc->succ();
					}
					fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
					free(label);
				}
			}
			delete rules;

			inputs++;
		}
	}

	//print statistics
	fprintf(stream,"total packet num:%u    processed packet num:%u   processed byte num:%u\n",total_pkt_num,proc_pkt_num,inputs);
	//首先计算nfa_depth
	int tmp_id,tmp_depth;
	nfa->traverse(nfa_queue);
	nfa->set_depth();
	FOREACH_LIST(nfa_queue,it){
		nfa_depth[(*it)->get_id()] = (*it)->get_depth();
	}
	
	//fprintf(stream,"\ntraversal statistics:: [state #,#traversal, %%time]\n");
	unsigned long num=0;
	for (int j=0;j<nfa->size();j++)
		if(stats[j]!=0){
			//fprintf(stream,"[%ld,%ld,%f %%]\n",j,stats[j],(float)stats[j]*100/inputs);
			num++;
		}
	fprintf(stream,"%ld out of %ld states traversed (%f %%)\n",num,nfa->size(),(float)num*100/nfa->size());
	fprintf(stream,"\ntraversal statistics:: [size of active state vector #,frequency, %%time]\n");
	num=0;
	for (int j=0;j<nfa->size();j++)
		if(active[j]!=0){
			fprintf(stream,"[%ld,%ld,%f %%]\n",j,active[j],(float)active[j]*100/inputs);
			num+=j*active[j];
		}
	fprintf(stream,"average size of active state vector %f\n",(float)num/inputs);

	//按NFA访问概率排序
	for(int j=0;j<nfa->size();j++)
		copy_nfa_stats[j]=stats[j];
	unsigned int tmp_access,tmp_no,tmp_order_no,tmp_accumulated_num;
	state_t tmp_state_no;
	tmp_access=0;	tmp_no=0;	tmp_order_no=0;	tmp_state_no=NO_STATE;	tmp_accumulated_num=0;
	for(int i=0;i<nfa->size();i++){
		tmp_access=0;
		tmp_no=0;
		for(int j=0;j<nfa->size();j++){
			if(copy_nfa_stats[j]>tmp_access){
				tmp_access=copy_nfa_stats[j];
				tmp_no=j;
			}
		}
		if(tmp_access==0)	break;
		copy_nfa_stats[tmp_no]=0;
		nfa_access_order[tmp_order_no]=tmp_no;
		tmp_order_no++;
	}
	fprintf(stream,"\nnfa access order statistics:[#NO,#state_id,#depth,%%access_prob,%%accumulated_prob]\n");
	for(int i=0;i<nfa->size();i++){
		tmp_state_no=nfa_access_order[i];
		if(tmp_state_no==NO_STATE){
			fprintf(stream,"%ld out of %ld NFA states traversed(%f %%)\n",i,nfa->size(),(float)i*100/nfa->size());
			break;
		}
		tmp_accumulated_num = tmp_accumulated_num + stats[tmp_state_no];
		fprintf(stream,"[%ld,%ld,%ld,%f %%, %f %%]\n",i,tmp_state_no,nfa_depth[tmp_state_no],(float)stats[tmp_state_no]*100/num,(float)tmp_accumulated_num*100/num);
	}

	fprintf(stream,"rules matched: %ld\n",accepted_rules->size());	
	free(stats);		
	free(active);
	free(copy_nfa_stats);
	free(nfa_access_order);
	free(nfa_depth);
	delete nfa_state;
	delete next_state;
	delete nfa_queue;
	delete accepted_rules;
	delete closure;
}


void trace::traverse(HybridFA *hfa, FILE *stream){
	if (tracefile==NULL) fatal("trace file is NULL!");
	rewind(tracefile);
	
	if (VERBOSE) fprintf(stderr, "\n=>trace::traverse Hybrid-FA on file %s\n...",tracename);
	
	NFA *nfa=hfa->get_nfa();
	DFA *dfa=hfa->get_head();
	map <state_t,nfa_set*> *border=hfa->get_border();
	
	//statistics
	unsigned long border_stats=0;  //border states
	
	//here we log how often a head/DFA state is active
	unsigned int *dfa_stats=allocate_uint_array(dfa->size());
	for (int j=0;j<dfa->size();j++) dfa_stats[j]=0;
	unsigned int *copy_dfa_stats=allocate_uint_array(dfa->size());
	for (int j=0;j<dfa->size();j++) copy_dfa_stats[j]=0;
	state_t *dfa_access_order=allocate_uint_array(dfa->size());
	for (int j=0;j<dfa->size();j++) dfa_access_order[j]=NO_STATE;
	
	//here we log how often a tail/NFA state is active and the size of the active set. Only tail states are considered 
	unsigned int *nfa_active=allocate_uint_array(nfa->size()); //active[x]=y if x states are active for y times
	unsigned int *nfa_stats=allocate_uint_array(nfa->size()); //stats[x]=y if state x was active y times
	unsigned int *copy_nfa_stats=allocate_uint_array(nfa->size());
	state_t *nfa_access_order=allocate_uint_array(nfa->size());
	unsigned int *nfa_depth=allocate_uint_array(nfa->size());
	for (int j=0;j<nfa->size();j++){
		nfa_stats[j]=0;
		nfa_active[j]=0;
		copy_nfa_stats[j]=0;
		nfa_access_order[j]=NO_STATE;
		nfa_depth[j]=0;
	}
	
	//code
	//读取tcpdump格式的文件，并对文件进行遍历
	//首先校验文件头，判断是否是tcpdump格式的文件(A1,B2,C3,D4)
	//.dmp文件开头是(D4 C3 B2 A1),  pcap文件开头是(D4,C3,B2,A1)
//	printf("\nfile type %d %d %d %d\n",(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile),(unsigned char)fgetc(tracefile));
	//for(D4,C3,B2,A1)
//	if((unsigned char)fgetc(tracefile)!=212||(unsigned char)fgetc(tracefile)!=195
//		||(unsigned char)fgetc(tracefile)!=178||(unsigned char)fgetc(tracefile)!=161)	
//		fatal("trace:: traverse: the trace file is not tcpdump format\n");
	//for (A1,B2,C3,D4)
	if((unsigned char)fgetc(tracefile)!=161||(unsigned char)fgetc(tracefile)!=178
		||(unsigned char)fgetc(tracefile)!=195||(unsigned char)fgetc(tracefile)!=212)	
		fatal("trace:: traverse: the trace file is not tcpdump format\n");
	//检查文件头的主次版本号
	unsigned short major_v,minor_v;
	fread((char*)(&major_v),2,1,tracefile);	
	major_v=SWAPSHORT(major_v);    //only for (A1,B2,C3,D4)
	fread((char*)(&minor_v),2,1,tracefile);	
	minor_v=SWAPSHORT(minor_v);   //only for (A1,B2,C3,D4)
	fprintf(stream,"\nthe version %hu.%hu\n",major_v,minor_v);
	if(major_v!=2||minor_v!=4)
		fatal("\ntrace:: traverse: the version is not 0204\n");
	//检查数据链路类型是否是以太网
	fseek(tracefile,12,SEEK_CUR);
	unsigned int ilink_type;
	fread((char*)(&ilink_type),4,1,tracefile);	
	ilink_type=SWAPLONG(ilink_type);     //only for (A1,B2,C3,D4)
	if(ilink_type!=1)
		fatal("\ntrace:: traverse: the link in tracefile is not Ethernet\n");
	//依次遍历各个数据包
	unsigned int eth_len, ip_len, pay_len, proc_pkt_num, total_pkt_num, c;
	unsigned char ip_head_len,ip_ver,trans_prot,tcp_head_len;
	unsigned short net_prot;
	unsigned int inputs=0;
	proc_pkt_num=0;	  total_pkt_num=0;
	//unsigned int acce_tail_num=0;	//处理报文中tail_NFA部分时访问(二级)存储器的次数
	unsigned int acce_dfa_num=0;	//处理报文中head_dfa部分访存的次数
	//unsigned int acce_mem_num=0;	//访问二级存储器的总次数
	unsigned int acce_ram_num=0;	//访问一级存储器的总次数
	unsigned int acce_ddr_dfa_num=0;
	unsigned int acce_qdr_nfa_num=0;
	unsigned int acce_qdr_nfa_min=0;
	unsigned int acce_qdr_border_num=0;
	unsigned int acce_nfa_table_num=0;
	float throughput=0;

	state_t dfa_state=0;	
	nfa_set *nfa_state=new nfa_set();
	nfa_set *next_state=new nfa_set();
	linked_set *accepted_rules=new linked_set();
	nfa_list *nfa_queue=new nfa_list();
	bool b_activate_NFA;
	unsigned activate_NFA_pkt_num=0;
	unsigned activate_NFA_byt_num=0;
	unsigned int active_NFA_pos_in_pkt[1500];
	unsigned se_proc_bytes=0;
	unsigned he_proc_bytes=0;
	unsigned sum_activeNFA_paylen=0;
	for(int i=0; i<1500; i++)
		active_NFA_pos_in_pkt[i]=0;
	while(feof(tracefile)==0)
	{
		if(fgetc(tracefile)==EOF)	{printf("end of file\n");  break;}  //到达文件尾，退出
		total_pkt_num++;
		fseek(tracefile,7,SEEK_CUR);	//跳过8个字节(上一句已经读过一个字节)，到当前包头的报文长度字段
		fread((char*)(&eth_len),4,1,tracefile);	  
		eth_len=SWAPLONG(eth_len);   //only for (A1,B2,C3,D4)
		//printf("debug: packet%u eth_len %u	",total_pkt_num,eth_len);
		fseek(tracefile,16,SEEK_CUR);	//跳过16个字节，到以太网的类型字段（<1500是802.3；>1500是Ethernet2）
		fread((char*)(&net_prot),2,1,tracefile);	
		net_prot=SWAPSHORT(net_prot);     //for both
		if(net_prot==0x0800);	//printf("ip	");	//以太网E2+IP
		else  //不是以太网E2+IP就跳到下一个数据包
			{fseek(tracefile,eth_len-14,SEEK_CUR);	/*printf("not Ethernet2\n");*/ continue;}
		fread((char*)(&ip_head_len),1,1,tracefile);
		ip_ver=(ip_head_len&0xf0)>>4;	ip_head_len=ip_head_len&0x0f;
		if(ip_ver!=4)
		{	
			//printf("not ipv4\n");	//不是ipv4,跳到下一个包
			fseek(tracefile,eth_len-15,SEEK_CUR);
			continue;
		}
		fseek(tracefile,8,SEEK_CUR);	//跳过8个字节，到传输层协议号
		fread((char*)(&trans_prot),1,1,tracefile);
		if(trans_prot==6)  //TCP
		{
			//printf("tcp		");
			fseek(tracefile,ip_head_len*4-10+12,SEEK_CUR);	//跳过剩余的IP头和部分TCP头，到TCP首部长度字段
			fread((char*)(&tcp_head_len),1,1,tracefile);
			tcp_head_len = (tcp_head_len&0xf0)>>4;   //注意tcp_head_len和ip_head_len都是以4字节为单位
			fseek(tracefile,tcp_head_len*4-13,SEEK_CUR);  //跳过TCP头,到负载
			pay_len = eth_len-14-ip_head_len*4-tcp_head_len*4 ;	//应用层负载长度
		}
		else if(trans_prot==17)  //UDP
		{	//printf("udp		");	
			fseek(tracefile,ip_head_len*4-10+8,SEEK_CUR);  //跳过UDP头
			pay_len = eth_len-14-ip_head_len*4-8;
		}
		else	//不是TCP或UDP，跳到下一个包
		{fseek(tracefile,eth_len-14-10,SEEK_CUR);  /*printf("not tcp or udp\n");*/  continue;}
		//printf("Debug pay_len: %u\n",pay_len);
		//下面是对报文负载进行正则表达式匹配
		dfa_state=0;
		dfa_stats[0]++;
		proc_pkt_num++;
		delete nfa_state;	nfa_state=new nfa_set();
		delete next_state;	next_state=new nfa_set();
		b_activate_NFA = false;
		for(int j=0;j<pay_len;j++)
		{
			c=fgetc(tracefile);
			/* head-DFA */
			//compute next state and update statistics
			dfa_state=dfa->get_next_state(dfa_state,(unsigned char)c);
			dfa_stats[dfa_state]++;
			//compute accepted rules
			if (!dfa->accepts(dfa_state)->empty()){
				accepted_rules->add(dfa->accepts(dfa_state));
				/*if (DEBUG){
					char *label=NULL;  
					linked_set *acc=dfa->accepts(dfa_state);
					while(acc!=NULL && !acc->empty()){
						if (label==NULL){
							label=(char *)malloc(100);
							sprintf(label,"%d",acc->value());
						}else{
							char *tmp=(char *)malloc(5);
							sprintf(tmp,",%d",acc->value());
							label=strcat(label,tmp); 
							free(tmp);
						}
						acc=acc->succ();
					}
					fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
					free(label);
				}*/
			}
			
			/* tail-NFA */
			//compute next state (the epsilon transitions had already been removed)
			FOREACH_SET(nfa_state,it){
				acce_qdr_nfa_num++; //add by xiaobai20150627, as for any NFA state and any symbol, there exists at most two target NFA states, 
					//which can be get from qdr  by one time through burst accessing. but whether there exists target, the access must be performed
				acce_nfa_table_num++;
				nfa_set *target=(*it)->get_transitions(c);
				if (target!=NULL){
					acce_qdr_nfa_min++;
					next_state->insert(target->begin(),target->end());
					acce_nfa_table_num = acce_nfa_table_num + 2*target->size(); //链表每获取一个元素需要两次访存
			    	delete target;        	   
				}
			}
			delete nfa_state;
			nfa_state=next_state;
			next_state=new nfa_set();
			//insert border state if needed
			border_it map_it=border->find(dfa_state);   //此处查找map可设置为多次访存，应该在log量级
														//(也可以不在此处增加访存次数，以DFA状态的一个bit表示是否是边界状态)
			if (map_it!=border->end()){
				//printf("%ld:: BORDER %ld !\n",i,dfa_state);
				nfa_state->insert(map_it->second->begin(), map_it->second->end());
				acce_nfa_table_num = acce_nfa_table_num + 2*map_it->second->size();
				acce_qdr_border_num = acce_qdr_border_num + ceil((float)(map_it->second->size())/2);
				border_stats++;
			}
			//update stats
			nfa_active[nfa_state->size()]++;
			FOREACH_SET (nfa_state,it) nfa_stats[(*it)->get_id()]++;
			//accepted rules computation
			linked_set *rules=new linked_set();
			FOREACH_SET (nfa_state,it) rules->add((*it)->get_accepting());		
			if (!rules->empty()){
				accepted_rules->add(rules);
				/*if (DEBUG){
					char *label=(char *)malloc(100);
					sprintf(label,"%d",rules->value());
					linked_set *acc=rules->succ();
					while(acc!=NULL && !acc->empty()){
						char *tmp=(char *)malloc(5);
						sprintf(tmp,",%d",acc->value());
						label=strcat(label,tmp); free(tmp);
						acc=acc->succ();
					}
					fprintf(stream,"\nrules: %s reached at character %ld \n",label,inputs);
					free(label);
				}*/
			}
			delete rules;
			inputs++;
			if(nfa_state->size()>0){
				if(b_activate_NFA == false){
					b_activate_NFA = true;
					active_NFA_pos_in_pkt[j] = active_NFA_pos_in_pkt[j] + 1;
					he_proc_bytes = he_proc_bytes + j;
					se_proc_bytes = se_proc_bytes + (pay_len-j);
				}
				activate_NFA_byt_num++;
			}	
		}//for
		if(false == b_activate_NFA){
			he_proc_bytes = he_proc_bytes + pay_len;
		}
		//对部分数据进行校正(并没有最后一个字符对应的DFA、NFA状态进行访存)
		//注意：：：NFA的访存次数不等于NFA所有状态被激活次数之和
		dfa_stats[dfa_state]--;
		//nfa_active[nfa_state->size()]--;   //这个数据与我们需要的无关，不管
		//FOREACH_SET (nfa_state,it) nfa_stats[(*it)->get_id()]--;
		if(b_activate_NFA==true){
			activate_NFA_pkt_num++;
			sum_activeNFA_paylen = sum_activeNFA_paylen + pay_len;
		}
	}	
	
	//compute number of states in the NFA part
	unsigned tail_size=hfa->get_tail_size();	//实际上是指所有能从边界状态到达的NFA状态数，包括边界状态对应的NFA状态	
	//print statistics
	printf("\n\ntotal packet num:%u    processed packet num:%u   processed byte num:%u\n",total_pkt_num,proc_pkt_num,inputs);
	printf("in general, %u out of %u packets(%f %%) have activated the tail NFA states\n",activate_NFA_pkt_num,proc_pkt_num,(float)activate_NFA_pkt_num*100/proc_pkt_num);
	printf("in general, %u out of %u bytes(%f %%) have encounted with tail NFA states\n",activate_NFA_byt_num,inputs,(float)activate_NFA_byt_num*100/inputs);
	printf("in general, nfa state transition table was accessed %u times during the matching process\n",acce_nfa_table_num);
	unsigned long sum_active_depth =0;
	unsigned long avg_active_depth =0;
	for(int i=0; i<1500; i++)
		sum_active_depth = sum_active_depth + i*active_NFA_pos_in_pkt[i];
	printf("in general, average payload length %d\n",inputs/proc_pkt_num);
	if(0!=activate_NFA_pkt_num){
		avg_active_depth = sum_active_depth/activate_NFA_pkt_num;
		printf("average payload length of packets that actived the NFA part %d, average depth of active position %d\n",sum_activeNFA_paylen/activate_NFA_pkt_num,avg_active_depth);
	}
	printf("in general, hardware engines processed %d payload bytes (%f %%); software engines processed %d payload bytes (%f %%).\n",he_proc_bytes,(float)he_proc_bytes*100/inputs,se_proc_bytes,(float)se_proc_bytes*100/inputs);
	//DFA
	dfa->set_depth();
	unsigned int *dfa_depth=dfa->get_depth();
	fprintf(stream,"\nhead-DFA=\n");
	fprintf(stream,"traversal statistics:: [state #, depth, # traversals, %%time, %%accumulated time]\n");
	int num=0;
	acce_dfa_num=0;
	for (int j=0;j<dfa->size();j++){
		if(dfa_stats[j]!=0){
			//fprintf(stream,"[%ld, %ld, %ld, %f %%, %f %%]\n",j,dfa_depth[j],dfa_stats[j],(float)dfa_stats[j]*100/inputs,(float)acce_dfa_num*100/inputs);
			num++;
			acce_dfa_num = acce_dfa_num + dfa_stats[j];   //访问dfa_次数
		}
	}
	fprintf(stream,"%ld out of %ld head-states traversed (%f %%)\n",num,dfa->size(),(float)num*100/dfa->size());
	fprintf(stream,"total dfa state access num(just for check): %ld\n",acce_dfa_num);

	if(0==inputs){
		printf("DEBUG: error, the input byte num is 0\n");
	}

	//按DFA访问概率进行排序
	for(int j=0;j<dfa->size();j++)
		copy_dfa_stats[j]=dfa_stats[j];
	unsigned int tmp_access,tmp_no,tmp_order_no;
	tmp_order_no=0;
	
	for(int i=0;i<dfa->size();i++){
		tmp_access=0;
		tmp_no=0;
		for(int j=0;j<dfa->size();j++){
			if(copy_dfa_stats[j]>tmp_access){
				tmp_access=copy_dfa_stats[j];				
				tmp_no=j;
			}
		}
		if(tmp_access==0)
			break;
		copy_dfa_stats[tmp_no]=0;
		dfa_access_order[tmp_order_no]=tmp_no;
		tmp_order_no++;
	}
	fprintf(stream,"\ndfa access order statistics:[#NO,#state_id,#depth,%%access_prob,%%accumulated_prob]\n");
	state_t tmp_state_no;
	unsigned int tmp_accumulated_num=0;	
	FILE* accu_prob_file = NULL;
	accu_prob_file = fopen("accu_prob_file","w");
	if(accu_prob_file==NULL){
		printf("\nopen file accu_prob_file error!\n");
	}
	for(int i=0;i<dfa->size();i++){
		tmp_state_no=dfa_access_order[i];
		if(tmp_state_no==NO_STATE){
			fprintf(stream,"%ld out of %ld head-states traversed (%f %%)\n",i,dfa->size(),(float)i*100/dfa->size());
			break;
		}
		tmp_accumulated_num = tmp_accumulated_num + dfa_stats[tmp_state_no];
		if(i<1024){
			fprintf(stream,"[%ld,%ld,%ld,%f %%, %f %%]\n",i,tmp_state_no,dfa_depth[tmp_state_no],(float)dfa_stats[tmp_state_no]*100/inputs,(float)tmp_accumulated_num*100/inputs);
			fprintf(accu_prob_file,"%lf\n",(double)tmp_accumulated_num*100/inputs);
		}	
	}
	fclose(accu_prob_file);

	
	//NFA
	//首先计算nfa_depth
	if(0!=tail_size)
	{
		int tmp_id,tmp_depth;
		nfa->traverse(nfa_queue);
		FOREACH_LIST(nfa_queue,it){
			//tmp_id = (*it)->get_id();
			//tmp_depth = (*it)->get_depth();
			nfa_depth[(*it)->get_id()] = (*it)->get_depth();
			//fprintf(stream,"DEBUG: nfa_id:%ld  depth:%d\n",tmp_id,tmp_depth);
		}
		fprintf(stream,"\ntail-NFA=\n");
		fprintf(stream,"traversal statistics:: [#state, #traversal, #depth, %%time=traversal/inputs]\n");
		num=0;
		for (int j=0;j<nfa->size();j++)
			if(nfa_stats[j]!=0){
				//fprintf(stream,"[%ld,%ld,%d,%f %%]\n",j,nfa_stats[j],nfa_depth[j],(float)nfa_stats[j]*100/inputs);
				num++;
			}
		fprintf(stream,"%ld out of %ld tail-states traversed (%f %%)\n",num,tail_size,(float)num*100/tail_size);
		fprintf(stream,"\ntraversal statistics:: [size of active state vector #,frequency, %%time=traversal/inputs]\n");
		num=0;
		for (int j=0;j<nfa->size();j++)
			if(nfa_active[j]!=0){
				fprintf(stream,"[%ld,%ld,%f %%]\n",j,nfa_active[j],(float)nfa_active[j]*100/inputs);
				num+=j*nfa_active[j];
			}
		if(0!=num)	
			fprintf(stream,"\naverage size of active NFA state size(total active nfa num/activate_NFA_byt_num++): %f\n",(float)num/activate_NFA_byt_num++);
		//按NFA访问概率进行排序
		for(int j=0;j<nfa->size();j++)
			copy_nfa_stats[j]=nfa_stats[j];
		tmp_access=0;	tmp_no=0;	tmp_order_no=0;	tmp_state_no=NO_STATE;	tmp_accumulated_num=0;
		for(int i=0;i<nfa->size();i++){
			tmp_access=0;
			tmp_no=0;
			for(int j=0;j<nfa->size();j++){
				if(copy_nfa_stats[j]>tmp_access){
					tmp_access=copy_nfa_stats[j];
					tmp_no=j;
				}
			}
			if(tmp_access==0)	break;
			copy_nfa_stats[tmp_no]=0;
			nfa_access_order[tmp_order_no]=tmp_no;
			tmp_order_no++;
		}
		fprintf(stream,"\nnfa access order statistics:[#NO,#state_id,#depth,%%access_prob,%%accumulated_prob]\n");
		if(0!=num){
			for(int i=0;i<nfa->size();i++){
				tmp_state_no=nfa_access_order[i];
				if(tmp_state_no==NO_STATE){
					fprintf(stream,"%ld out of %ld tail-states traversed(%f %%)\n",i,tail_size,(float)i*100/tail_size);
					break;
				}
				tmp_accumulated_num = tmp_accumulated_num + nfa_stats[tmp_state_no];
				fprintf(stream,"[%ld,%ld,%ld,%f %%, %f %%]\n",i,tmp_state_no,nfa_depth[tmp_state_no],(float)nfa_stats[tmp_state_no]*100/num,(float)tmp_accumulated_num*100/num);
			}
		}
	}
	
	fprintf(stream,"\nborder states: [total traversal, %%time]\n %ld %f %%\n",border_stats,(float)border_stats*100/inputs);
	
	fprintf(stream,"\nrules matched: %ld\n",accepted_rules->size());	
	
	//一级存储器和二级存储器访问次数统计
/*	FILE* infile=fopen("state_order","r");	//导入存储在cache中的dfa状态
	vector<unsigned int> cache_state_order;
	vector<double> cache_state_probility;
	int order;
	double prob;
	while(fscanf(infile,"%u%lf",&order,&prob)!=EOF)	
	{
		cache_state_order.push_back(order);
		cache_state_probility.push_back(prob);
	}
	fclose(infile);
	fprintf(stream,"access dfa %u times	;	access tail_nfa %u times\n",acce_dfa_num,acce_tail_num);
	fprintf(stream,"access_dfa/access_total: %lf%%	;	access_tail_nfa/access_total: %lf%%\n",(double)acce_dfa_num/(acce_dfa_num+acce_tail_num),(double)acce_tail_num/(acce_dfa_num+acce_tail_num));
	for(int k=1;k<10;k++)
	{	
		acce_cache_num=0;
		for(int i=0;i<dfa->size();i++)
		{
			if(dfa_stats[i]!=0)
			{
				for(int j=0;j<cache_state_order.size();j++)
				{
					if(cache_state_order[j]==i && j<100*k)
					{	//计算访问cache的次数
						acce_cache_num = acce_cache_num + dfa_stats[i];
						break;				
					}
				}		
			}
		}
		acce_mem_num = acce_dfa_num - acce_cache_num + acce_tail_num;
		fprintf(stream,"******************************cache/mem access stastic********************************\n");	
		fprintf(stream,"CACHE SIZE:%d	CACHE_ACC_TIME:%d	MEM_ACC_TIME:%d\n",100*k,CACHE_ACC_TIME,MEM_ACC_TIME);
		fprintf(stream,"cache_size	hit_rate	acce_cache_num	acce_mem_num	cost_time\n");
		fprintf(stream,"%d		%lf	%u	%u	%u\n",100*k,(float)acce_cache_num*100/(acce_cache_num	+acce_mem_num),acce_cache_num,acce_mem_num,acce_cache_num*CACHE_ACC_TIME+acce_mem_num*MEM_ACC_TIME);
	}
*/

	//一级存储器和二级存储器访问次数统计
	//处理一个字节所需要的时间由两部分组成，访问DFA的时间和访问非DFA表项的时间，两者相加
	//访问DFA时间可按访问百分比由cache部分和qdr部分组成
	//访问非DFA表项时间指访问qdr中除掉DFA的其它访问
/*	for(int i=0;i<dfa->size();i++){
		if(dfa_access_order[i]==NO_STATE)
			break;
		fprintf(stream,"DEBUG:  NO:%ld, state_id:%ld, dfa_stats:%ld\n",i,dfa_access_order[i],dfa_stats[dfa_access_order[i]]);
	}
*/	
/*	fprintf(stream,"\nPerformance Estimation\n");
	fprintf(stream,"[ram size, acce ram, acce ddr dfa, acce qdr nfa, acce qdr nfa min, acce qdr border]\n");
	for(int i=32;i<=256;i=i*2){
		acce_ram_num = 0;
		for(int j=0; j<i; j++){
			if(dfa_access_order[j]==NO_STATE){
				break;
			}
			acce_ram_num = acce_ram_num + dfa_stats[dfa_access_order[j]];
		}	
		acce_ddr_dfa_num = inputs - acce_ram_num;
		//throughput = 1/(((float)acce_ram_num/inputs)/(70*64/i)+((float)acce_ddr_dfa_num/inputs)/1.12+((float)(acce_qdr_nfa_num+acce_qdr_border_num)/inputs)/3.6);
		//fprintf(stream,"[%d, %u, %u, %u, %u, %f]\n",i,acce_ram_num,acce_ddr_dfa_num,acce_qdr_nfa_num,acce_qdr_border_num,throughput);
		fprintf(stream,"[%d, %u, %u, %u, %u, %u]\n",i,acce_ram_num,acce_ddr_dfa_num,acce_qdr_nfa_num,acce_qdr_nfa_min,acce_qdr_border_num);
	}
*/	
	free(dfa_stats);
	free(copy_dfa_stats);
	free(dfa_access_order);
	free(nfa_stats);		
	free(nfa_active);
	free(copy_nfa_stats);
	free(nfa_access_order);
	free(nfa_depth);
	delete nfa_state;
	delete next_state;
	delete nfa_queue;
	delete accepted_rules;
}

int trace::bad_traffic(DFA *dfa, state_t s){
	state_t **tx=dfa->get_state_table();
	int forward[CSIZE];
	int backward[CSIZE];
	int num_fw=0;
	int num_bw=0;
	for (int i=0;i<CSIZE;i++){
		if(dfa->get_depth()[tx[s][i]]>dfa->get_depth()[s]) forward[num_fw++]=i;
		else backward[num_bw++]=i;
	}
	//assert(num_fw+num_bw==CSIZE);
	srand(seed++);
	if (num_fw>0) return forward[randint(0,num_fw-1)];
	else return backward[randint(0,num_bw-1)];
}

int trace::avg_traffic(DFA *dfa, state_t s){
	return randint(0,CSIZE-1);
}

int trace::syn_traffic(DFA *dfa, state_t s,float p_fw){
	state_t **tx=dfa->get_state_table();
	int forward[CSIZE];
	int backward[CSIZE];
	int num_fw=0;
	int num_bw=0;
	int threshold=(int)(p_fw*10);
	for (int i=0;i<CSIZE;i++){
		if(dfa->get_depth()[tx[s][i]]>dfa->get_depth()[s]) forward[num_fw++]=i;
		else backward[num_bw++]=i;
	}
	srand(seed++);
	int selector=randint(1,10);
	if ((selector<=threshold && num_fw>0)||num_bw==0){
		num_forward_tx++;
		return forward[randint(0,num_fw-1)];
	}
	else{
		num_backward_tx++;
		return backward[randint(0,num_bw-1)];
	}
}

int trace::syn_traffic(NFA *root, nfa_set *active_set, float p_fw, bool forward){
	srand(seed++);
	int selector=randint(1,100);
	//random character
	if (selector > 100*p_fw) {
		num_random_tx++;
		return randint(0,CSIZE-1);
	} 
    //maximize moves forward
	int weight[CSIZE];
	int max_weight=0;
	int num_char=0;
	for (int c=0;c<CSIZE;c++){
		if (forward){
			NFA *next = get_next_forward(active_set,c);
			if (next!=NULL) weight[c] = next->get_depth();
			else weight[c] = 0;
		}
		else {
			nfa_set *next=get_next(active_set,c,true);
			weight[c]=next->size();
			delete next;
		}
		if (weight[c]>max_weight) max_weight=weight[c];
	}
	for (int c=0;c<CSIZE;c++) if (max_weight>0 && weight[c]==max_weight) num_char++;
	if (num_char==0){
		num_visits++;
		num_random_tx++;
		root->reset_visited(); //cleaning the visiting to allow another traversal
		return randint(0,CSIZE-1); //no forward transitions available - falling backwards
	}
	int selection=randint(1,num_char);
	int c ;
	for (c=0;c<CSIZE;c++) {
		if (weight[c]==max_weight) selection--;
		if (selection==0) break;
	}
	//character leading to a forward transition
	num_forward_tx++;
	return c;
}

nfa_set *trace::get_next(nfa_set *active_set, int c, bool forward){
	nfa_set *next_state = new nfa_set();
	FOREACH_SET(active_set,it){
		NFA *state=*it;
		nfa_set *tx = state->get_transitions(c);
		if (tx!=NULL){
			FOREACH_SET(tx,it2){
				if (!forward || (*it2)->get_depth() > state->get_depth()){
					nfa_set *closure=(*it2)->epsilon_closure();
					next_state->insert(closure->begin(),closure->end());
					delete closure;	
				}
			}
			delete tx;
		}
	}
	return next_state;
}

NFA *trace::get_next_forward(nfa_set *active_set, int c){
	NFA *result = NULL;
	FOREACH_SET(active_set,it){
		nfa_set *tx = (*it)->get_transitions(c);
		if (tx!=NULL){
			FOREACH_SET(tx,it2){
				if ((!SET_MBR(active_set,*it2) && !(*it2)->is_visited()) && (result==NULL || (*it2)->get_depth()>result->get_depth())) result=*it2;
			}
			delete tx;
		}
	}
	return result;
}

FILE *trace::generate_trace(NFA *nfa,int in_seed, float p_fw, bool forward, char *trace_name){
	
	FILE *trace=fopen(trace_name,"w");
	if (trace==NULL) fatal("trace::generate_trace(): could not open the trace file\n");
	
	//seed setting
	seed=in_seed;
	srand(seed);
	
	//some initializations...
	nfa->reset_depth();
	nfa->set_depth();
	nfa->reset_visited();
	num_forward_tx = num_random_tx = num_visits=0;
	
	nfa_set *active_set = nfa->epsilon_closure();
	unsigned inputs=0;
	unsigned int stream_size=randint(MIN_INPUTS,MAX_INPUTS);
	if (forward) FOREACH_SET(active_set,it) (*it)->visit();
	
	if (VERBOSE) printf("generating NFA trace: %s of %d bytes...\n",trace_name,stream_size);
	
	//traversal
	int c=syn_traffic(nfa,active_set,p_fw,forward);
	while(inputs<stream_size){
	
		inputs++;
		fputc(c,trace);
			
		//update active set
		nfa_set *next_state=get_next(active_set,c);
		delete active_set;
		active_set=next_state;
		if (forward) FOREACH_SET(active_set,it) (*it)->visit();
		
		//read next char
		c=syn_traffic(nfa,active_set,p_fw,forward);
		
	} //end traversal
	
	delete active_set;
	nfa->reset_visited();
	if (DEBUG) printf("trace::generate_trace(NFA): forward_tx=%d, random_tx=%d, visits=%d\n",
					  num_forward_tx,num_random_tx,num_visits);
	
	return trace;
}

FILE *trace::generate_trace(DFA *dfa,int in_seed, float p_fw, char *trace_name){
	
	FILE *trace=fopen(trace_name,"w");
	if (trace==NULL) fatal("trace::generate_trace(): could not open the trace file\n");
	
	//seed setting
	seed=in_seed;
	srand(seed);
	
	//some initializations...
	dfa->set_depth();
	unsigned inputs=0;
	unsigned int stream_size=randint(MIN_INPUTS,MAX_INPUTS);
	num_forward_tx = num_backward_tx=0;
	
	if (VERBOSE) printf("generating DFA trace: %s of %d bytes...\n",trace_name,stream_size);
	
	//traversal
	state_t state=0;
	int c=syn_traffic(dfa,state,p_fw);
	
	while(inputs<stream_size){
	
		inputs++;
		fputc(c,trace);
			
		state = dfa->get_next_state(state,c); //update active state
		c=syn_traffic(dfa,state,p_fw);    //get next character
		
	} //end traversal
	
	if (DEBUG) printf("trace::generate_trace(DFA): forward_tx=%d, backward_tx=%d\n",
						  num_forward_tx,num_backward_tx);

	return trace;
}

void trace::traverse(dfas_memory *mem, double *data){
	
	printf("trace:: traverse(dfas_memory) : using tracefile = %s\n",tracename);
	
	//tracefile
	if (tracefile!=NULL) rewind(tracefile);
	else fatal ("trace::traverse(): No tracefile\n");

    unsigned num_dfas=mem->get_num_dfas();
	DFA **dfas=mem->get_dfas();
	for (int i=0;i<num_dfas;i++) dfas[i]->set_depth();
	
	//statistics
	long cache_stats[2]; cache_stats[0]=cache_stats[1]=0; //0=hit, 1=miss
	
	linked_set *accepted_rules=new linked_set();
	
	long mem_accesses=0; //total number of memory accesses
	unsigned int inputs=0; //total number of inputs
	unsigned max_depth=0;
	unsigned *visited=new unsigned[mem->get_num_states()];
	for (unsigned i=0;i<mem->get_num_states();i++) visited[i]=0;
	unsigned int state_traversals=0; //state traversals
	int m_size; //size of current number of memory accesses

	/* traversal */
	
	state_t *state=new state_t[num_dfas]; //current state (for each active DFA)
	for (int i=0;i<num_dfas;i++) state[i]=0;
		
	int c=fgetc(tracefile);
	while(c!=EOF){
		inputs++;
		for (int idx=0;idx<num_dfas;idx++){
			DFA *dfa=dfas[idx];
			bool fp=true;
			while (fp){
				state_traversals++;
				visited[mem->get_state_index(idx,state[idx])]++;
				//access memory
				int *m = mem->get_dfa_accesses(idx,state[idx],c,&m_size);
				mem_accesses+=m_size;
				//trace_accesses(m,m_size,mem_trace);
				
				for (int i=0;i<m_size;i++) cache_stats[mem->read(m[i])]++;
				delete [] m;
				
				//next state
				state_t next_state=NO_STATE;
				FOREACH_TXLIST(dfa->get_labeled_tx()[state[idx]],it){
					if ((*it).first==c){
						next_state=(*it).second;
						break;
					}
				}
				if (next_state==NO_STATE && mem->allow_def_tx(idx,state[idx])){ 
					state[idx]=dfa->get_default_tx()[state[idx]];
				}else{
					state[idx]=dfa->get_state_table()[state[idx]][c];
					if (dfa->get_depth()[state[idx]]>max_depth) max_depth=dfa->get_depth()[state[idx]];
					fp=false;	
				}
			}
			
			//matching
			if (!dfa->accepts(state[idx])->empty()){
				accepted_rules->add(dfa->accepts(state[idx]));  
				linked_set *acc=dfa->accepts(state[idx]);
			}
		}
		c=fgetc(tracefile);
	} //end traversal
	delete [] state;
	
	// statistics computation
	unsigned num=0;
	for (int j=0;j<mem->get_num_states();j++) if (visited[j]!=0) num++;
			
	if (data!=NULL){
		data[0]=inputs; //number of inputs
		data[1]=accepted_rules->size(); //matches
		data[2]=max_depth; //max depth reached
		data[3]=(double)num*100/mem->get_num_states(); //% states traversed
		data[4]=(double)state_traversals/inputs;  //state traversal/input
		data[5]=(double)mem_accesses/inputs; //mem access/input
		data[6]=(double)100*cache_stats[0]/mem_accesses; //hit rate
		data[7]=(double)100*cache_stats[1]/mem_accesses; //miss rate
		data[8]=(double)(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY)/inputs; //clock cycle/input
	}
	
	//free memory
	delete [] visited;		
	delete accepted_rules;
}

void trace::traverse(fa_memory *mem, double *data){
		
	//tracefile
	if (tracefile!=NULL) rewind(tracefile);
	else fatal ("trace::traverse(fa_memory): No tracefile\n");

	if (mem->get_dfa()!=NULL) 
		traverse_dfa(mem, data);
	else if (mem->get_nfa()!=NULL) 
		traverse_nfa(mem, data);
	else 
		fatal("trace:: traverse - empty memory");
	
}

void trace::traverse_dfa(fa_memory *mem, double *data){
	//dfa
	DFA *dfa=mem->get_dfa();
	dfa->set_depth();
	unsigned int *dfa_depth=dfa->get_depth();
	state_t *fp=dfa->get_default_tx();
	tx_list **lab_tx=dfa->get_labeled_tx();
	state_t **tx=dfa->get_state_table();
	
	//statistics
	unsigned int *lab_stats=allocate_uint_array(dfa->size());
	unsigned int *fp_stats=allocate_uint_array(dfa->size());
	unsigned int *mem_stats=allocate_uint_array(MAX_MEM_REF);
	for (int j=0;j<dfa->size();j++){
		lab_stats[j]=0;
		fp_stats[j]=0;
	}
	for (int j=0;j<MAX_MEM_REF;j++) mem_stats[j]=0;
	lab_stats[0]++;
	long cache_stats[2]; cache_stats[0]=cache_stats[1]=0; //0=hit, 1=miss
	
	linked_set *accepted_rules=new linked_set();
	long mem_accesses=0; //total number of memory accesses
	unsigned int inputs=0; //total number of inputs
	unsigned int state_traversals=0; //state traversals
	int m_size; //size of current number of memory accesses

	//traversal
	state_t state=0; 
	
	int c = fgetc(tracefile);
	
	while(c!=EOF){
		state_traversals++;
		//access memory
		int *m = mem->get_dfa_accesses(state,c,&m_size);
		mem_accesses+=m_size;
		mem_stats[m_size]++;
		for (int i=0;i<m_size;i++) cache_stats[mem->read(m[i])]++;
		delete [] m;
		
		//next state
		state_t next_state=NO_STATE;
		FOREACH_TXLIST(lab_tx[state],it){
			if ((*it).first==c){
				next_state=(*it).second;
				break;
			}
		}
		if (next_state==NO_STATE && mem->allow_def_tx(state)){ 
			state=fp[state];
			fp_stats[state]++;
		}else{
			state=tx[state][c];
			lab_stats[state]++;
			//next character
			c=fgetc(tracefile);			
			inputs++;	
		}
		
		//matching
		if (!dfa->accepts(state)->empty()){
			accepted_rules->add(dfa->accepts(state));  
			linked_set *acc=dfa->accepts(state);
			if (DEBUG){
				while(acc!=NULL && !acc->empty()){
					printf("\nrule: %d reached at character %ld \n",acc->value(),inputs);
					acc=acc->succ();
				}
			}
		}
	} //end traversal
	
	// statistics computation
	//printf("\ntraversal statistics:: [state #,depth, #lab tr, #def tr, %%lab tr, %%def tr, %%time]\n");
	int num=0;
	int more_states=0;
	int max_depth=0;
	int num_fp=0;
	for (int j=0;j<dfa->size();j++){
		num_fp+=fp_stats[j];
		if(lab_stats[j]!=0 || fp_stats[j]!=0 ){
			if (dfa_depth[j]>max_depth) max_depth=dfa_depth[j];
			//printf("[%ld, %ld, %ld, %ld, %f %%, %f %%, %f %%]\n",j,dfa_depth[j],lab_stats[j],fp_stats[j],(float)lab_stats[j]*100/state_traversals,
			//		(float)fp_stats[j]*100/state_traversals,(float)(lab_stats[j]+fp_stats[j])*100/state_traversals);		
			num++;
		}
	}
	int *depth_stats=new int[max_depth+1];
	for (int i=0;i<=max_depth;i++) depth_stats[i]=0;

#ifdef LOG	
	//for (state_t s=0;s<dfa->size();s++) depth_stats[dfa->get_depth()[s]]+=(lab_stats[s]+fp_stats[s]);
	printf("%ld out of %ld states traversed (%f %%)\n",num,dfa->size(),(float)num*100/dfa->size());
	printf("rules matched: %ld\n",accepted_rules->size());
	printf("fraction mem accesses/character %f\n",(float)mem_accesses/inputs);
	printf("state traversal - /character %d %f\n",state_traversals,(float)state_traversals/inputs);
	printf("default transitions taken: tot=%ld per input=%f\n",num_fp, (float)num_fp/inputs);
	printf("max depth reached %ld\n",max_depth);	
    /*printf("depth statistics::\n");
    for(int i=0;i<=max_depth;i++) if(depth_stats[i]!=0) printf("[%d=%f]",i,(float)depth_stats[i]*100/state_traversals);
    printf("\n");
    */ 
	printf("cache hits=%ld /hit rate=%f %%, misses=%ld /miss rate=%f %%\n",
			cache_stats[0],(float)100*cache_stats[0]/mem_accesses,cache_stats[1],(float)100*cache_stats[1]/mem_accesses);
	printf("clock cycles = %ld, /input= %f\n",(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY),
			(float)(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY)/inputs);			
	
#endif	
	//mem->get_cache()->debug();
			
	if (data!=NULL){
		data[0]=inputs; //number of inputs
		data[1]=accepted_rules->size(); //matches
		data[2]=max_depth; //max depth reached
		data[3]=(double)num*100/dfa->size(); //% states traversed
		data[4]=(double)state_traversals/inputs;  //state traversal/input
		data[5]=(double)mem_accesses/inputs; //mem access/input
		data[6]=(double)100*cache_stats[0]/mem_accesses; //hit rate
		data[7]=(double)100*cache_stats[1]/mem_accesses; //miss rate
		data[8]=(double)(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY)/inputs; //clock cycle/input
	}
	
	//free memory
	free(fp_stats);
	free(lab_stats);
	free(mem_stats);
	delete [] depth_stats;		
	delete accepted_rules;
}


void trace::traverse_nfa(fa_memory *mem, double *data){
	//nfa
	NFA *nfa=mem->get_nfa();
	nfa->set_depth();
	
	//statistics
	int nfa_size=nfa->size();
	unsigned int *state_stats=allocate_uint_array(nfa_size);
	unsigned int *set_stats=allocate_uint_array(nfa_size);
	unsigned int *mem_stats=allocate_uint_array(MAX_MEM_REF);
	for (int j=0;j<nfa_size;j++){
		state_stats[j]=0;
		set_stats[j]=0;
	}
	for (int j=0;j<MAX_MEM_REF;j++) mem_stats[j]=0;
	
	long cache_stats[2]; cache_stats[0]=cache_stats[1]=0; //0=hit, 1=miss
	
	linked_set *accepted_rules=new linked_set();
	long mem_accesses=0; //total number of memory accesses
	unsigned int inputs=0; //total number of inputs
	unsigned int state_traversals=0; //state traversals
	int m_size =0; //size of current number of memory accesses

	//active set
	nfa_set *active_set = nfa->epsilon_closure();
 
	int c=fgetc(tracefile);
	
	while(c!=EOF){
		
		inputs++;
		//update statistics
		state_traversals+=active_set->size();
		set_stats[active_set->size()]++;
		FOREACH_SET(active_set,it) state_stats[(*it)->get_id()]++;
		 
		FOREACH_SET(active_set,it){
			
			NFA *state=*it;
			
			//memory access
			int *m = mem->get_nfa_accesses(state,c,&m_size);
			mem_accesses+=m_size;
			mem_stats[m_size]++;
			//trace_accesses(m,m_size,mem_trace);
			if (m!=NULL){
				for (int i=0;i<m_size;i++) cache_stats[mem->read(m[i])]++;
				delete [] m;
			}
			
			//matching
			if (!state->get_accepting()->empty()){
				accepted_rules->add(state->get_accepting());
				if (DEBUG){  
					linked_set *acc=state->get_accepting();
					while(acc!=NULL && !acc->empty()){
						printf("\nrule: %d reached at character %ld \n",acc->value(),inputs);
						acc=acc->succ();
					}
				}
			}
				
		}
		//update active set
		nfa_set *next_state=get_next(active_set,c);
		delete active_set;
		active_set=next_state;
		
		//read next char
		c=fgetc(tracefile);
		
	} //end traversal
	delete active_set;
	
	// statistics computation
	//printf("\ntraversal statistics:: [state #,depth, #lab tr, #def tr, %%lab tr, %%def tr, %%time]\n");
	int num=0;
	int more_states=0;
	int max_depth=0;
	
	nfa_list *queue=new nfa_list();
	nfa->traverse(queue);
	
	FOREACH_LIST(queue,it){
		NFA *state=*it;
		state_t j=state->get_id();
		if(state_stats[j]!=0){
			if (state->get_depth()>max_depth) max_depth=state->get_depth();
			//printf("[%ld, %ld, %ld, %f %%]\n",j,state->get_depth(),state_stats[j],(float)state_stats[j]*100/state_traversals);		
			num++;
		}
	}
	
	int *depth_stats=new int[max_depth+1];
	for (int i=0;i<=max_depth;i++) depth_stats[i]=0;
	FOREACH_LIST(queue,it) depth_stats[(*it)->get_depth()]+=state_stats[(*it)->get_id()];

#ifdef LOG	
	printf("p_m %f\n",p_fw);
	printf("%ld out of %ld states traversed (%f %%)\n",num,nfa_size,(float)num*100/nfa_size);
	printf("rules matched: %ld\n",accepted_rules->size());
	printf("fraction mem accesses/character %f\n",(float)mem_accesses/inputs);
	printf("state traversal - /character %d %f\n",state_traversals,(float)state_traversals/inputs);
	printf("max depth reached %ld\n",max_depth);	
	/*
    printf("depth statistics::\n");
    for(int i=0;i<=max_depth;i++) if(depth_stats[i]!=0) printf("[%d=%f%%]",i,(float)depth_stats[i]*100/state_traversals);
    printf("\n");
    */
	printf("cache hits=%ld /hit rate=%f %%, misses=%ld /miss rate=%f %%\n",
			cache_stats[0],(float)100*cache_stats[0]/mem_accesses,cache_stats[1],(float)100*cache_stats[1]/mem_accesses);
	printf("clock cycles = %ld, /input= %f\n",(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY),
			(float)(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY)/inputs);				
#endif
	
	//mem->get_cache()->debug();
		
	if (data!=NULL){
		data[0]=inputs; //number of inputs
		data[1]=accepted_rules->size(); //matches
		data[2]=max_depth; //max depth reached
		data[3]=(double)num*100/nfa_size; //% states traversed
		data[4]=(double)state_traversals/inputs;  //state traversal/input
		data[5]=(double)mem_accesses/inputs; //mem access/input
		data[6]=(double)100*cache_stats[0]/mem_accesses; //hit rate
		data[7]=(double)100*cache_stats[1]/mem_accesses; //miss rate
		data[8]=(double)(cache_stats[0]*CACHE_HIT_DELAY+cache_stats[1]*CACHE_MISS_DELAY)/inputs; //clock cycle/input
	}
	
	//free memory
	free(state_stats);
	free(set_stats);
	free(mem_stats);
	delete [] depth_stats;		
	delete accepted_rules;
	delete queue;
}
