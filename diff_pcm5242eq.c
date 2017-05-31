/*This progorm is for compare the orgial register map and the eq register map.
*the file must be ASCII text.
*/



#include<stdio.h>
#include<string.h>


#define INIT_REG "init_reg"
#define COMPAER_REG "compare_reg"


//#define DBG 1
#ifdef DBG 
#define debug_info(fmt,args...)\
	  do{\
			printf("\033[33m[%s:%d]\033[0m "#fmt"\r\n", __FUNCTION__, __LINE__, ##args);\
		}while(0)
#else
#define debug_info(fmt,args...)
#endif

/*register address and value*/
typedef struct{
	int addr;
	int value;
}addr_value;
/*one page register*/
typedef struct{
	int page;
	int num_reg;
	addr_value obj_addr_value[128];
}page_reg;
/*all regiseter or diffrent register*/
typedef struct{
	char *name;
	int num_page;
	page_reg *obj_page_reg;
}codec_reg;
typedef unsigned char cfg_u8;
typedef union {
	struct {
        cfg_u8 page_cmd;
        cfg_u8 page;
    };
    struct {
        cfg_u8 offset;
        cfg_u8 value;
    };

} result_cfg_reg;

static int streg_to_int(addr_value *obj_addr_value,char *s)
{
	int str_int[4];
	int i,ss[4] = {3,4,8,9};

	if(s[0] != '{'){
		printf("The frist char:%c,%c,%c\n",s[0],s[1],s[2]);
		debug_info("can not change the string to int");
		return -1;
	}

	for(i = 0;i < 4;i++){
		if(s[ss[i]] >= '0' &&s[ss[i]] <= '9'){
			str_int[i] = s[ss[i]] - '0';
		}
		else if(s[ss[i]] >= 'A' &&s[ss[i]] <= 'Z'){
			str_int[i] = s[ss[i]] - 'A' + 10;
		}
	}
	obj_addr_value->addr = str_int[0] * 16 + str_int[1];
	obj_addr_value->value = str_int[2] * 16 + str_int[3];
	
	debug_info("addr:0x%x",obj_addr_value->addr);
	debug_info("value:0x%x",obj_addr_value->value);

}

static int reg_to_page(page_reg *obj_page_reg,addr_value *obj_addr_value,int i)
{
	/*ignore the page*/
	if(obj_addr_value->addr == 0){
		return 1;
	}
	obj_page_reg->obj_addr_value[i - 1].addr = obj_addr_value->addr;
	obj_page_reg->obj_addr_value[i - 1].value = obj_addr_value->value;

	return 0;

}
static int file_to_array(char *file_path,page_reg *obj_pr)
{
	int num_page = 0,num_reg = 0;
	addr_value tmp_obj_av;
	FILE *id;	
	char tmp_org[512],*tmp_a;

	debug_info("%s",file_path);
	/*read file and storg in the codec_reg*/
	id = fopen(file_path,"r");
	if(id == NULL){
		printf("Open file error\n");
		return -1;
	}
	/*read in the register map to codec_reg */
	while(fgets(tmp_org,sizeof(tmp_org),id)){	
		/*get addr and value*/
		tmp_a = strstr(tmp_org,"{0");	
		if(tmp_a != NULL){
			debug_info("%s",tmp_a);
			streg_to_int(&tmp_obj_av,tmp_a);
			
			/*store the register map into page*/
				if(tmp_obj_av.addr == 0){
					obj_pr[num_page].page = tmp_obj_av.value;
					
					//printf("1==============%d============\n",num_reg);
					if(num_page > 0) obj_pr[num_page - 1].num_reg = num_reg - 1;				
					num_reg = 0;
					num_page++;
				}
			
			reg_to_page(&obj_pr[num_page - 1],&tmp_obj_av,num_reg);
			
			num_reg++;
		}
		
		/*read the regiter map over*/
		if(strstr(tmp_org,"};")){
			//printf("2==============%d============\n",num_reg);
			obj_pr[num_page - 1].num_reg = num_reg - 1;
			//printf("3==============%d:%d============\n",num_page,obj_pr[num_page - 1].num_reg);
			if(num_reg == 0){
				printf("No array in the file!\n");
				return -2;
				}
			break;
			}
	}
	

	
	fclose(id);

	return num_page;

}

static int compare_fun(codec_reg *org,codec_reg *cmp,result_cfg_reg *org_ru,result_cfg_reg *cmp_ru)
{
	int i,j,page = 0,reg = 0,k = 0,s = -1;
	int addr_o,addr_c,value_o,value_c,tmp_page_addr = 0;
	
	printf("Compare:%s<->%s.\n",org->name,cmp->name);	
	/*compare codec*/
	if(org->num_page != cmp->num_page){
			printf("file error!\n");
			printf("Please checkout the file is right!\n");
			return -1;
	}
	page = org->num_page;
	printf("Codec page number is:%d\n",page);
	/*compare page*/
	for(i = 0;i < page;i++){
		/*checkout every page regisger is right*/
		if(org->obj_page_reg[i].num_reg != cmp->obj_page_reg[i].num_reg){
			printf("Page error!\n");
			printf("Please checkout the page:%02x is right!\n",org->obj_page_reg[i].page);
		}
		reg = org->obj_page_reg[i].num_reg;
		tmp_page_addr = org->obj_page_reg[i].page;
		//printf("Codec page:0x%02x reg:number is:%d\n",org->obj_page_reg[i].page,reg);
		/*compare every reg*/
		for(j = 0;j < reg;j++){
			addr_o = org->obj_page_reg[i].obj_addr_value[j].addr;		
			addr_c = cmp->obj_page_reg[i].obj_addr_value[j].addr;
			if(addr_o != addr_c){
				printf("the reg address differents\n");		
				printf("========addr_o:0x%02x,addr_c:0x%02x=============\n",addr_o,addr_c);
			}
			value_o = org->obj_page_reg[i].obj_addr_value[j].value;	
			value_c = cmp->obj_page_reg[i].obj_addr_value[j].value;
			//printf("========reg:0x%02x,value_o:0x%02x,value_c:0x%02x=============\n",addr_o,value_o,value_c);
			if(value_o != value_c){
				if(s != i){
					org_ru[k].page_cmd = 0x00;
					org_ru[k].page = tmp_page_addr;
					cmp_ru[k].page_cmd = 0x00;
					cmp_ru[k].page = tmp_page_addr;
					s = i;
					}
				org_ru[k + 1].offset = addr_o;
				org_ru[k + 1].value = value_o;
				cmp_ru[k + 1].offset = addr_c;
				cmp_ru[k + 1].value = value_c;			
				k++;
				
//				printf("page:0x%02x,addr:0x%02x,value:%02x\n",tmp_page_addr,tmp_reg[k].offset,tmp_reg[k].value);
			}
		}


	}

	return k;

}
static int get_result_file(char *file_path,result_cfg_reg *org,result_cfg_reg *cmp,int diff_num)
{
	char begin[64];
	char *oc[32] = {"_org","_cmp"};
	int i,j;
	FILE *id_result;
	result_cfg_reg *tmp_result[2];

	tmp_result[0] = org;
	tmp_result[1] = cmp;

	printf("Result file name is :%s\n",file_path);
	id_result = fopen(file_path,"w+");
	if(id_result == NULL){
		printf("Open result file error\n");
		return -1;
	}
	/*begin*/
	memset(begin,0,64);
	strncpy(begin,file_path,strlen(file_path) - 2);
//	printf("Result file begin is :%s\n",begin);
	for(j = 0; j < 2;j++){	
		fprintf(id_result,"cfg_reg %s%s[]{\n",begin,oc[j]);
		for(i = 0;i < diff_num;i++){
			fprintf(id_result,"{0x%02x,0x%02x},\n",tmp_result[j][i].offset,tmp_result[j][i].value);
		}
		fprintf(id_result,"};\n");
	}

	fclose(id_result);
	return 0;

}
int main(int argc,char **argv)
{
	page_reg obj_org_pr[16],obj_cmp_pr[16];
	codec_reg org_obj_cr,cmp_obj_cr;

	int org_page,cmp_page;
	int i;
	
	/*printf help for user*/
	if(argc < 4){
		printf("The parameter number must be 3.\n");
		printf("Like this: \"reg_tool [original file] [compare file] [output file]\"\n");
		goto exit;
	}	
	
	printf("*********Compare %s and %s*************\n",argv[1],argv[2]);
	org_page = file_to_array(argv[1],obj_org_pr);	
	if(org_page < 0){
		printf("Open the %s fail ,pleas check\n",argv[1]);
		goto exit;
		}
	org_obj_cr.num_page = org_page;
	org_obj_cr.name = INIT_REG;
	org_obj_cr.obj_page_reg = obj_org_pr;
	
	cmp_page = file_to_array(argv[2],obj_cmp_pr);
	if(cmp_page < 0) {
		printf("Open the %s fail ,pleas check\n",argv[2]);
		goto exit;
		}
	cmp_obj_cr.num_page = cmp_page;
	cmp_obj_cr.name = COMPAER_REG;
	cmp_obj_cr.obj_page_reg = obj_cmp_pr;

#if 0
	/*for debug*/
	for(i = 0;i < org_page;i++){
		debug_info("NUM::::%d,num reg:%d,page:0x%x,reg 0 addr:%d,reg 0 value:%d",i,\
			obj_org_pr[i].num_reg,obj_org_pr[i].page,\
			obj_org_pr[i].obj_addr_value[0].addr,obj_org_pr[i].obj_addr_value[0].value);
	}
	for(i = 0;i < obj_org_pr[1].num_reg;i++){
		debug_info("page------------>{0x00,0x%02x}",obj_org_pr[1].page);
		debug_info("{0x%02x,0x%02x}",obj_org_pr[1].obj_addr_value[i].addr,obj_org_pr[1].obj_addr_value[i].value);		
	}
	for(i = 0;i < obj_org_pr[15].num_reg;i++){
		debug_info("page------------>{0x00,0x%02x}",obj_org_pr[15].page);
		debug_info("{0x%02x,0x%02x}",obj_org_pr[15].obj_addr_value[i].addr,obj_org_pr[15].obj_addr_value[i].value);		
	}
	for(i = 0;i < cmp_page;i++){
		debug_info("NUM::::%d,num reg:%d,page:0x%x,reg 0 addr:%d,reg 0 value:%d",i,\
			obj_cmp_pr[i].num_reg,obj_cmp_pr[i].page,\
			obj_cmp_pr[i].obj_addr_value[0].addr,obj_cmp_pr[i].obj_addr_value[0].value);
	}	
		for(i = 0;i < obj_cmp_pr[0].num_reg;i++){
		debug_info("page------------>{0x00,0x%02x}",obj_org_pr[15].page);
		debug_info("{0x%02x,0x%02x}",obj_cmp_pr[0].obj_addr_value[i].addr,obj_cmp_pr[0].obj_addr_value[i].value);		
	}
	for(i = 0;i < obj_cmp_pr[1].num_reg;i++){
		debug_info("page------------>{0x00,0x%02x}",obj_org_pr[1].page);
		debug_info("{0x%02x,0x%02x}",obj_cmp_pr[1].obj_addr_value[i].addr,obj_cmp_pr[1].obj_addr_value[i].value);		
	}

	debug_info("%d,%d!\n",org_page,cmp_page);
#endif
	int diff_num;
	result_cfg_reg org_result[128];
	result_cfg_reg cmp_result[128];
	/*compare the file and find the different*/
	diff_num = compare_fun(&org_obj_cr,&cmp_obj_cr,org_result,cmp_result);
	if(diff_num == 0){
		printf("Check out the file and ensure they are different!\n");
		goto exit;
		}

	/*get the different file for kernel*/
	get_result_file(argv[3],org_result,cmp_result,diff_num);
	
exit:
		return 0;
}



