/**
 * myHash
 * hashCode generator
 * @param  input string
 * @return = str[0]*1 + str[1]*2 + str[2]*3 + str[3]*4 + ...
 */
int myHash(string str){
    int i=0;
    int code=0;
    while(str[i])
        code+=str[i++]*(i);
    return code;
}

/**
 * class Table
 * table :
 *      hashCode (index) => function_ptr + para_given_when_added
 * 
 * function interface: string (*)(int para_given_when_added,int para_given_when_called)
 */
class Table{
public:
    // constructor: int as its size
    Table(int);

    // add: put a record into the table, with or without hashCode
    void add(string name,int hashCode,int para_given_when_added,string (*funcPointer)(int,int));
    void add(string name,int para_given_when_added,string (*funcPointer)(int,int));

    // call: search a record and call its function with or without hashCode
    string call(string name,int hashCode,int para_given_when_called);
    string call(string name,int para_given_when_called);

    // touch: search a record. Call it if found,or add into (useful to attempt if a record is an undefined label)
    string touch(string name,int hashCode,int para_given_when_called,int para_given_when_added,string (*funcPointer)(int,int));
    string touch(string name,int para_given_when_called,int para_given_when_added,string (*funcPointer)(int,int));

    // dump: print out all information about the table, give dump_ false to print ALL records even they're empty
    void dump();
    void dump_(bool);

private:
    int size;
    int used;
    int requests;
    int attempts;
    int* paras;
    string* names;
    intptr_t* funcPointers;
};

Table::Table(int inputSize){
    size=inputSize;
    used=0;
    names=new string[size];
    paras=new int[size];
    funcPointers=new intptr_t[size];
    requests=0;
    attempts=0;

    for (int i = 0; i < size; ++i)
    {
        funcPointers[i]=0;
        paras[i]=0;
        names[i][0]=0;
    }
}

void Table::add(string name,int hashCode,int para_given_when_added,string (*funcPointer)(int,int)){
    if(used == size)
        throw "table overflowed. Please increase table size.";

    int i=hashCode%size;
    requests++;attempts++;

    // When the row is empty or found the same name, quit searching.
    while(*(funcPointers+i) && names[i].compare(name)){ i=(i+1)%size; attempts++; }

    if(!*(funcPointers+i))
        used++;

    names[i]=name;
    paras[i]=para_given_when_added;
    funcPointers[i]=(intptr_t)funcPointer;
}

void Table::add(string name,int para_given_when_added,string (*funcPointer)(int,int)){
    this->add(name,myHash(name),para_given_when_added,funcPointer);
}

string Table::call(string name,int hashCode,int para_given_when_called){
    int i=hashCode%size;
    bool found=false;
    requests++;attempts++;

    // search in records that have value
    while(*(funcPointers+i)){
        attempts++;

        // quit when there is a match
        if((found=(!name.compare(names[i]))))
            break;
        i=(i+1)%size;
    }

    if(!found)
        throw ("Symbol not found:"+name).c_str();

    string (*funcToBeCall)(int,int) = (string (*)(int,int)) (funcPointers[i]);
    return funcToBeCall(paras[i],para_given_when_called);
}

string Table::call(string name,int para_given_when_called){
    return call(name,myHash((string)name.c_str()),para_given_when_called);
}

string Table::touch(string name,int hashCode,int para_given_when_called,int para_given_when_added,string (*funcPointer)(int,int)){
    int i=hashCode%size;
    bool found=false;
    requests++;attempts++;

    // search in records that have value
    while(*(funcPointers+i)){
        attempts++;

        // quit when there is a match
        if((found=(!name.compare(names[i]))))
            break;
        i=(i+1)%size;
    }

    // when not found... add it!
    if(!found){
        names[i]=name;
        paras[i]=para_given_when_added;
        funcPointers[i]=(intptr_t)funcPointer;
        used++;
        return "";
    }

    string (*funcToBeCall)(int,int) = (string (*)(int,int)) (funcPointers[i]);
    return funcToBeCall(paras[i],para_given_when_called);
}

string Table::touch(string name,int para_given_when_called,int para_given_when_added,string (*funcPointer)(int,int)){
    return touch(name,myHash(name),para_given_when_called,para_given_when_added,funcPointer);
}

void Table::dump_(bool hasValueOnly){
    printf("======= Table info =======\nsize = %d | used = %d | %f is used | %d requests | %d attempts | %f attempts/request in average.\n",size,used,(double)used/(double)size,requests,attempts,(double)attempts/(double)requests);
    printf("======= Table Content =======\n[index] => _____name __para funcPointer\n");
    for (int i = 0; i < size; ++i)
    {
        if(!hasValueOnly || funcPointers[i])
            printf("[%05d] => %9s %6d %08X\n",i,names[i].c_str(),paras[i],funcPointers[i]);
    }
}

void Table::dump(){
    dump_(true);
}
