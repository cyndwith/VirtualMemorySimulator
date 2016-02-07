#ifndef VIRTUALPROGRAM_H_INCLUDED
#define VIRTUALPROGRAM_H_INCLUDED
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "randomFunction.h"
#include "statistics.h"
using namespace std;

/*
"virtualProgram.h - file contains the following functions/Classes:

1. inst (Instruction Class) : Contains the Instruction Details : OpCode, Operand Count, Operand Address (if any), Branch Address
        Instruction class has two internal function :
            a) setValues : Initializes the internal variables OpCode, Operand Count, Operand Address (if any), Branch Address
            b) print : Prints the contents of the Instruction (Object)
2. virtual Program (Function) : Whenever the function is called it generates a random instruction with following values
        Random Instruction generated may have:
            a) Opcode : 0-3 (any value from 0-load, 1-store,2-Branch,3-Other)
            b) # Operands : 0-3 (any values 0-3 - at most 3 operands)
            c) Operand Address : ( if # Operands is non-zero -> we generate random 48 bits with Spatial Locality (last 12bits) as operands virtual address
            d) Branch Address : We generate random 48 bits address -> it is used to update PC if we have a branch instruction
*/


/*
1. inst (Instruction Class) : Contains the Instruction Details : OpCode, Operand Count, Operand Address (if any), Branch Address
        Instruction class has two internal function :
            a) setValues : Initializes the internal variables OpCode, Operand Count, Operand Address (if any), Branch Address
            b) print : Prints the contents of the Instruction (Object)
*/
class inst
{
    public:
            int opCode;
            int oprCount;
             bool brnTest;
            long long int oprAdd[3];
            long long int brnAdd;
    // Function to use in this object
    void setValues(int opCode,int opCount,long long int brnAdd,long long int oprAdd[]);
    void print(void);
};

/*
        Instruction class has internal function :
            a) setValues : Initializes the internal variables OpCode, Operand Count, Operand Address (if any), Branch Address -> taking inputs in the same order
*/

void inst::setValues(int a,int b, long long int c, long long int opr[])
{
    opCode = a;
    oprCount = b;
    for (int i=0;i<oprCount;i++)
        oprAdd[i] = opr[i];
    brnAdd = c;
}

/*
        Instruction class has internal function :
            b) print : Prints the contents of the Instruction (Object)
*/

void inst::print(void)
{
    cout<<"\n \t\tInstruction Details:\n \t\t Opcode : "<<opCode;
    if(opCode==0)
        cout<<"-Load";
    else if (opCode ==1)
        cout<<"-Store";
    else if (opCode ==2)
        cout<<"-Test & Branch";
    else
        cout<<"-Other";
    cout<<" | Operand Count : "<<oprCount<<endl;
    cout<<"\t\t Address: Branch Address : "<<brnAdd<<endl;
    for (int i=0;i<oprCount;i++)
            cout<<"\t\t Operand "<<i+1<<" Address : "<<oprAdd[i]<<endl;
    //cout<<endl;
}

/*
2. virtual Program (Function) : Whenever the function is called it generates a random instruction with following values
        Random Instruction generated may have:
            a) Opcode : 0-3 (any value from 0-load, 1-store,2-Branch,3-Other)
            b) # Operands : 0-3 (any values 0-3 - at most 3 operands)
            c) Operand Address : ( if # Operands is non-zero -> we generate random 48 bits with Spatial Locality (last 12bits) as operands virtual address
            d) Branch Address : We generate random 48 bits address -> it is used to update PC if we have a branch instruction
*/

inst virtualProgram()
{
    inst add;
    int a,b;
    bool brnTest;
    long long int c,oprAdd[3];
    a = rand()%4;           //  Used to generate random number 0-3 - for OpCode
    b = rand()%4;           //  Used to generate random number 0-3 - for Number of Operands
    c = randomBits(48);     //  Used to generate 48 bit random address for Branch Address
    long long int P = randomBits(36);       //  Random 36bits for the Operands -> help in getting some spatial locality
    for(int i=0;i<b;i++)                    //  Loops for number of operands
        oprAdd[i] = /*randomBits(48);*/(P<<12)|randomBits(12); //  Randomize last 12 bits to get some spatial locality in operands
    brnTest = (rand()%1)? true : false;     //  Set the Branch test variable true or false
    add.setValues(a,b,c,oprAdd);            //  call the function setValues to assign values to variables of the object
    //add.print();
    return add;
}

#endif // VIRTUALPROGRAM_H_INCLUDED
