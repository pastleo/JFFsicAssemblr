#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <list>

using namespace std;

#include "hashTableModel.h"

#define TABLE_SIZE 400

// debug and test functions
void hashTableModel_test();
void pb(int); // print bool
string hashTableModel_callback(int,int);

// hash table, see hashTableModel.h for detailed description
Table table(TABLE_SIZE);

// namespace pass1 ===================================
namespace pass1 {
	/**
	 * === GLOBAL variables ===
	 * int mode:
	 * 0: waiting label,op or directive
	 * 1: require 0 operand for the op or dir just detected
	 * 2: require 1 operand for the op or dir just detected
	 * 3: require 2 operand for the op or dir just detected
	 *
	 * -1: ended,build SYMTAB
	 *
	 * int addr: relative addr to the start addr
	 * int start_addr: set by START directive
	 * int Tl: current Text record length for pass2
	 */
	int mode,addr,start_addr,Tl;

	/**
	 * string startLab: Store the label for START directive
	 * string labTmp: Store the last label
	 */
	string startLab,labTmp;

	// current line read from the src file
	char line[256]={0};

	// a string list for symtable;
	list<string> symtable;

	// Text records length for pass 2
	list<int*> T_len;

	/**
	 * Symbol Handlers
	 * handler functions (char* (*)(int)) for processing all kinds of symbols
	 * @param  int para_given_when_added
	 * @param  int para_given_when_called
	 * @return whatever string
	 */
	string labelHandler(int,int);
	string opHandler(int,int);
	string d_start(int,int);
	string d_end(int,int);
	string d_wb(int,int);
	string d_resx(int,int);

	/**
	 * import
	 * import optable and directives to the table
	 */
	void import();

	/**
	 * Op2AddrAdd
	 * Opcode to AddrAdd and Number of parameters
	 */
	struct Op2AddrAdd
	{
		string name;
		int addradd,NofPara;
	} op2addradd[59]={
		"ADD",3,1,
		"ADDF",3,1,
		"ADDR",2,1,
		"AND",3,1,
		"CLEAR",2,1,
		"COMP",3,1,
		"COMPF",3,1,
		"COMPR",2,1,
		"DIV",3,1,
		"DIVF",3,1,
		"DIVR",2,1,
		"FIX",1,1,
		"FLOAT",1,1,
		"HIO",1,1,
		"J",3,1,
		"JEQ",3,1,
		"JGT",3,1,
		"JLT",3,1,
		"JSUB",3,1,
		"LDA",3,1,
		"LDB",3,1,
		"LDCH",3,1,
		"LDF",3,1,
		"LDL",3,1,
		"LDS",3,1,
		"LDT",3,1,
		"LDX",3,1,
		"LPS",3,1,
		"MUL",3,1,
		"MULF",3,1,
		"MULR",2,1,
		"NORM",1,1,
		"OR",3,1,
		"RD",3,1,
		"RMO",2,1,
		"RSUB",3,0,
		"SHIFTL",2,1,
		"SHIFTR",2,1,
		"SIO",1,1,
		"SSK",3,1,
		"STA",3,1,
		"STB",3,1,
		"STCH",3,1,
		"STF",3,1,
		"STI",3,1,
		"STL",3,1,
		"STS",3,1,
		"STSW",3,1,
		"STT",3,1,
		"STX",3,1,
		"SUB",3,1,
		"SUBF",3,1,
		"SUBR",2,1,
		"SVC",2,1,
		"TD",3,1,
		"TIO",1,1,
		"TIX",3,1,
		"TIXR",2,1,
		"WD",3,1
	};

	/**
	 * main process of pass1
	 * @param src_file_name
	 * @param intermediate_file_name 
	 */
	void main(char* src_file_name,char* intermediate_file_name){

		// get assembler prepared...
		import();

		// prepare file IO
		fstream src_file;
		src_file.open(src_file_name, ios::in);
		FILE* im_file=fopen(intermediate_file_name,"w");
		
		/**
		 * string symTmp: Store the last symbol (word)
		 * char* opTmp: Store the last operation
		 * int ln: line number
		 * int wn: word length
		 * int hash: hashCode value
		 * int hashTmp: hashCode saved for the directive
		 * int paraTmp: store parameter char* location for op or di
		 * int addrTmp: remember the address of current line
		 */
		string symTmp;
		char* opTmp;
		int ln=1,wn,hash,hashTmp,paraTmp,addrTmp;
		addr=0;
		try{
			while(!src_file.eof()){
				src_file.getline(line, 256);
				ln++;

				wn=0;
				hash=0;
				mode=0;
				paraTmp=-2;
				labTmp="";
				addrTmp=addr;
				for(int i=0;line[i];i++){
					if(line[i]==46)
						break;
					else if(line[i]>=33 && line[i]!=44){
						if(!wn)
							wn=1;
						else
							wn++;

						if(line[i]>97)
							line[i]-=32;

						hash+=wn*line[i];
					}
					else if(wn){
						// solve the ,x problem
						if(line[i]==44)
							if(line[i+1]==88 && line[i+2]<33)
								i+=2;
						line[i]=0;

						// mode > 1, this word should be an operand
						if(mode>1){
							mode--;
							paraTmp<<=16;
							paraTmp+=i-wn;
						}
						else{ // this word may be label or OpSymbol
							symTmp=table.touch((char*)(line+i-wn),hash,-1,addr,labelHandler);

							if(!symTmp.size()){ // this word is a label
								symtable.push_back(string((char*)(line+i-wn)));
								labTmp=string((char*)(line+i-wn));
							}
							else{ // this word is a opSymbol
								opTmp=(char*)(line+i-wn);
								hashTmp=hash;
								if(mode)
									paraTmp=0;

								mode++;
							}
						}
						
						// if the last Op or dir got all its parameters 
						if(mode==1){
							mode=0;
							table.call(opTmp,hashTmp,paraTmp);

							if(((addr-addrTmp)+Tl) > 30){
								T_len.push_back(new int(Tl));
								Tl=0;
							}

							// write to intermediate file
							if(paraTmp==-2)
								fprintf(im_file,"%X\t%s\t%s\n",(start_addr+addrTmp),labTmp.c_str(),opTmp);
							else if(paraTmp & 0xFFFF0000){
								fprintf(im_file,"%X\t%s\t%s\t%s\t%s\n",
									(start_addr+addrTmp),
									labTmp.c_str(),opTmp,
									(char*)(line+((paraTmp>>16 & 0x0000FFFF))),
									(char*)(line+(paraTmp & 0x0000FFFF)));
								paraTmp>>=16;
							}
							else
								fprintf(im_file,"%X\t%s\t%s\t%s\n",(start_addr+addrTmp),labTmp.c_str(),opTmp,(char*)(line+paraTmp));
						}
						wn=0;
						hash=0;
					}
				}
			}
		}catch(char const* e){ // got an error or end signal
			if(!(string(e).compare("ended"))){ // if is end signal
				T_len.push_back(new int(Tl));
				fprintf(im_file,"\t%s\t%s\t%s\n",labTmp.c_str(),opTmp,(char*)(line+paraTmp));
				fprintf(im_file,"...=== SYMTAB ===\n.\n");
				string symNameTmp;
				int symtable_size=symtable.size();
				for (int i = 0; i < symtable_size; ++i)
				{
					symNameTmp=symtable.front();
					fprintf(im_file,".%s\t%s\n",symNameTmp.c_str(),table.call(symNameTmp,0).c_str());
					symtable.pop_front();
					symtable.push_back(symNameTmp);
				}
				// while(!symtable.empty()){
				// 	symNameTmp=symtable.front();
				// 	symtable.pop_front();
				// }
				fprintf(im_file, "...=== SIZE : %X ===\n",addr );

				// printf("dumping hash table\n");
				// table.dump();

				src_file.close();
				fclose(im_file);
			}
			else{ // if is error
				src_file.close();
				fclose(im_file);

				throw e;
			}
		}
	}

	void import(){
		int length=sizeof(op2addradd)/sizeof(Op2AddrAdd);
		for (int i = 0; i < length; ++i)
			table.add(op2addradd[i].name,((op2addradd[i].addradd<<4)+op2addradd[i].NofPara),opHandler);
		
		table.add("START",0,d_start);
		table.add("END",0,d_end);
		table.add("BYTE",1,d_wb);
		table.add("WORD",3,d_wb);
		table.add("RESB",1,d_resx);
		table.add("RESW",3,d_resx);
	}

	string labelHandler(int addr,int d){
		static char Strtmp[40];
		if(!mode)
			throw "label already defined!";
		else if(mode==-1){
			sprintf(Strtmp,"%X",addr+start_addr);
			return string(Strtmp);
		}
		else
			return "";

	}

	string opHandler(int para,int s){
		int addr_to_add=para>>4;
		if(s==-1){
			addr+=addr_to_add;
			Tl+=addr_to_add;
			mode+=para & 0x0000000F;
		}
		return "`";
	}

	string d_start(int d,int s){
		if(s!=-1){
			sscanf((char*)(line+s),"%x",&start_addr);
			startLab=labTmp;
		}
		else
			mode+=1;
		return "`";
	}

	string d_end(int d,int s){
		if(s!=-1){
			mode=-1;
			throw "ended";
		}
		else
			mode+=1;
		return "`";
	}

	string d_wb(int n,int s){
		if(s!=-1){
			int addraddTmp;
			if((*(line+s))=='C')
				addraddTmp=(strlen((char*)(line+s))-3);
			else if((*(line+s))=='X')
				addraddTmp=(strlen((char*)(line+s))-2)>>1;
			else
				addraddTmp=n;

			Tl+=addraddTmp;
			addr+=addraddTmp;
		}
		else
			mode+=1;
		return "`";
	}

	string d_resx(int n,int s){
		if(s!=-1){
			int addraddTmp;
			sscanf((char*)(line+s),"%d",&addraddTmp);
			addr+=(addraddTmp)*n;
		}
		else
			mode+=1;
		return "`";
	}
}

// namespace pass2 ===================================
namespace pass2 {

	int addr,mode;

	fstream im_file;
	FILE* des_file;

	void addToT(char*);
	void import();

	//OP code Table
	struct OpTable
	{
		string name;
		int opcodehex,format;
	}optable[59]={
		"ADD",0x18,3,
		"ADDF",0x58,3,
		"ADDR",0x90,2,
		"AND",0x40,3,
		"CLEAR",0xB4,2,
		"COMP",0x28,3,
		"COMPF",0x88,3,
		"COMPR",0xA0,2,
		"DIV",0x24,3,
		"DIVF",0x64,3,
		"DIVR",0x9C,2,
		"FIX",0xC4,1,
		"FLOAT",0xC0,1,
		"HIO",0xF4,1,
		"J",0x3C,3,
		"JEQ",0x30,3,
		"JGT",0x34,3,
		"JLT",0x38,3,
		"JSUB",0x48,3,
		"LDA",0x00,3,
		"LDB",0x68,3,
		"LDCH",0x50,3,
		"LDF",0x70,3,
		"LDL",0x08,3,
		"LDS",0x6C,3,
		"LDT",0x74,3,
		"LDX",0x04,3,
		"LPS",0xD0,3,
		"MUL",0x20,3,
		"MULF",0x60,3,
		"MULR",0x98,2,
		"NORM",0xC8,1,
		"OR",0x44,3,
		"RD",0xD8,3,
		"RMO",0xAC,2,
		"RSUB",0x4C,3,
		"SHIFTL",0xA4,2,
		"SHIFTR",0xA8,2,
		"SIO",0xF0,1,
		"SSK",0xEC,3,
		"STA",0x0C,3,
		"STB",0x78,3,
		"STCH",0x54,3,
		"STF",0x80,3,
		"STI",0xD4,3,
		"STL",0x14,3,
		"STS",0x7C,3,
		"STSW",0xE8,3,
		"STT",0x84,3,
		"STX",0x10,3,
		"SUB",0x1C,3,
		"SUBF",0x5C,3,
		"SUBR",0x94,2,
		"SVC",0xB0,2,
		"TD",0xE0,3,
		"TIO",0xF8,1,
		"TIX",0x2C,3,
		"TIXR",0xB8,2,
		"WD",0xDC,3
	};

	/**
	 * Symbol Handlers
	 * handler functions (char* (*)(int)) for processing all kinds of symbols
	 * @param  int para_given_when_added
	 * @param  int para_given_when_called
	 * @return whatever string
	 */
	string opHandler(int,int);
	string d_start(int,int);
	string d_end(int,int);
	string d_word(int,int);
	string d_byte(int,int);
	string d_resx(int,int);

	void main(char* intermediate_file_name,char* obj_file_name){

		// while(!pass1::T_len.empty()){
		// 	printf(">> %X ",*(pass1::T_len.front()) );
		// 	pass1::T_len.pop_front();
		// }
		// cout << endl;

		// while(!pass1::symtable.empty()){
		// 	printf("%s ",pass1::symtable.front().c_str());
		// 	pass1::symtable.pop_front();
		// }

		// cout << endl;

		// cout << "Start label:" << pass1::startLab << endl;
		
		// return;

		// get assembler prepared...
		import();



		// prepare file IO
		
		im_file.open(intermediate_file_name, ios::in);
		des_file=fopen(obj_file_name,"w");

		char line[256];
		int ln=1,ws,i,wc,wn,hash,tmpInt;
		char tmpChar[10];
		bool neol;
		mode=-1;
		
		try{

			//printf("====== file output started ======\n");
			fprintf(des_file,"H%-6s%06X%06X",pass1::startLab.c_str(),pass1::start_addr,pass1::addr);
			while(!im_file.eof()){
				im_file.getline(line, 256);
				//printf("\n%3d | %s",ln,line);

				ws=0; // word start
				wc=0; // word cnt
				i=0;
				hash=0;
				neol=true;
				while(neol){
					//printf("[%c:%d]",line[i],line[i] );

					if(line[i]==46)
						break;
					else if(line[i]<32){
						//printf("{%d}",line[i] );
						neol=!!line[i];
						line[i]=0;

						//printf("[%s]",(char*)(line+ws));

						switch(wc++){
							case 0:
								sscanf((char*)(line+ws),"%x",&addr);
								break;
							case 1:
								break;
							case 2:
								// printf("{%d}{%d}\n", hash, myHash(string((char*)(line+ws))));
								// getchar();
								addToT((char*)table.call((char*)(line+ws),hash,0).c_str());
								break;
							default:
								switch(mode){
									case 1:
										break;
									case 2:
										break;
									case 3:
										if(line[i-1]=='X' && line[i-2]==','){
											addToT((char*)"9039");
											//throw "DONT KNOW HOW TO Process indexed address...";
										}
										else{
											tmpInt=mode;
											mode=-1;
											addToT((char*)table.call((char*)(line+ws),hash,0).c_str());
											mode=tmpInt;
										}
										mode=0;
										break;
									case 4:
										break;
									case 5:
										if(line[ws]=='X' && line[i-1]=='\'' && line[ws+1]=='\''){
											line[i-1]=0;
											addToT((char*)(line+ws+2));
										}
										else if(line[ws]=='C' && line[i-1]=='\'' && line[ws+1]=='\''){
											line[i-1]=0;
											tmpInt=i-ws-3;
											for (int k = 0; k < tmpInt; ++k){
												sprintf((char*)(tmpChar+k*2),"%02X",*(line+ws+2+k));
												*(tmpChar+k*2+2)=0;
											}
											//printf("::%s::\n",tmpChar );
											addToT(tmpChar);
										}
										break;
									case 6:
										sscanf((char*)(line+ws),"%d",&tmpInt);
										sprintf(tmpChar,"%06X",tmpInt);
										addToT(tmpChar);
										break;
									default:
										break;
								}
								break;
						}

						ws=i+1;
						hash=0;
					}
					else{
						hash+=(i+1-ws)*line[i];
					}
					i++;

				}
				if(mode==3){
					addToT((char*)"0000");
					mode=0;
				}
				ln++;
			}
			
		}catch(char const* e){
			if(!(string(e).compare("ended"))){ // if is end signal
				fprintf(des_file,"\nE%06X\n",pass1::start_addr );
				//printf("====== file output ended ======\n");
				im_file.close();
				fclose(des_file);
			}
			else{ // if is error
				im_file.close();
				fclose(des_file);

				throw e;
			}
		}

	}

	void import(){
		int length=sizeof(optable)/sizeof(OpTable);
		for (int i = 0; i < length; ++i)
			table.add(optable[i].name,(optable[i].opcodehex<<4)+optable[i].format,opHandler);
		
		table.add("START",123,d_start);
		table.add("END",0,d_end);
		table.add("BYTE",1,d_byte);
		table.add("WORD",3,d_word);
		table.add("RESB",1,d_resx);
		table.add("RESW",3,d_resx);


		// printf("dumping table...\n");
		// table.dump();

		// printf("START address: %X\n",&d_start );
		// getchar();
	}

	void newT(bool NotFirst){
		if(NotFirst)
			pass1::T_len.pop_front();
		fprintf(des_file,"\nT%06X%02X",addr,*(pass1::T_len.front()));
		addToT((char*)0);
	}

	void addToT(char* str){
		static int tc;
		if(str==0){
			tc=0;
			return;
		}
		int tcAdd=strlen(str);
		if(tc+tcAdd > (*(pass1::T_len.front()))*2)
			newT(true);
		fprintf(des_file,"%s",str);
		tc+=tcAdd;
	}

	string opHandler(int para,int s){
		static char hexTmp[40];
		sprintf(hexTmp,"%02X",para>>4);

		mode=(para & 0x0000000F);
		return string(hexTmp);
	}

	string d_start(int d,int s){
		if(mode>=0)
			throw "Already START!";
		mode=0;
		newT(false);
		return "";
	}

	string d_end(int d,int s){
		throw "ended";
		return "";
	}

	string d_byte(int n,int s){
		mode=5;
		return "";
	}

	string d_word(int n,int s){
		mode=6;
		return "";
	}

	string d_resx(int n,int s){
		mode=0;
		return "";
	}
}

/**
 * the main function
 * usage:
 *     this_exe [src_file] [output_file] [intermediate_file]
 *
 * if no src_file name got, will prompt user to enter one.
 */
int main(int argc,char* argv[]){
	try{
		// hashTableModel_test();

		char*src_file_name;
		char*intermediate;
		src_file_name=new char[50];
		if(argc <= 1){
			printf("Please input the source file name:");
			scanf("%s",src_file_name);
		}
		else
			src_file_name=argv[1];

		if(argc <= 3)
			intermediate=(char *)"intermediate";
		else
			intermediate=argv[3];

		printf("Pass 1: processing...\n");
		pass1::main(src_file_name,intermediate);
		printf("Pass 1: process complete!\n");

		printf("Pass 2: processing...\n");
		if(argc <= 2)
			pass2::main(intermediate,(char*)"a.des");
		else
			pass2::main(intermediate,argv[2]);
		printf("Pass 2: process complete!\n");

	}catch(char const* e){
		printf("\nFATAL ERROR:\n\n\t\t%s\n\n",e);
	}

	return 0;
}

// debug functions ==================================
void hashTableModel_test(){
	// Test function pointer using intptr_t
	intptr_t funcptr=(intptr_t)&hashTableModel_callback;
	string (*funcPointer)(int);
	funcPointer=(string (*)(int))funcptr;
	funcPointer(29);

	// Test class Table...
	char* hi=(char*)"Hey";
	char* ho=(char*)"Hello";
	char* kk=(char*)"World!";

	Table table(10);

	table.add(hi,1,hashTableModel_callback);
	table.add(ho,2,hashTableModel_callback);
	table.add(kk,3,&hashTableModel_callback);

	printf("\ndumping table... (has value only)\n");
	table.dump();

	printf("\ndumping table... (all)\n");
	table.dump_(false);

	cout << endl;

	printf("calling hi with 99 => \"Hello\"...\n");
	table.call("Hello",99);

	cout << endl;

	printf("calling ?? with 99 => \"??\"...\n");
	table.call("??",99);
}

string hashTableModel_callback(int para_given_when_added,int para_given_when_called){
	printf("testing called: para_given_when_added=%d | para_given_when_called=%d\n",para_given_when_added,para_given_when_called);
	return "testing...";
}

void pb(int d){
    (d)?(puts("true")):(puts("false"));
}


