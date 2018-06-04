#include<iostream>
using namespace std;
int main()
{
	int array[100];
	int c,s=0;
	cin>>c;
	for(int i=0;i<c;i++) cin>>array[i];
	int tong[1000]={0};
	for(int i=0;i<c;i++) tong[array[i]]++;
	for(int i=0;i<1000;i++) if(tong[i]!=0) s++;
	cout<<s<<endl;
	for(int i=0;i<1000;i++) if(tong[i]!=0) cout<<i<<" ";
	return 0;
}
