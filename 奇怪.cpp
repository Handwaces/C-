#include<iostream>
#define array_probe 10
using namespace std;
void swap(int &a,int &b)
{
	int _tmp=a;
	a=b;
	b=_tmp;
}
int main()
{
	int a[10]={9,8,7,6,5,4,3,2,1};
	for(int i=0;i<array_probe;i++)
	{
		for(int j=i-1;j<array_probe-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);
	}
}
