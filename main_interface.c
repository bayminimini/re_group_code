//做基于FPGA的纯硬件引擎的仿真实验
#include "stdinc.h"
#include "nfa.h"
#include "dfa.h"
#include "hybrid_fa.h"
#include "parser.h"
#include "trace.h"
#include "classify.h"
#include "crosslist.h"



#ifndef CUR_VER
#define CUR_VER		"Michela  Becchi 1.4.1"
#endif

int VERBOSE;
int DEBUG;

#define MAX_CMD_LEN 1000
#define RAM_SIZE 64
#define MAX_RE_NUM 4096
#define MAX_RE_LEN 128   /////注意，实际大小
unsigned char re_array[MAX_RE_NUM][MAX_RE_LEN]={0};
int re_len[MAX_RE_NUM]={0};
int cur_re_num=0;

/*
 * Returns the current version string
 */
void version(){
    printf("version:: %s\n", CUR_VER);
}

/* usage */
static void usage() 
{
	fprintf(stderr,"\n");
	fprintf(stderr,"Three kinds of commands\n");
    fprintf(stderr, "1. Add(modify) a regular pattern\n"); 
    fprintf(stderr, "   Format:   add -id <regex_id> -re <regular_pattern> \n");

	fprintf(stderr,"\n");
	fprintf(stderr, "2. Configurations for patterns and trace file\n"); 
    fprintf(stderr, "Format:  config [options]\n"); 
    fprintf(stderr, "\nOptions:\n");
    //fprintf(stderr, "    --help,-h       print this message\n");
    //fprintf(stderr, "    --version,-r    print version number\n");				    
    fprintf(stderr, "    --verbose,-v    basic verbosity level \n");
    fprintf(stderr, "    --debug,  -d    enhanced verbosity level \n");
    fprintf(stderr, "\nOther:\n");
    fprintf(stderr, "    --m,--i  m modifier, ignore case\n");
    //fprintf(stderr, "    --export,-e <out_file>   export DFA to file\n");    
    //fprintf(stderr, "    --graph,-g <dot_file>    export DFA in DOT format into specified file\n"); 
    //fprintf(stderr, "	 --graph_nfa,-gnfa <dot_file>    export NFA in DOT format into specified file");  //add by xu
	fprintf(stderr, "    --train <trace_file>  trace file to be processed\n");

	fprintf(stderr,"\n");
	fprintf(stderr, "3. Compile the regular expressions to FAs and load FAs to memories\n");
	fprintf(stderr, "   Generally, this command should be the last command\n");
    fprintf(stderr, "   Format:   compile \n");
    fprintf(stderr, "\n");
    //exit(0);
}

/* configuration */
static struct conf {
	//char *regex_file;
	//char *in_file;
	char *out_file;
	char *dot_file;
	char *nfa_dot_file; //add by xu
	char train_file[30];  //供规则集训练的报文文件
	bool i_mod;
	bool m_mod;
	bool verbose;
	bool debug;
	//bool hfa;
} config;

/* initialize the configuration */
void init_conf(){
	//config.regex_file=NULL;
	//config.in_file=NULL;
	config.out_file=NULL;
	config.dot_file=NULL;
	config.nfa_dot_file=NULL;
	for(int i=0;i<30;i++)
		config.train_file[i]='\0';
	config.i_mod=false;
	config.m_mod=false;
	config.debug=false;
	config.verbose=false;
	//config.hfa=false;
}

/* print the configuration */
void print_conf(){
	fprintf(stderr,"\nCONFIGURATION: \n");
	//if (config.regex_file) fprintf(stderr, "- RegEx file: %s\n",config.regex_file);
	//if (config.in_file) fprintf(stderr, "- DFA import file: %s\n",config.in_file);
	if (config.out_file) fprintf(stderr, "- DFA export file: %s\n",config.out_file);
	if (config.dot_file) fprintf(stderr, "- DOT file: %s\n",config.dot_file);
	if (config.nfa_dot_file) fprintf(stderr, "- NFA DOT file: %s\n",config.nfa_dot_file);
	if (config.train_file[0]!='\0') fprintf(stderr,"- Train file: %s\n",config.train_file);
	if (config.i_mod) fprintf(stderr,"- ignore case selected\n");
	if (config.m_mod) fprintf(stderr,"- m modifier selected\n");
	if (config.verbose && !config.debug) fprintf(stderr,"- verbose mode\n");
	if (config.debug) fprintf(stderr,"- debug mode\n");
	//if (config.hfa)   fprintf(stderr,"- hfa generation invoked\n");
}

/*
// parse the main call parameters 
static int parse_arguments(int argc, char **argv)
{
	int i=1;
    if (argc < 2) {
        usage();
		return 0;
    }
    while(i<argc){
    	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
    		usage();
    		return 0;
    	}else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--version") == 0){
    		version();
    		return 0;
    	}else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
    		config.verbose=1;
    	}else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0){
    		config.debug=1;
    	}else if(strcmp(argv[i], "--hfa") == 0){
    	    		config.hfa=1;	
    	}else if(strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0){
    		i++;
    		if(i==argc){
    			fprintf(stderr,"Dot file name missing.\n");
    			return 0;
    		}
    		config.dot_file=argv[i];
    	}else if(strcmp(argv[i], "-gnfa") ==0 || strcmp(argv[i], "-graph_nfa") == 0){
		i++;
		if(i==argc){
			fprintf(stderr,"Dot file name missing.\n");
			return 0;
		}
		config.nfa_dot_file=argv[i];
	}else if(strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--import") == 0){
    		i++;
    		if(i==argc){
    			fprintf(stderr,"Import file name missing.\n");
    			return 0;
    		}
    		config.in_file=argv[i];	
    	}else if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--export") == 0){
    		i++;
    		if(i==argc){
    			fprintf(stderr,"Export file name missing.\n");
    			return 0;
    		}
    		config.out_file=argv[i];
    	}else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parse") == 0){
    		i++;
    		if(i==argc){
    			fprintf(stderr,"Regular expression file name missing.\n");
    			return 0;
    		}
    		config.regex_file=argv[i];
    	}else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0){
    		i++;
    		if(i==argc){
    			fprintf(stderr,"Trace file name missing.\n");
    			return 0;
    		}
    		config.trace_file=argv[i];		
    	}else if(strcmp(argv[i], "--m") == 0){
			config.m_mod=true;
		}else if(strcmp(argv[i], "--i") == 0){
			config.i_mod=true;	    		
    	}else{
    		fprintf(stderr,"Ignoring invalid option %s\n",argv[i]);
    	}
    	i++;
    }
	return 1;
}
*/

/* check that the given file can be read/written */
void check_file(char *filename, char *mode){
	FILE *file=fopen(filename,mode);
	if (file==NULL){
		fprintf(stderr,"Unable to open file %s in %c mode",filename,mode);
		fatal("\n");
	}else fclose(file);
}

int RE_Train(HybridFA *hfa, char *file, unsigned int *dfa_order){
	if(hfa==NULL){
		printf("RE_Train failed, hfa is NULL\n");
		return 0;
	}
	FILE *file_stream = fopen(file,"r"); 
	if(file_stream == NULL){
		printf("RE_Train failed, open train_file %s error!\n",file);
		return 0;
	}
	rewind(file_stream);
	fprintf(stdout, "\n=>Train Hybrid-FA on file %s\n...",file);
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
	if((unsigned char)fgetc(file_stream)!=161||(unsigned char)fgetc(file_stream)!=178
		||(unsigned char)fgetc(file_stream)!=195||(unsigned char)fgetc(file_stream)!=212){	
		fprintf(stderr,"\nRE_Train failed: the trace file is not tcpdump format\n");
		return 0;
	}
	//检查文件头的主次版本号
	unsigned short major_v,minor_v;
	fread((char*)(&major_v),2,1,file_stream);	
	//major_v=SWAPSHORT(major_v);    //only for (A1,B2,C3,D4)
	major_v = ((major_v&0xff)<<8)|((u_short)((major_v&0xff00))>>8);
	fread((char*)(&minor_v),2,1,file_stream);	
	//minor_v=SWAPSHORT(minor_v);   //only for (A1,B2,C3,D4)
	minor_v = ((minor_v&0xff)<<8)|((u_short)((minor_v&0xff00))>>8);
	fprintf(stdout,"\nthe version %hu.%hu\n",major_v,minor_v);
	if(major_v!=2||minor_v!=4){
		fprintf(stderr,"\nRE_Train failed: the version is not 0204\n");
		return 0;
	}
	//检查数据链路类型是否是以太网
	fseek(file_stream,12,SEEK_CUR);
	unsigned int ilink_type;
	fread((char*)(&ilink_type),4,1,file_stream);	
	//ilink_type=SWAPLONG(ilink_type);     //only for (A1,B2,C3,D4)
	ilink_type = ((ilink_type&0xff)<<24)|((ilink_type&0xff00)<<8)|((ilink_type&0xff0000)>>8)|((ilink_type>>24)&0xff);
	if(ilink_type!=1){
		fprintf(stderr,"\nRE_Train failed: the link in tracefile is not Ethernet\n");
		return 0;
	}
	//依次遍历各个数据包
	unsigned int eth_len, ip_len, pay_len, proc_pkt_num, total_pkt_num, c;
	unsigned char ip_head_len,ip_ver,trans_prot,tcp_head_len;
	unsigned short net_prot;
	unsigned int inputs=0;
	proc_pkt_num=0;	  total_pkt_num=0;
	unsigned int acce_tail_num=0;	//处理报文中tail_NFA部分时访问(二级)存储器的次数
	unsigned int acce_dfa_num=0;	//处理报文中head_dfa部分访存的次数
	unsigned int acce_mem_num=0;	//访问二级存储器的总次数
	unsigned int acce_cache_num=0;	//访问一级存储器的总次数
	unsigned int acce_qdr_dfa_num=0;
	unsigned int acce_qdr_nfa_num_low=0;
	unsigned int acce_qdr_nfa_num_high=0;
	unsigned int acce_qdr_border_num=0;
	float throughput_low=0;
	float throughput_high=0;

	state_t dfa_state=0;	
	nfa_set *nfa_state=new nfa_set();
	nfa_set *next_state=new nfa_set();
	linked_set *accepted_rules=new linked_set();
	nfa_list *nfa_queue=new nfa_list();
	bool b_activate_NFA;
	unsigned activate_NFA_pkt_num=0;
	unsigned activate_NFA_byt_num=0;

	while(feof(file_stream)==0)
	{
		if(fgetc(file_stream)==EOF)	{printf("end of file\n");  break;}  //到达文件尾，退出
		total_pkt_num++;
		fseek(file_stream,7,SEEK_CUR);	//跳过8个字节(上一句已经读过一个字节)，到当前包头的报文长度字段
		fread((char*)(&eth_len),4,1,file_stream);	  
		//eth_len=SWAPLONG(eth_len);   //only for (A1,B2,C3,D4)
		eth_len = ((eth_len&0xff)<<24)|((eth_len&0xff00)<<8)|((eth_len&0xff0000)>>8)|((eth_len>>24)&0xff);
		//printf("debug: packet%u eth_len %u\n",total_pkt_num,eth_len);
		fseek(file_stream,16,SEEK_CUR);	//跳过16个字节，到以太网的类型字段（<1500是802.3；>1500是Ethernet2）
		fread((char*)(&net_prot),2,1,file_stream);	
		//net_prot=SWAPSHORT(net_prot);     //for both
		net_prot = ((net_prot&0xff)<<8)|((u_short)((net_prot&0xff00))>>8);
		if(net_prot==0x0800);	//printf("ip	");	//以太网E2+IP
		else  //不是以太网E2+IP就跳到下一个数据包
			{fseek(file_stream,eth_len-14,SEEK_CUR);	/*printf("not Ethernet2\n");*/ continue;}
		fread((char*)(&ip_head_len),1,1,file_stream);
		ip_ver=(ip_head_len&0xf0)>>4;	ip_head_len=ip_head_len&0x0f;
		if(ip_ver!=4)
		{	
			//printf("not ipv4\n");	//不是ipv4,跳到下一个包
			fseek(file_stream,eth_len-15,SEEK_CUR);
			continue;
		}
		fseek(file_stream,8,SEEK_CUR);	//跳过8个字节，到传输层协议号
		fread((char*)(&trans_prot),1,1,file_stream);
		if(trans_prot==6)  //TCP
		{
			//printf("tcp\n");
			fseek(file_stream,ip_head_len*4-10+12,SEEK_CUR);	//跳过剩余的IP头和部分TCP头，到TCP首部长度字段
			fread((char*)(&tcp_head_len),1,1,file_stream);
			tcp_head_len = (tcp_head_len&0xf0)>>4;   //注意tcp_head_len和ip_head_len都是以4字节为单位
			fseek(file_stream,tcp_head_len*4-13,SEEK_CUR);  //跳过TCP头,到负载
			pay_len = eth_len-14-ip_head_len*4-tcp_head_len*4 ;	//应用层负载长度
		}
		else if(trans_prot==17)  //UDP
		{	//printf("udp\n");	
			fseek(file_stream,ip_head_len*4-10+8,SEEK_CUR);  //跳过UDP头
			pay_len = eth_len-14-ip_head_len*4-8;
		}
		else	//不是TCP或UDP，跳到下一个包
		{fseek(file_stream,eth_len-14-10,SEEK_CUR);  /*printf("not tcp or udp\n");*/  continue;}
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
			c=fgetc(file_stream);
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
				acce_qdr_nfa_num_high = acce_qdr_nfa_num_high + (*it)->get_transitions()->size();
				nfa_set *target=(*it)->get_transitions(c);
				//寻找下一个状态时访问二级存储器的次数
				//这个计算可能有问题，因为nfa的transition是以链表存储的，可能需要访问整个链表
				//fprintf(stream,"DEBUG: acce_qdr_nfa_num_low: %u\n",acce_qdr_nfa_num_low);
				if (target!=NULL){
					next_state->insert(target->begin(),target->end());
					acce_tail_num = acce_tail_num + target->size();
					acce_qdr_nfa_num_low = acce_qdr_nfa_num_low + target->size()+1; //+1是因为先访问该状态，再查找对应表项
			    		delete target;        	   
				}
			}
			delete nfa_state;
			nfa_state=next_state;
			next_state=new nfa_set();
			//insert border state if needed
			border_it map_it=border->find(dfa_state);
			if (map_it!=border->end()){
				//printf("%ld:: BORDER %ld !\n",i,dfa_state);
				nfa_state->insert(map_it->second->begin(), map_it->second->end());
				acce_qdr_border_num = acce_qdr_border_num + map_it->second->size();
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
				b_activate_NFA = true;
				activate_NFA_byt_num++;
			}	
		}//for
		//对部分数据进行校正(并没有最后一个字符对应的DFA、NFA状态进行访存)
		//注意：：：NFA的访存次数不等于NFA所有状态被激活次数之和
		dfa_stats[dfa_state]--;
		//nfa_active[nfa_state->size()]--;   //这个数据与我们需要的无关，不管
		//FOREACH_SET (nfa_state,it) nfa_stats[(*it)->get_id()]--;
		if(b_activate_NFA==true)
			activate_NFA_pkt_num++;
	}

	//compute number of states in the NFA part
	unsigned tail_size=hfa->get_tail_size();	//实际上是指所有能从边界状态到达的NFA状态数，包括边界状态对应的NFA状态	
	//print statistics
	printf("total packet num:%u    processed packet num:%u   processed byte num:%u\n",total_pkt_num,proc_pkt_num,inputs);
	printf("in general, %u out of %u packets(%f %%) have activated the tail NFA states\n",activate_NFA_pkt_num,proc_pkt_num,(float)activate_NFA_pkt_num*100/proc_pkt_num);
	printf("in general, %u out of %u bytes(%f %%) have encounted with tail NFA states\n",activate_NFA_byt_num,inputs,(float)activate_NFA_byt_num*100/inputs);

	//DFA
	dfa->set_depth();
	unsigned int *dfa_depth=dfa->get_depth();
	fprintf(stdout,"\nhead-DFA=\n");
	fprintf(stdout,"traversal statistics:: [state #, depth, # traversals, %%time, %%accumulated time]\n");
	int num=0;
	acce_dfa_num=0;
	for (int j=0;j<dfa->size();j++){
		if(dfa_stats[j]!=0){
			fprintf(stdout,"[%ld, %ld, %ld, %f %%, %f %%]\n",j,dfa_depth[j],dfa_stats[j],(float)dfa_stats[j]*100/inputs,(float)acce_dfa_num*100/inputs);
			num++;
			acce_dfa_num = acce_dfa_num + dfa_stats[j];   //访问dfa_次数
		}
	}
	fprintf(stdout,"%ld out of %ld head-states traversed (%f %%)\n",num,dfa->size(),(float)num*100/dfa->size());
	fprintf(stdout,"total dfa state access num(just for check): %ld\n",acce_dfa_num);

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
	fprintf(stdout,"\ndfa access order statistics:[#NO,#state_id,#depth,%%access_prob,%%accumulated_prob]\n");
	state_t tmp_state_no;
	unsigned int tmp_accumulated_num=0;
	for(int i=0;i<dfa->size();i++){
		tmp_state_no=dfa_access_order[i];
		if(tmp_state_no==NO_STATE){
			fprintf(stdout,"%ld out of %ld head-states traversed (%f %%)\n",i,dfa->size(),(float)i*100/dfa->size());
			break;
		}
		tmp_accumulated_num = tmp_accumulated_num + dfa_stats[tmp_state_no];
		fprintf(stdout,"[%ld,%ld,%ld,%f %%, %f %%]\n",i,tmp_state_no,dfa_depth[tmp_state_no],(float)dfa_stats[tmp_state_no]*100/inputs,(float)tmp_accumulated_num*100/inputs);
	}
	for(int i=0; i<RAM_SIZE; i++)
		dfa_order[i] = dfa_access_order[i];

	
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

	return 1;
}

//注意:RE_Rename对hfa中的DFA部分进行重命名。
//其中DFA涉及到的改动有state_table, accept_rules, depth. 由于没有涉及到压缩算法，所以并未对default_tx 和labeled_tx进行改动
//hfa涉及到的改动有border, specialTailDFA.
int HFA_Rename(HybridFA *hfa,unsigned int *dfa_order){
	fprintf(stdout, "\n=>RE_Rename is on process ...\n");
	DFA *dfa = hfa->get_head();
	state_t origin_state=0;
	state_t order_state;
	state_t tmp_state;
	linked_set *tmp_accept_rules;
	unsigned int tmp_depth;
	unsigned int *tmp_p_depth = dfa->get_depth();
	map <state_t, nfa_set*> *tmp_border = hfa->get_border();
	nfa_set *tmp_nfa_set;
	for(int i=0; i<RAM_SIZE; i++){
		order_state = dfa_order[i];
		if(order_state==NO_STATE)
			break;
		//DFA部分的重命名(不涉及default_tx 和label_tx)
		if(order_state==origin_state)
			continue;
		for(int j=0; j<CSIZE; j++){
			tmp_state = dfa->get_next_state(origin_state,j);
			dfa->add_transition(origin_state,j,dfa->get_next_state(order_state,j));
			dfa->add_transition(order_state,j,tmp_state);
		}
		for(int l=0; l<dfa->size(); l++)
			for(int m=0; m<CSIZE; m++){
				if(dfa->get_next_state(l,m)==origin_state)
					dfa->add_transition(l,m,order_state);
				else if(dfa->get_next_state(l,m)==order_state)
					dfa->add_transition(l,m,origin_state);
				else ;
			}

		tmp_accept_rules = dfa->accepts(origin_state);
		dfa->set_accepts(origin_state,dfa->accepts(order_state));
		dfa->set_accepts(order_state,tmp_accept_rules);

		tmp_depth = tmp_p_depth[origin_state];
		tmp_p_depth[origin_state] = tmp_p_depth[order_state];
		tmp_p_depth[order_state] = tmp_depth;
		//dead_state
		if(dfa->get_dead_state() == origin_state)
			dfa->set_dead_state(order_state);
		if(dfa->get_dead_state() == order_state)
			dfa->set_dead_state(origin_state);

		//hfa部分的重命名,主要是border(没有涉及到specialTailDFA)
		tmp_nfa_set = (*tmp_border)[origin_state];
		(*tmp_border)[origin_state] = (*tmp_border)[order_state];
		(*tmp_border)[order_state] = tmp_nfa_set;

		origin_state++;
	}

	return 1;
}

//after HFA renaming, add some marks for IDs of DFA and NFA. The first bit of DFA ID is employed to identify whether this DFA state is an accepted state,
//the second bit of DFA ID is employed to identify whether this DFA state is a border state. Similiarly, the first bit of NFA ID is employed to identify whether this
//NFA state is an accepted state.
//ATTENTION: for DFA part, we only mark dfa transition table, we donot mark the dfa id in border and accepted_rules
//			 for NFA part, we 
int mark_hfa_id(HybridFA *hfa){
	//mark DFA ids
	state_t origin_state;
	DFA* dfa = hfa->get_head();
	map <state_t,nfa_set*> *border=hfa->get_border();
	state_t* dfa_marked_id = new state_t[dfa->size()];
	for(state_t i=0; i<dfa->size(); i++){
		dfa_marked_id[i] = i;
		if(!dfa->accepts(i)->empty())
			dfa_marked_id[i] = dfa_marked_id[i] | 0x8000;
		if(border->find(i) != border->end())
			dfa_marked_id[i] = dfa_marked_id[i] | 0x4000;
	}
	for(state_t i=0; i<dfa->size(); i++){
		for(int j=0; j<CSIZE; j++){
			origin_state = dfa->get_next_state(i,j);
			if(origin_state != dfa_marked_id[origin_state])
				dfa->add_transition(i,j,dfa_marked_id[origin_state]);
		}
	}
	
	if(dfa_marked_id != NULL)
		delete dfa_marked_id;
	//mark NFA ids
	NFA* nfa=hfa->get_nfa();
	nfa_list* nfa_queue;
	nfa->traverse(nfa_queue);
	state_t* nfa_marked_id = new state_t[nfa_queue->size()];
	FOREACH_LIST(nfa_queue,it){
		nfa_marked_id[(*it)->get_id()] = (*it)->get_id();
		if(!(*it)->get_accepting()->empty())
			nfa_marked_id[(*it)->get_id()] = nfa_marked_id[(*it)->get_id()] | 0x8000;	
	}
	FOREACH_LIST(nfa_queue,it){
		//first, mark the state id(with or without this is OK)
		(*it)->set_id(nfa_marked_id[(*it)->get_id()]);
		//then, mark the transitions of this state 
		//FOREACH_PAIRSET((*it)->transitions,it2){
			
		//}
	}

	if(nfa_marked_id != NULL)
		delete nfa_marked_id;
		
}

//hfa重命名后，将前RAM_SIZE个DFA状态表写入到文件中。这里需要
int dfa_ram_mem(HybridFA *hfa, FILE* dfa_ram_file){
	
}

int RE_Load(HybridFA *hfa){
	//DFA的前RAM_SIZE个写到RAM中，NFA和border各写到一个QDR中，整个DFA写ddr中，DFA状态的接受规则和NFA状态的接收规则写入到DDR中
	
}

int RE_Add(int id, const char* reg_pattern, int len){
	if(id<0 || id>=MAX_RE_NUM){
		printf("RE_ADD failed: the id should be in [0,%d)\n",MAX_RE_NUM);
		return -1;
	}
	if(len<1 || len>MAX_RE_LEN){
		printf("RE_ADD failed: the regex pattern length should be in [1,%d]\n",MAX_RE_LEN);
		return -1;
	}
	//(如果是重写规则)，先把原规则清空
	for(int i=0; i<MAX_RE_LEN; i++)
		re_array[id][i] = '\0';
	for(int i=0; i<len; i++)	
		re_array[id][i] = reg_pattern[i];
	re_len[id] = len;
	cur_re_num++;
	printf("Add rule %d:	%s\n",id,re_array[id]);
	return 1;
}

int RE_Compile(){
	if(cur_re_num==0){
		printf("RE_Compile failed: no regular patterns exist, please input some regular patterns!\n");
		return 0;
	}

	if(config.train_file[0]!='\0')	check_file(config.train_file,"r");
	// FA declaration 
	NFA *nfa=NULL;  	// NFA
	HybridFA *hfa=NULL; // Hybrid-FA

	fprintf(stderr,"\nParsing the regular expressions ...\n");
	regex_parser *parse=new regex_parser(config.i_mod,config.m_mod);
	nfa = parse->parse((char **)re_array, MAX_RE_NUM, MAX_RE_LEN, re_len);
	nfa->remove_epsilon();
 	nfa->reduce();
	printf("SET: MAX_TX: %d, SPECIAL_MIN_DEPTH: %d,  MAX_HEAD_SIZE: %d\n",MAX_TX,SPECIAL_MIN_DEPTH,MAX_HEAD_SIZE);	
	if (nfa==NULL) fatal("Impossible to build a Hybrid-FA if no NFA is given.");
	printf("compute the hfa with tail NFAs....\n");
	hfa=new HybridFA(nfa);
	printf("before minimize HFA:: head size=%d, tail size=%d, number of tails=%d, border size=%d\n",hfa->get_head()->size(),hfa->get_tail_size(),hfa->get_num_tails(),hfa->get_border()->size());
	if (hfa->get_head()->size()<100000){ 
		hfa->minimize();
	}
	printf("after minimize HFA:: head size=%d, tail size=%d, number of tails=%d, border size=%d\n\n\n",hfa->get_head()->size(),hfa->get_tail_size(),hfa->get_num_tails(),hfa->get_border()->size());
		
	//对于每一个special NFA状态，对它能到达的状态进行遍历
	hfa->statistics_nfapart();

	unsigned int dfa_order[RAM_SIZE];
	if(config.train_file[0]!='\0')
		RE_Train(hfa,config.train_file,dfa_order);
		for(int i=0; i<RAM_SIZE; i++){
		fprintf(stdout,"NO %d: state %d\n",i,dfa_order[i]);
		HFA_Rename(hfa,dfa_order);  
		RE_Load(hfa);
	}


	//释放hfa
	if(parse!=NULL)
		delete parse;
	if(nfa!=NULL)
		delete nfa;
	if(hfa!=NULL)
		delete hfa;
	return 1;
}


static int parse_command(char *cmd)
{
	char *p1, *p2;
	p1 = cmd;
	int id;
	char *re;
	while(isspace(*p1)&&(p1-cmd<MAX_CMD_LEN))
		p1++;
	p2 = p1;
	while(!isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
		p2++;
	if((p2-p1==3)&&!strncmp(p1,"add",3)){
		while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
			p2++;
		p1=p2;
		while(!isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
			p2++;
		if((p2-p1==3)&&!strncmp(p1,"-id",3)){
			while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
				p2++;
			p1=p2;	
			while(isdigit(*p2)&&(p2-cmd<MAX_CMD_LEN))
				p2++;	
			if(isspace(*p2)){
				id = atoi(p1);
				if(id<0 || id>=MAX_RE_NUM){
					printf("RE_ADD failed: the id should be in [0,%d)\n",MAX_RE_NUM);
					return 0;
				}
				while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
					p2++;
				p1=p2;
				while(!isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
					p2++;
				if((p2-p1==3)&&!strncmp(p1,"-re",3)){
					while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
						p2++;
					p1=p2;
					while((*p2)!='\n'&&(p2-cmd<MAX_CMD_LEN))
						p2++;
					RE_Add(id,p1,p2-p1);
				}else
					return 0;	
			}
			else
				return 0;	
		}
		else
			return 0;
	}
	else if((p2-p1==6)&&!strncmp(p1,"config",6)){
		while((*p2)!='\n'&&(p2-cmd<MAX_CMD_LEN)){
			while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
				p2++;
			p1=p2;
			while(!isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
				p2++;
			if(((p2-p1==7)&&!strncmp(p1,"--debug",7))||((p2-p1==2)&&!strncmp(p1,"-d",2)))
				config.debug=1;
			else if((p2-p1==3)&&!strncmp(p1,"--m",3))
				config.m_mod=true;
			else if((p2-p1==3)&&!strncmp(p1,"--i",3))
				config.i_mod=true;
			else if(((p2-p1==7)&&!strncmp(p1,"--verbose",9))||((p2-p1==2)&&!strncmp(p1,"-v",2)))
				config.verbose=1;
			else if((p2-p1==7)&&!strncmp(p1,"--train",7)){
				while(isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
					p2++;
				p1=p2;
				while(!isspace(*p2)&&(p2-cmd<MAX_CMD_LEN))
					p2++;
				strncpy(config.train_file, p1, p2-p1);
			}
			else
				return 0;
		}
	}
	else if((p2-p1==7)&&!strncmp(p1,"compile",7)){
		print_conf();
		VERBOSE=config.verbose;
		DEBUG=config.debug; if (DEBUG) VERBOSE=1;
		RE_Compile();

		
	}
	else
		return 0;

	return 1;
}








/*
//将规则集生成的自动机写入到RAM 、DDR、QDR中(涉及数据集训练、DFA状态重命名、写入三个过程)
int RE_load(char *train_file){
	FILE *file=fopen(filename,mode);
	if (file==NULL){
		fprintf(stderr,"Unable to open file %s in %c mode",train_file,mode);
		fatal("\n");
	}else fclose(file);
}
*/
int main(int argc, char **argv){
	char cmd[MAX_CMD_LEN];
	for(int i=0; i<MAX_CMD_LEN; i++)
		cmd[i] = '\0';
	init_conf();
	usage();
	while(1){
		while(!fgets(cmd,MAX_CMD_LEN,stdin)){
			usage();
			for(int i=0; i<MAX_CMD_LEN; i++)
				cmd[i] = '\0';
		}
		if(!parse_command(cmd)){
			usage();
		}
		for(int i=0; i<MAX_CMD_LEN; i++)
			cmd[i] = '\0';
	}
}

/*
int main(int argc, char **argv){
	//read configuration
	init_conf();
	while(!parse_arguments(argc,argv)) usage();
	print_conf();
	VERBOSE=config.verbose;
	DEBUG=config.debug; if (DEBUG) VERBOSE=1; 
	
	//check that it is possible to open the files
	if (config.regex_file!=NULL) check_file(config.regex_file,"r");
	if (config.in_file!=NULL) check_file(config.in_file,"r");
	if (config.out_file!=NULL) check_file(config.out_file,"w");
	if (config.dot_file!=NULL) check_file(config.dot_file,"w");
	if (config.nfa_dot_file!=NULL) check_file(config.nfa_dot_file,"w");
	if (config.trace_file!=NULL) check_file(config.trace_file,"r");
	
	// check that either a regex file or a DFA import file are given as input
	if (config.regex_file==NULL && config.in_file==NULL){
		fatal("No data file - please use either a regex or a DFA import file\n");
	}
	if (config.regex_file!=NULL && config.in_file!=NULL){
		printf("DFA will be imported from the Regex file. Import file will be ignored");
	}
	
	// FA declaration 
	NFA *nfa=NULL;  	// NFA
	DFA *dfa=NULL;		// DFA
	dfa_set *dfas=NULL; // set of DFAs, in case a single DFA for all RegEx in the set is not possible
	HybridFA *hfa=NULL; // Hybrid-FA
	
	// if regex file is provided, parses it and instantiate the corresponding NFA.
	// if feasible, convert the NFA to DFA
	if (config.regex_file!=NULL){
		FILE *regex_file=fopen(config.regex_file,"r");
		fprintf(stderr,"\nParsing the regular expression file %s ...\n",config.regex_file);
		regex_parser *parse=new regex_parser(config.i_mod,config.m_mod);
		nfa = parse->parse(regex_file);
		nfa->remove_epsilon();
	 	nfa->reduce();
		dfa=nfa->nfa2dfa();
		if (dfa==NULL) printf("Max DFA size %ld exceeded during creation: the DFA was not generated\n",MAX_DFA_SIZE);
		else dfa->minimize();
		fclose(regex_file);
		delete parse;
	}
	
	// if a regex file is not provided, import the DFA 
	if (config.regex_file==NULL && config.in_file!=NULL){
		FILE *in_file=fopen(config.in_file,"r");
		fprintf(stderr,"\nImporting from file %s ...\n",config.in_file);
		dfa=new DFA();
		dfa->get(in_file);
		fclose(in_file);
	}
	
	// DFA export 
	if (dfa!=NULL && config.out_file!=NULL){
		FILE *out_file=fopen(config.out_file,"w");
		fprintf(stderr,"\nExporting to file %s ...\n",config.out_file);
		dfa->put(out_file);
		fclose(out_file);
	}
	
	// DOT file generation
	if (dfa!=NULL && config.dot_file!=NULL){
		FILE *dot_file=fopen(config.dot_file,"w");
		fprintf(stderr,"\nExporting to DOT file %s ...\n",config.dot_file);
		char string[100];
		if (config.regex_file!=NULL) sprintf(string,"source: %s",config.regex_file);
		else sprintf(string,"source: %s",config.in_file);
		dfa->to_dot(dot_file, string);
		fclose(dot_file);
	}
	//NFA DOT file generation
	if (nfa!=NULL && config.nfa_dot_file!=NULL){
		FILE *nfa_dot_file = fopen(config.nfa_dot_file,"w");
		fprintf(stderr,"\nExporting to NFA DOT file %s ...\n",config.nfa_dot_file);
		char string[100];
		if(config.regex_file!=NULL) sprintf(string,"source: %s",config.regex_file);
		else sprintf(string,"source: NONE");
		nfa->to_dot(nfa_dot_file,string);
		fclose(nfa_dot_file);
	}

	// HFA generation
	long time_start, time_NFA, time_rem_eps, time_redu, time_hfa, time_min;
	if (config.hfa){
		FILE *regex_file=fopen(config.regex_file,"r");
		fprintf(stderr,"\nParsing the regular expression file %s ...\n",config.regex_file);
		regex_parser *parse=new regex_parser(config.i_mod,config.m_mod);
		time_start = time(NULL);
		nfa = parse->parse(regex_file);
		time_NFA = time(NULL);
		printf("time NFA:%ld s, time_total:%ld s\n",time_NFA-time_start,time_NFA-time_start);
		nfa->remove_epsilon();
		time_rem_eps = time(NULL);
		printf("time remove epsilon:%ld s, time_total:%ld s\n",time_rem_eps-time_NFA,time_rem_eps-time_start);
	 	nfa->reduce();
		time_redu = time(NULL);
		printf("time reduce:%ld s, time_total:%ld s\n",time_redu-time_rem_eps,time_redu-time_start);
		printf("SET: MAX_TX: %d, SPECIAL_MIN_DEPTH: %d,  MAX_HEAD_SIZE: %d\n",MAX_TX,SPECIAL_MIN_DEPTH,MAX_HEAD_SIZE);	
		if (nfa==NULL) fatal("Impossible to build a Hybrid-FA if no NFA is given.");
		printf("compute the hfa with tail NFAs....\n");
		hfa=new HybridFA(nfa);
		time_hfa = time(NULL);
		printf("time nfa2hfa:%ld s, time_total:%ld s\n",time_hfa-time_redu,time_hfa-time_start);
		printf("before minimize HFA:: head size=%d, tail size=%d, number of tails=%d, border size=%d\n",hfa->get_head()->size(),hfa->get_tail_size(),hfa->get_num_tails(),hfa->get_border()->size());
		if (hfa->get_head()->size()<100000){ 
			hfa->minimize();
			time_min = time(NULL);
			printf("time min:%ld s, time_total:%ld s\n",time_min-time_hfa,time_min-time_start);
		}
		printf("after minimize HFA:: head size=%d, tail size=%d, number of tails=%d, border size=%d\n\n\n",hfa->get_head()->size(),hfa->get_tail_size(),hfa->get_num_tails(),hfa->get_border()->size());
		
		//对于每一个special NFA状态，对它能到达的状态进行遍历
		hfa->statistics_nfapart();
	
	}

	//导出(head_dfa)转移概率矩阵,同时将转移概率矩阵赋值给M 
//	CrossList M;
//	printf("compute the trans matrix.......\n");
//	hfa->get_head()->output_matrix(M);
//	//dfa->output_matrix(M);

//	//计算M的稳态矩阵中闭合状态的排序情况
//	vector<unsigned int> final_order;
//	vector<double> steady_probility;
//	final_steady_order(M,final_order,steady_probility);
	


	// trace file traversal
	if (config.trace_file){
		trace *tr=new trace(config.trace_file);
		//if (nfa!=NULL) tr->traverse(nfa);
//	    	if (dfa!=NULL){
//			tr->traverse(dfa);	
//			if (dfa->get_default_tx()!=NULL) tr->traverse_compressed(dfa);
//		}		
		if (hfa!=NULL) tr->traverse(hfa);
		delete tr;
	}
	 
	// if the DFA was not generated because of state blow-up during NFA-DFA transformation,
	// then generate multiple DFAs
//	if (config.regex_file!=NULL && dfa==NULL){
//		printf("\nCreating multiple DFAs...\n");
//		FILE *re_file=fopen(config.regex_file,"r");
//		regex_parser *parser=new regex_parser(config.i_mod,config.m_mod);
//		dfas = parser->parse_to_dfa(re_file);
//		printf("%d DFAs created\n",dfas->size());
//		fclose(re_file);
//		delete parser;
//		int idx=0;
//		FOREACH_DFASET(dfas,it) {
//			printf("DFA #%d::  size=%ld\n",idx,(*it)->size());
//			if (config.out_file!=NULL){
//				char out_file[100];
//				sprintf(out_file,"%s%d",config.out_file,idx);
//				FILE *file=fopen(out_file,"w");
//				fprintf(stderr,"Exporting DFA #%d to file %s ...\n",idx,out_file);
//				(*it)->put(file);
//				fclose(file);
//				idx++;
//			}
//		}
//	}

	
	
	
	// write your code here
//	dfa->print_table();	//打印出状态转换表
//	dfa->output_table();   //导出完整的DFA转换表
//	dfa->output_edge();	//导出DFA出边情况

	
	
	
	// Automata de-allocation 
	
	if (nfa!=NULL) delete nfa;
	if (dfa!=NULL) delete dfa;
	if (dfas!=NULL){
		FOREACH_DFASET(dfas,it) delete (*it);
		delete dfas;
	}
	if (hfa!=NULL) delete hfa;				
	
	return 0;
}
*/
