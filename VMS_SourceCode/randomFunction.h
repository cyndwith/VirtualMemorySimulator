#ifndef RANDOMFUNCTION_H_INCLUDED
#define RANDOMFUNCTION_H_INCLUDED
#include <iostream>
#include <ctime>
/*
"randomFunction.h" - contains the following functions : random64(), randomBits(int), convertToString(), bitMask()

1. random64()       -   generates a 64bit (long long int)
2. randomBits(int)  -   generates required number of variables (int - values given to the function
3. convertToString() -  used to prints the bits in the variables for debug
4. bitMask(start,stop) -  used to mask the bits with bits with index values [start-stop]

*/
using namespace std;
// random64() - Generates 64 bit random bits
long long int random64()
{
    //srand(time(0));
    long long int x,y,z,w;
    x = rand();
    y = rand();
    z = rand();
    w = rand();
    return (((x*y)<<32) | (z*w));
    //return x;
}

// Generates x number of random bits and adds zeros in the beginning as padding for 64 bit data type
long long int randomBits(int x)
{

    return (random64()>>(64-x));
}
// Used to print the bits string in the variables - converts data type into bits
string convertToBitString(long long int x,int nbits)
{
    string str(nbits, '0');
    for(int i = 0; i < nbits; i++)
    {
        if( (1ll << i) & x)
            str[nbits-1-i] = '1';
    }
    return str;
 }
// Used to mask the required bits from x to y (x>y) to get the requirement bits form group of bits
long long int bitMask(long long int A, int x,int y)
{
    unsigned long long int temp = 0x8000000000000000;
    unsigned long long int temp2 = (unsigned long long int )A;
    temp = ((temp2<<(64-x))>>(64-x+y-1));
    return temp;
}

// lruBlock function is used to select the LRU block
int lruBlock(int LRU[],int block,int bankSize)
{
    int temp=LRU[0];
    int bank = 0;
    for(int i=0;i<bankSize;i++)
    {
        if(temp<LRU[i])
        {
             temp = LRU[i];
             bank = i;
        }
    }
    return bank;
}

#endif // RANDOMFUNCTION_H_INCLUDED
