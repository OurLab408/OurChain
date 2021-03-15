#include<stdio.h>
#include<string.h>
bool IsNum(char *d){
	int i,a=strlen(d);
	for(i=0;i<a;i++){
		if(d[i]!='-'&&d[i]!='0'&&d[i]!='1'&&d[i]!='2'&&d[i]!='3'&&d[i]!='4'&&d[i]!='5'&&d[i]!='6'&&d[i]!='7'&&d[i]!='8'&&d[i]!='9') return false;
	}
	return true;
}
bool ScanNum(int *c){
	char s[100];
	if(scanf("%s",s)==EOF) return false;
	while(!IsNum(s)){
		if(scanf("%s",s)==EOF) return false;
	}
	sscanf(s,"%d",c);
	return true;
}
main(){
	int g = 0;
	while(1){
		fprintf(stderr,"work %d\n",++g);
		int a,b,c,d,i[2],j[4],max;
		max=0; a=0; b=0; d=0; j[0]=0; j[1]=0; j[2]=0; j[3]=0; i[0]=0; i[1]=0;
		while(ScanNum(&c)){
			if(c==-1){
				d=1;
				break;
			}
			if(c==0){
				j[0]++;
				ScanNum(&c);
				if(c==-1){
					d=1;
					ScanNum(&c);
				}
				j[1]=j[1]+c;
				ScanNum(&c);
				if(c==-1){
					d=1;
					ScanNum(&c);
				}
				j[2]=j[2]+c;
				ScanNum(&c);
				if(c==-1){
					d=1;
					ScanNum(&c);
				}
				j[3]=j[3]+c;
			}
			i[0]++;
			i[1]=i[1]+c;
			b=b+c;
			a++;
			if(d==1) break;
		}
		if(j[0]==0) continue;
		printf("total        time %7d microseconds, average %4d microseconds, %d times\ntransaction total %7d microseconds, average %4d microseconds, %d times\ncheckmempol total %7d microseconds, average %4d microseconds\ncheckinputs total %7d microseconds, average %4d microseconds\notherproces total %7d microseconds, average %4d microseconds\n",b,b/a,a,j[3],j[3]/j[0],j[0],j[2],j[2]/j[0],j[1],j[1]/j[0],j[3]-j[2]-j[1],(j[3]-j[2]-j[1])/j[0]);
		printf("\n");

		//for csv
		/*
		printf("%d,%d,%d,%d,%d\n",j[0],j[3]/j[0],j[2]/j[0],j[1]/j[0],(j[3]-j[2]-j[1])/j[0]);
		*/
		if(d==0) break;
		d=0;
	}
}
