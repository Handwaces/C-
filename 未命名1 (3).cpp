#include<cstdio>
#include<ctime>
#include<iostream>
#include<cmath>
using namespace std;
int a[303][196];
int func(int x,int y)
{
	return (int) (a[x][y]+a[x-1][y]+a[x+1][y]+a[x][y-1]+a[x][y+1])/5;
}
int main()
{
	freopen("in.txt","r",stdin);
	freopen("out.txt","w",stdout);
	for(int x=0;x<303;x++)
	{
		for(int y=0;y<196;y++)
		{
			scanf("%d",&a[x][y]);
		}
	}
	for(int x=1;x<302;x++)
	{
		for(int y=1;y<195;y++)
		{
			a[x][y]=func(x,y);
		}
	}
	for(int x=1;x<302;x++)
	{
		for(int y=1;y<195;y++)
		{
			printf("%d ",a[x][y]);
		}
	}
}
