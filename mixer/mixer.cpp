#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>

#include "includes/elf64.h"

using namespace std;

#define ELFSize 64
#define shSize 64










class ExeFile{
  private:

  public:
    string filePath;

    Elf64_Ehdr *ELFhdr;


    vector<Elf64_Shdr*> shdrs;
    vector<string> shnames;

    vector<Elf64_Phdr*> phdrs;
    //vector<string> phnames;
    
    vector<Elf64_Sym*> symtblents;
    vector<string> symtblentnames;


    int symtblindex = -1;
    int symtblentNo = -1;
    int strtblshndx = -1;

    vector<unsigned char> fileContent;

    ExeFile(string filePath_, vector<unsigned char> buffer){
      filePath = filePath_;
      fileContent = buffer;
      

      //reading and storing elfhdr
      unsigned char* ELFBuffer = new unsigned char[ELFSize];
      for (int i=0; i<ELFSize; i++){
        ELFBuffer[i] = fileContent[i];
      }
      ELFhdr = (Elf64_Ehdr*) ELFBuffer;

      //reading and storing section headers
      for (int i=0; i<ELFhdr->e_shnum; i++){
        unsigned char* shdrBuffer = new unsigned char[ELFhdr->e_shentsize];
        for (int j=0; j<ELFhdr->e_shentsize; j++){
          shdrBuffer[j] = fileContent[ELFhdr->e_shoff + i * ELFhdr->e_shentsize + j];
        }
        shdrs.push_back((Elf64_Shdr*)shdrBuffer);

        if(shdrs[i]->sh_type == SHT_SYMTAB){
          symtblindex = i;
          symtblentNo = (shdrs[i]->sh_size)/(shdrs[i]->sh_entsize);
          //cout << "symbtble index: " << symtblindex << ", No: " << symtblentNo <<endl;
        }
      }

      //finding and storing section header names
      for (int i=0; i<ELFhdr->e_shnum; i++){
        string tempName = "";
        int counter = shdrs[ELFhdr->e_shstrndx]->sh_offset + shdrs[i]->sh_name;
        while(fileContent[counter]){
          tempName.push_back(fileContent[counter]);
          counter = counter + 1;
        }
        //cout << tempName << endl;
        shnames.push_back(tempName);
          
      }

      //reading and storing program headers
      for (int i=0; i<ELFhdr->e_phnum; i++){
        unsigned char* phdrBuffer = new unsigned char[ELFhdr->e_phentsize];
        for (int j=0; j<ELFhdr->e_phentsize; j++){
          phdrBuffer[j] = fileContent[ELFhdr->e_phoff + i * ELFhdr->e_phentsize + j];
        }
        phdrs.push_back((Elf64_Phdr*)phdrBuffer);
      }

      //finding and storing program header names

      //reading and storing symbol table entries
      for (int i=0; i<symtblentNo; i++){
        unsigned char* symtblentBuffer = new unsigned char[shdrs[symtblindex]->sh_entsize];
        for (int j=0; j<shdrs[symtblindex]->sh_entsize; j++){
          symtblentBuffer[j] = fileContent[shdrs[symtblindex]->sh_offset + i * shdrs[symtblindex]->sh_entsize + j];
        }
        symtblents.push_back((Elf64_Sym*)symtblentBuffer);
      }



      //finding string table index

      for (int i=0; i<shnames.size(); i++){
        if(shnames[i]==".strtab"){
          strtblshndx = i;
          //cout << "strtbleshndx: "<<strtblshndx << endl;
        } 
      }


      //finding and storing symbol table entry names
      for (int i=0; i<symtblentNo; i++){
        string tempName = "";
        
        int counter = shdrs[strtblshndx]->sh_offset + symtblents[i]->st_name;
        
        while(fileContent[counter]){
          tempName.push_back(fileContent[counter]);
          counter = counter + 1;
        }
        //cout << tempName << endl;
        symtblentnames.push_back(tempName);
          
      }

      for (int i=0; i<ELFhdr->e_shnum-3; i++){
        symtblentnames[i] = shnames[i];
      }

    }

};











class MixedExeFile{
  public:


    vector<ExeFile> exefiles;

    int sectionNo;

    Elf64_Ehdr ELFhdr;

    vector<Elf64_Shdr> shdrs;
    vector<string> shnames;

    vector<Elf64_Phdr> phdrs;
    
    
    vector<Elf64_Sym> symTbl;
    vector<string> symtblentnames;







    //text section
    vector<unsigned char> text;
    vector<unsigned int> textsizes;
    vector<unsigned int> mixedtextoffsets;
    int mixedtextsize = 0;
    Elf64_Shdr textShdr;
    Elf64_Phdr textPhdr;

    //bss section
    vector<unsigned int> bsssizes; 
    vector<int> mixedbssoffsets;
    int mixedbsssize = 0;
    int previousbssaddress = 0;
    int tempbssoffset = 0;
    int bssflag = 0;
    int firstfilewbss = 0;
    int bssAddrAllign = 0;
    int bsscount = 0;
    Elf64_Shdr bssShdr;
    Elf64_Phdr bssPhdr;
    

    //comment section
    Elf64_Shdr cmntShdr;
    vector<unsigned char> comment;

    //riscvattr
    Elf64_Shdr rvattrShdr;
    vector<unsigned char> rvattr;
    Elf64_Phdr rvattrPhdr;


    //symbol table section
    Elf64_Shdr symtblShdr;



    
    //shift amounts

    vector<int> shifts;
    int originaltextbssdiff;
    int currenttextbssdiff;

    //strtab
    vector<unsigned char> strtab;
    vector<int> strtabindexes;
    Elf64_Shdr strtblShdr;



    //section header string table section
    vector<unsigned char> shstrtab;
    Elf64_Shdr shstrtblShdr;

    //GNU stack program header
    Elf64_Phdr GNUStckPhdr;


    

    

    MixedExeFile(vector <ExeFile> exefiles_){
      exefiles = exefiles_;

      for (int i=0; i<exefiles.size(); i++){
        textsizes.push_back(0);
        bsssizes.push_back(0);
        for (int j=0; j<exefiles[i].shnames.size(); j++){
          if(exefiles[i].shnames[j]==".text"){
            textsizes[i] = exefiles[i].shdrs[j]->sh_size;
          }
          else if (exefiles[i].shnames[j]==".bss"){
            bsssizes[i] = exefiles[i].shdrs[j]->sh_size;
            mixedbsssize = mixedbsssize + bsssizes[i];
            bssflag = 1;
            sectionNo = bssflag + 7;
            firstfilewbss = i;
          }
        }
      }

      //mixed text size calculation
      for (int i=0; i<exefiles.size(); i++){
        if(exefiles[i].shnames[1]==".text"){
          mixedtextoffsets.push_back(mixedtextsize);
          mixedtextsize = mixedtextsize + exefiles[i].shdrs[1]->sh_size - 2;
        }
        else{
          cout << "wrong section name for .text" << endl;
        }
      }
      mixedtextsize = mixedtextsize + 2;



      //bss placement
      previousbssaddress = mixedtextsize;

      
      for (int i=0; i<exefiles.size(); i++){
        mixedbssoffsets.push_back(-1);
        if(bsssizes[i]!=0){
          if(exefiles[i].shnames[2]==".bss"){
            bsscount = bsscount + 1;
            if(previousbssaddress%exefiles[i].shdrs[2]->sh_addralign==0){
              tempbssoffset = previousbssaddress;
              mixedbssoffsets[i] = tempbssoffset;
            }
            else{
              tempbssoffset = ((previousbssaddress/exefiles[i].shdrs[2]->sh_addralign) + 1) * exefiles[i].shdrs[2]->sh_addralign;
              mixedbssoffsets[i] = tempbssoffset;
            }
            previousbssaddress = tempbssoffset + exefiles[i].shdrs[2]->sh_size;
            if(exefiles[i].shdrs[2]->sh_addralign > bssAddrAllign) bssAddrAllign = exefiles[i].shdrs[2]->sh_addralign;
          }
        }
      }
      

      //shift calculation
      for (int i=0; i<exefiles.size(); i++){
        if(bsssizes[i]==0){
          shifts.push_back(0);
        }
        else{
          originaltextbssdiff = exefiles[i].shdrs[2]->sh_addr - exefiles[i].shdrs[1]->sh_addr;
          currenttextbssdiff = mixedbssoffsets[i] - mixedtextoffsets[i];
          cout << "original diff: " << hex << originaltextbssdiff << endl;
          cout << "current diff: " << hex << currenttextbssdiff << endl;

          shifts.push_back(currenttextbssdiff - originaltextbssdiff);
        }
      }

      //debug prints
      // cout << "mixed text size: " << hex << mixedtextsize << endl;
      // for (int i=0; i<mixedtextoffsets.size(); i++) cout<<"text offsets:"<<hex<<mixedtextoffsets[i]<<endl;
      // for (int i=0; i<mixedbssoffsets.size(); i++) cout<<"bss offsets:"<<hex<<mixedbssoffsets[i]<<endl;
      // cout << "end of bss address: " << hex << previousbssaddress << endl;
      // for (int i=0; i<shifts.size(); i++) cout<< "shifts:" << hex << shifts[i] << endl;
     
      
    }

    void mixTextSection(){

      int temptextoffset;
      int temptextsize;
      unsigned char tempByte0, tempByte1, tempByte2, tempByte3, tempByte4, tempByte5, tempByte6, tempByte7;

      int copyCounter = 0;


      int auipcbase = 0;
      int nextoffset = 0;
      int oldjumpamount = 0;
      int newjumpamount = 0;
      int newauipcbase = 0;
      int newnextoffset = 0;

      for (int i=0; i<exefiles.size(); i++){

        temptextoffset = exefiles[i].shdrs[1]->sh_offset;
        temptextsize = textsizes[i];

        for (int j=0; j<temptextsize - 2; j=j+2){

          tempByte0 = exefiles[i].fileContent[temptextoffset+j];
          tempByte1 = exefiles[i].fileContent[temptextoffset+j+1];

          if ((tempByte0%4)== 0 || (tempByte0%4)== 1 || (tempByte0%4)== 2){ //16 bit instruction
            text.push_back(tempByte0);
            text.push_back(tempByte1);
            // cout << "pc: " << hex << copyCounter << ", temp0: " << hex << (unsigned int)tempByte0 << ", temp1: " << hex << (unsigned int)tempByte1 << endl;
            copyCounter = copyCounter + 2;
          }
          else if((tempByte0%4)== 3) {  //32 bit instruction
            tempByte2 = exefiles[i].fileContent[temptextoffset+j+2];
            tempByte3 = exefiles[i].fileContent[temptextoffset+j+3];

            if((tempByte0%128)!= 23){  //32 bit instruction not auipc
              text.push_back(tempByte0);
              text.push_back(tempByte1);
              text.push_back(tempByte2);
              text.push_back(tempByte3);
              
              // cout << "pc: " << hex << copyCounter << ", temp0: " << hex << (unsigned int)tempByte0 << ", temp1: " << hex << (unsigned int)tempByte1; 
              // cout << ", temp2: " << hex << (unsigned int)tempByte2 << ", temp3: " << hex << (unsigned int)tempByte3 << endl;
              copyCounter = copyCounter + 4;
              j = j + 2;
            }
            else if ((tempByte0%128)== 23){//auipc
              tempByte4 = exefiles[i].fileContent[temptextoffset+j+4];
              tempByte5 = exefiles[i].fileContent[temptextoffset+j+5];
              tempByte6 = exefiles[i].fileContent[temptextoffset+j+6];
              tempByte7 = exefiles[i].fileContent[temptextoffset+j+7];



              auipcbase = tempByte1/16 + tempByte2*16 + tempByte3*16*16*16;
              nextoffset = (tempByte6/16) + tempByte7*16;

              if(nextoffset > 2047) {
                nextoffset = -1 * (4096 - nextoffset);
              }

              oldjumpamount = (auipcbase << 12) + nextoffset;


              newjumpamount = oldjumpamount + shifts[i];
              cout << "new jump amount: " << hex << newjumpamount << endl;

              if((newjumpamount%4096) > 2047 ){
                newauipcbase = (newjumpamount/4096) + 1;
                newnextoffset = (newjumpamount%4096) - 4096;
              }
              else{
                newauipcbase = newjumpamount/4096;
                newnextoffset = newjumpamount%4096;
              }
              cout << "newauipcbase:" <<newauipcbase <<endl;
              cout << "newnextoffset:" <<newnextoffset <<endl;


              tempByte1 = (tempByte1%16) + ((newauipcbase%16) << 4);
              tempByte2 = (newauipcbase/16)%256;
              tempByte3 = newauipcbase>>12;

              cout << "temp6:" <<hex<<(unsigned int)tempByte6 <<endl;
              tempByte6 = (tempByte6%16) + ((newnextoffset%16)<<4);
              cout << "temp6:" <<hex<<(unsigned int)tempByte6 <<endl;
              tempByte7 = newnextoffset/16;

              text.push_back(tempByte0);
              text.push_back(tempByte1);
              text.push_back(tempByte2);
              text.push_back(tempByte3);
              text.push_back(tempByte4);
              text.push_back(tempByte5);
              text.push_back(tempByte6);
              text.push_back(tempByte7);
              
              cout << "pc: " << hex << copyCounter << ", temp0: " << hex << (unsigned int)tempByte0 << ", temp1: " << hex << (unsigned int)tempByte1; 
              cout << ", temp2: " << hex << (unsigned int)tempByte2 << ", temp3: " << hex << (unsigned int)tempByte3;
              cout << ", temp4: " << hex << (unsigned int)tempByte4 << ", temp5: " << hex << (unsigned int)tempByte5;
              cout << ", temp6: " << hex << (unsigned int)tempByte6 << ", temp7: " << hex << (unsigned int)tempByte7 << endl;
              copyCounter = copyCounter + 8;
              j = j + 6;


            }
          }


        }

        
      }
      //push back riscv return instruction
      tempByte0 = (unsigned char)(0x82);
      tempByte1 = (unsigned char)(0x80);
      text.push_back(tempByte0);
      text.push_back(tempByte1);
    }

    void getCmntRVAttrContent(){
      if (exefiles.size() == 0) {cout << "error: no exe file!!" << endl; return; }
      

      int commentIdx = -1;
      int rvattrIdx = -1;
      for (int i=0; i<exefiles[0].shnames.size(); i++){
        if(exefiles[0].shnames[i]== ".comment"){
          commentIdx = i;
        }
        else if (exefiles[0].shnames[i]== ".riscv.attributes"){
          rvattrIdx = i;
        }
      }

      if(commentIdx == -1 || rvattrIdx == -1) {cout << "error: comment or rvattr not found !!!" << endl; return;}

      for (int i=0; i<exefiles[0].shdrs[commentIdx]->sh_size; i++){
        comment.push_back(exefiles[0].fileContent[exefiles[0].shdrs[commentIdx]->sh_offset+i]);
      }
      for (int i=0; i<exefiles[0].shdrs[rvattrIdx]->sh_size; i++){
        rvattr.push_back(exefiles[0].fileContent[exefiles[0].shdrs[rvattrIdx]->sh_offset+i]);
      }

      cout << "rvattr size:"<<hex<<rvattr.size()<<endl;




    }

    void generateShStrTab(){

      int shstrtabsize = 0;
      int shstrtaboff = 0;

      if(bssflag == 1){
        shstrtabsize = exefiles[firstfilewbss].shdrs[7]->sh_size;
        shstrtaboff = exefiles[firstfilewbss].shdrs[7]->sh_offset;

        for (int i=0 ; i<shstrtabsize; i++){
          shstrtab.push_back(exefiles[firstfilewbss].fileContent[shstrtaboff+i]);
        }
      }
      else if (bssflag == 0){
        
        shstrtabsize = exefiles[0].shdrs[6]->sh_size;
        shstrtaboff = exefiles[0].shdrs[6]->sh_offset;
        for (int i=0 ; i<shstrtabsize; i++){
          shstrtab.push_back(exefiles[0].fileContent[shstrtaboff+i]);
        }
      }
      
      for (int i=0;i<shstrtab.size();i++) cout<<shstrtab[i];
      cout<<endl;
      
    }

    void generateStrTab(){
      

      strtabindexes.push_back(0);
      strtab.push_back('\0');
      strtabindexes.push_back(1);
      strtab.push_back('m');strtab.push_back('a');strtab.push_back('i');strtab.push_back('n');strtab.push_back('\0');
      strtabindexes.push_back(6);

      int tempindex = 6;
      string tempstr;


      //adding text labels
      for(int i=0; i<exefiles.size();i++){
        tempstr.clear();
        tempstr = tempstr + "text" + to_string(i+1);

        for(int j=0; j<tempstr.size(); j++) strtab.push_back(tempstr[j]);
        strtab.push_back('\0');
        
        tempindex = tempindex + tempstr.size() + 1;
        strtabindexes.push_back(tempindex);

      }

      //adding bss labels
      for(int i=0; i<exefiles.size();i++){
        if(bsssizes[i]!=0){
          tempstr = "";
          tempstr = tempstr + "bss" + to_string(i+1);

          for(int j=0; j<tempstr.size(); j++) strtab.push_back(tempstr[j]);
          strtab.push_back('\0');
          
          tempindex = tempindex + tempstr.size() + 1;
          strtabindexes.push_back(tempindex);
        }

      }

      strtabindexes.pop_back();

      
      for (int i=0;i<strtab.size();i++) cout<<strtab[i];
      cout << endl;

      for (int i=0;i<strtabindexes.size();i++) cout<<strtabindexes[i]<<endl;
      
      


    }

    void generateELFhdr(){
      for (int i=0; i<EI_NIDENT; i++) ELFhdr.e_ident[i] = exefiles[0].ELFhdr->e_ident[i];
      ELFhdr.e_type = exefiles[0].ELFhdr->e_type;
      ELFhdr.e_machine = exefiles[0].ELFhdr->e_machine;
      ELFhdr.e_version = exefiles[0].ELFhdr->e_version;
      ELFhdr.e_entry = 0x0;
	    ELFhdr.e_flags =  exefiles[0].ELFhdr->e_flags;	/* Architecture-specific flags. */
	    ELFhdr.e_ehsize = exefiles[0].ELFhdr->e_ehsize;	/* Size of ELF header in bytes. */
	    ELFhdr.e_phentsize = exefiles[0].ELFhdr->e_phentsize;	/* Size of program header entry. */
	    ELFhdr.e_phnum = 3 + bssflag;	/* Number of program header entries. */
      ELFhdr.e_shentsize = exefiles[0].ELFhdr->e_shentsize;	/* Size of section header entry. */
      ELFhdr.e_shnum = 7 + bssflag;	/* Number of section header entries. */
      ELFhdr.e_shstrndx = 6 + bssflag;	/* Section name strings section. */
	    ELFhdr.e_shoff = ELFhdr.e_ehsize;	/* Section header file offset. */
      ELFhdr.e_phoff = ELFhdr.e_ehsize + ELFhdr.e_shentsize * ELFhdr.e_shnum;	/* Program header file offset. */      
    }


    void generateShdrs(){
      

      //first null section header
      shdrs.push_back(*(exefiles[0].shdrs[0]));

      //text section header
      textShdr.sh_addr = 0x0; textShdr.sh_addralign = 0x2; textShdr.sh_entsize = 0x0; textShdr.sh_flags = 0x6; textShdr.sh_info = 0x0; textShdr.sh_link = 0x0;
      textShdr.sh_name = exefiles[0].shdrs[1]->sh_name; textShdr.sh_offset = ELFhdr.e_ehsize + ELFhdr.e_shentsize * ELFhdr.e_shnum + ELFhdr.e_phentsize * ELFhdr.e_phnum; 
      textShdr.sh_size = mixedtextsize; textShdr.sh_type = exefiles[0].shdrs[1]->sh_type;
      shdrs.push_back(textShdr);

      cout<<"mixed text size:"<<text.size()<<endl;

      //bss section header
      if (bssflag == 1){
        for (int i=0; i<mixedbssoffsets.size(); i++){
          if(mixedbssoffsets[i]!= -1){
            bssShdr.sh_addr = mixedbssoffsets[i];
            break;
          }  
        }
        cout <<"bssShdraddr: " << bssShdr.sh_addr << endl;
        
        bssShdr.sh_addralign = bssAddrAllign; bssShdr.sh_entsize = 0x0; bssShdr.sh_flags = 0x3; bssShdr.sh_info = 0x0; bssShdr.sh_link = 0x0; bssShdr.sh_name = 0x21;
        bssShdr.sh_offset = bssShdr.sh_addr; bssShdr.sh_size = mixedbsssize; bssShdr.sh_type = 0x8;
        shdrs.push_back(bssShdr);
        
      }

      //comment section header
      cmntShdr.sh_addr = 0x0; cmntShdr.sh_addralign = 0x1; cmntShdr.sh_entsize = 0x1; cmntShdr.sh_flags = 0x30; cmntShdr.sh_info = 0x0; cmntShdr.sh_link = 0x0; 
      if (bssflag == 1){ cmntShdr.sh_name = 0x26;} else if (bssflag == 0){cmntShdr.sh_name = 0x21;}
      cmntShdr.sh_offset = textShdr.sh_offset + textShdr.sh_size;
      cmntShdr.sh_size = 0x10; cmntShdr.sh_type = textShdr.sh_type;
      shdrs.push_back(cmntShdr);



      // riscv attributes section header
      rvattrShdr.sh_addr = 0x0; rvattrShdr.sh_addralign = 0x1; rvattrShdr.sh_entsize = 0x0; rvattrShdr.sh_flags = 0x0; rvattrShdr.sh_info = 0x0; rvattrShdr.sh_link = 0x0; 
      if (bssflag == 1){ rvattrShdr.sh_name = 0x2f;} else if (bssflag == 0){rvattrShdr.sh_name = 0x2a;}
      rvattrShdr.sh_offset = cmntShdr.sh_offset + cmntShdr.sh_size;
      rvattrShdr.sh_size = 0x33; rvattrShdr.sh_type = 0x70000003;
      shdrs.push_back(rvattrShdr);


      // symbol table section header
      symtblShdr.sh_addr = 0x0; symtblShdr.sh_addralign = 0x8; symtblShdr.sh_entsize = 0x18; symtblShdr.sh_flags = 0x0; symtblShdr.sh_info = 0x6 + bssflag; 
      symtblShdr.sh_link = 0x5 + bssflag; 
      symtblShdr.sh_name = 0x1;

      //fixing symbol table alignment
      if((rvattrShdr.sh_offset + rvattrShdr.sh_size) % 8 == 0){
        symtblShdr.sh_offset = rvattrShdr.sh_offset + rvattrShdr.sh_size;
      }
      else {
        symtblShdr.sh_offset = (((rvattrShdr.sh_offset + rvattrShdr.sh_size) / 8) + 1) * 8;
        for (int i=0; i<symtblShdr.sh_offset-(rvattrShdr.sh_offset + rvattrShdr.sh_size); i++){
          rvattr.push_back('\0');
        }
      }   

      symtblShdr.sh_size = symtblShdr.sh_entsize * (4 + bssflag + 1 + exefiles.size() + bsscount);  // null; text; bss; comment; riscv.attributes; main; text1-n bss1-n
      // cout << "symtblShdr.sh_size:" << hex << symtblShdr.sh_size << endl;
      symtblShdr.sh_type = 0x2;
      shdrs.push_back(symtblShdr);


      //string table section header
      strtblShdr.sh_addr = 0x0; strtblShdr.sh_addralign = 0x1; strtblShdr.sh_entsize = 0x0; strtblShdr.sh_flags = 0x0; strtblShdr.sh_info = 0x0;
      strtblShdr.sh_link = 0x0; strtblShdr.sh_name = 0x9; strtblShdr.sh_offset = symtblShdr.sh_offset + symtblShdr.sh_size; strtblShdr.sh_size = strtab.size(); strtblShdr.sh_type = 0x3;
      shdrs.push_back(strtblShdr);

      //section header string table section header
      shstrtblShdr.sh_addr = 0x0; shstrtblShdr.sh_addralign = 0x1; shstrtblShdr.sh_entsize = 0x0; shstrtblShdr.sh_flags = 0x0; shstrtblShdr.sh_info = 0x0;
      shstrtblShdr.sh_link = 0x0; shstrtblShdr.sh_name = 0x11; shstrtblShdr.sh_offset = strtblShdr.sh_offset + strtblShdr.sh_size; 
      shstrtblShdr.sh_size = shstrtab.size(); shstrtblShdr.sh_type = 0x3;
      shdrs.push_back(shstrtblShdr);










      // cout<< "debug:"  <<exefiles[0].shdrs[6]->sh_name << endl;
      // cout<< hex <<exefiles[1].shdrs[7]->sh_name << endl;
      // for (int i=0; i<mixedbssoffsets.size();i++) cout<< mixedbssoffsets[i] << endl;
      //cout << shdrs[0].sh_addralign << endl;
    }

    void generatePhdrs(){

      //riscv attributes program header
      rvattrPhdr.p_align = rvattrShdr.sh_addralign; rvattrPhdr.p_filesz = rvattrShdr.sh_size; rvattrPhdr.p_flags = 0x4;
      rvattrPhdr.p_memsz = rvattrShdr.sh_size; rvattrPhdr.p_offset = rvattrShdr.sh_offset; rvattrPhdr.p_paddr = rvattrShdr.sh_addr;
      rvattrPhdr.p_type = rvattrShdr.sh_type; rvattrPhdr.p_vaddr = rvattrShdr.sh_addr;
      phdrs.push_back(rvattrPhdr);

      //text program header
      textPhdr.p_align = textShdr.sh_addralign; textPhdr.p_filesz = textShdr.sh_size; textPhdr.p_flags = 0x5; textPhdr.p_memsz = textShdr.sh_size;
      textPhdr.p_offset = textShdr.sh_offset; textPhdr.p_paddr = textShdr.sh_addr; textPhdr.p_type = 0x1; textPhdr.p_vaddr = textShdr.sh_addr;
      phdrs.push_back(textPhdr);


      //bss program header
      if(bssflag == 1){
        bssPhdr.p_align =  bssShdr.sh_addralign; bssPhdr.p_filesz = 0x0; bssPhdr.p_flags = 0x6; bssPhdr.p_memsz = bssShdr.sh_size; 
        bssPhdr.p_offset = bssShdr.sh_offset; bssPhdr.p_paddr = bssShdr.sh_addr; bssPhdr.p_type = 0x1; bssPhdr.p_vaddr = bssShdr.sh_addr; 
        phdrs.push_back(bssPhdr);
      }
      

      // GNU stack program header
      GNUStckPhdr.p_align = 0x10; GNUStckPhdr.p_filesz = 0x0; GNUStckPhdr.p_flags = 0x6; GNUStckPhdr.p_memsz = 0x0; GNUStckPhdr.p_offset = 0x0;
      GNUStckPhdr.p_paddr = 0x0; GNUStckPhdr.p_type = 0x6474e551; GNUStckPhdr.p_vaddr = 0x0;
      phdrs.push_back(GNUStckPhdr);


      //cout<< "debug:"  <<exefiles[1].phdrs[3]->p_type << endl; 
      

    }

    void generateSymTbl(){
      
      //null symtbl entry
      Elf64_Sym nullent;
      nullent.st_info = 0x0; nullent.st_name = 0x0; nullent.st_other = 0x0; nullent.st_shndx = 0x0; nullent.st_size = 0x0; nullent.st_value = 0x0;
      symTbl.push_back(nullent);

      //.text symtbl entry
      Elf64_Sym textent;
      textent.st_info = STT_SECTION; textent.st_name = 0x0; textent.st_other = 0x0; textent.st_shndx = 0x1; textent.st_size = textShdr.sh_size; textent.st_value = textShdr.sh_addr;
      symTbl.push_back(textent);

      //.bss  symtbl entry
      if(bssflag == 1){
        Elf64_Sym bssent;
        bssent.st_info = STT_SECTION; bssent.st_name = 0x0; bssent.st_other = 0x0; bssent.st_shndx = 0x2; bssent.st_size = bssShdr.sh_size; bssent.st_value = bssShdr.sh_addr;
        symTbl.push_back(bssent);
      }

      //.comment symtbl entry
      Elf64_Sym cmntent;
      cmntent.st_info = STT_SECTION; cmntent.st_name = 0x0; cmntent.st_other = 0x0; cmntent.st_shndx = 0x2 + bssflag; 
      cmntent.st_size = cmntShdr.sh_size; cmntent.st_value = cmntShdr.sh_addr;
      symTbl.push_back(cmntent);

      //.rvattr symtbl entry
      Elf64_Sym rvent;
      rvent.st_info = STT_SECTION; rvent.st_name = 0x0; rvent.st_other = 0x0; rvent.st_shndx = 0x3 + bssflag; 
      rvent.st_size = rvattrShdr.sh_size; rvent.st_value = rvattrShdr.sh_addr;
      symTbl.push_back(rvent);

      //main symtbl entry
      Elf64_Sym mainent;
      mainent.st_info = STB_HIOS; mainent.st_name = strtabindexes[1]; mainent.st_other = 0x0; mainent.st_shndx = 0x1; mainent.st_size = textShdr.sh_size;
      mainent.st_value = textShdr.sh_addr;
      symTbl.push_back(mainent);


      //mixed text entries
      for (int i=0; i<exefiles.size(); i++){
        Elf64_Sym mtextent;
        mtextent.st_info = 0x0; mtextent.st_name = strtabindexes[2+i]; mtextent.st_other = 0x0; mtextent.st_shndx = 0x1; mtextent.st_size = exefiles[i].shdrs[1]->sh_size - 2;
        mtextent.st_value = mixedtextoffsets[i];
        symTbl.push_back(mtextent);
      }

      if(bssflag == 1){
        int counter = 0;
        for(int i=0; i<mixedbssoffsets.size(); i++){
          if(mixedbssoffsets[i]==-1){
            continue;
          }
          else{
            Elf64_Sym mbssent;
            mbssent.st_info = 10; mbssent.st_name = strtabindexes[2+exefiles.size()+counter]; mbssent.st_other = 0x0; mbssent.st_shndx = 0x1 + bssflag; 
              mbssent.st_size = bsssizes[i]; mbssent.st_value = mixedbssoffsets[i];
            symTbl.push_back(mbssent);
            counter = counter + 1;
          }
        }
      }


      cout<< "debug:"  << symTbl.size() <<endl; 
      cout<< "debug2:"  << symtblShdr.sh_size <<endl; 
      //for (int i=0; i<mixedbssoffsets.size(); i++) cout<< "debug:"  << mixedbssoffsets[i] <<endl; 

    }

    void writeOutputFile(){
      int bytesWritten = 0;
      
      fstream f;
      f.open("p_out", ios::out | ios::binary);

      if(f.is_open()){

        //ELF header
        f.write(reinterpret_cast<char*>(&ELFhdr), sizeof(Elf64_Ehdr));

        bytesWritten = bytesWritten + sizeof(Elf64_Ehdr);
        // cout<< "bytes written:" << hex << bytesWritten << endl;
        
        //section headers
        for (int i=0; i<shdrs.size(); i++){
          f.write(reinterpret_cast<char*>(&shdrs[i]), sizeof(Elf64_Shdr));
        }

        bytesWritten = bytesWritten + sizeof(Elf64_Shdr) * shdrs.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //program headers
        for (int i=0; i<phdrs.size(); i++){
          f.write(reinterpret_cast<char*>(&phdrs[i]), sizeof(Elf64_Phdr));
        }
        bytesWritten = bytesWritten + sizeof(Elf64_Phdr) * phdrs.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //text
        for (int i=0; i<text.size(); i++){
          f.write(reinterpret_cast<char*>(&text[i]), sizeof(unsigned char));
        }
        bytesWritten = bytesWritten + sizeof(unsigned char) * text.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //comment
        for (int i=0; i<comment.size(); i++){
          f.write(reinterpret_cast<char*>(&comment[i]), sizeof(unsigned char));
        }
        bytesWritten = bytesWritten + sizeof(unsigned char) * comment.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //rvattr
        for (int i=0; i<rvattr.size(); i++){
          f.write(reinterpret_cast<char*>(&rvattr[i]), sizeof(unsigned char));
        }
        bytesWritten = bytesWritten + sizeof(unsigned char) * rvattr.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;
        
        

        //symtbl
        for (int i=0; i<symTbl.size(); i++){
          f.write(reinterpret_cast<char*>(&symTbl[i]), sizeof(Elf64_Sym));
        }
        bytesWritten = bytesWritten + sizeof(Elf64_Sym) * symTbl.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //strtbl
        for (int i=0; i<strtab.size(); i++){
          f.write(reinterpret_cast<char*>(&strtab[i]), sizeof(unsigned char));
        }
        bytesWritten = bytesWritten + sizeof(unsigned char) * strtab.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        //shstrtbl
        for (int i=0; i<shstrtab.size(); i++){
          f.write(reinterpret_cast<char*>(&shstrtab[i]), sizeof(unsigned char));
        }
        bytesWritten = bytesWritten + sizeof(unsigned char) * shstrtab.size();
        // cout<< "bytes written:" << hex << bytesWritten << endl;

        f.close();
      }
      else cout<<"ERROR!!\n";


    }

    void generateOutput(){
      mixTextSection();
      getCmntRVAttrContent();
      generateShStrTab();
      generateStrTab();


      generateELFhdr();
      generateShdrs();
      generatePhdrs();
      generateSymTbl();

      cout<<bssShdr.sh_offset<<endl;
      writeOutputFile();
      
      
      

    }
};

int main(){


  string a("p2");

  const char* jaber = a.c_str();
  ifstream input2 (jaber, ios::binary );
  vector<unsigned char> buffer2(istreambuf_iterator<char>(input2), {});
  
  ExeFile p2("p2", buffer2);



  ifstream input ("p", ios::binary );
  vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
  
  ExeFile p("p", buffer);


  vector<ExeFile> files;

  files.push_back(p);
  files.push_back(p2);
  //files.push_back(p2);


  MixedExeFile o(files);
  o.generateOutput();

  cout<<"koskos\n";
  ifstream input3 ("p_out", ios::binary );
  vector<unsigned char> buffer3(istreambuf_iterator<char>(input3), {});

  

  
  return 0;
  
}

