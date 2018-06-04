#include<cstdio>
#include<iostream>
#include<cstring>
using namespace std;
int main()
{
	char s[1000];
	char str[10]="aaa";
	int c=0;
	while(1)
	{
		if(scanf("%s",s)==1) break;
		if(strcmp(s,str)==0) c++;
	}
	cout<<c<<endl;
}
