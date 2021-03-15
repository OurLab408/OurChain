#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string>
using namespace std;
bool exists_test(const string& name){
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
main(){
    int thiscomputer,miner = 0;
    int ip[5],server = 1;
    string scp,files,s,mkdir;
    FILE* p;
    ip[0]=127;
    ip[1]=0;
    ip[2]=0;
    ip[3]=1;
    ip[4]=2500;
    printf("this computer name:");
    scanf("%d",&thiscomputer);
    printf("miner number:");
    scanf("%d",&miner);
    
    printf("Is there server or trader?(Neither enter 0, server enter 1, only trader enter 2)");
    scanf("%d",&server);
    if(server == 0||server == 2){
        printf("enter the node's IP and port(IPv4) which you want to connect(enter 5 numbers separated by blanks):");
        scanf("%d %d %d %d %d",&ip[0],&ip[1],&ip[2],&ip[3],&ip[4]);
        scp="sshpass -p asdf scp tmp/"+to_string(thiscomputer)+" 192.168.0."+to_string(ip[3])+":~/Desktop/test/testtps/tmp/"+to_string(thiscomputer);
        files="tmp/"+to_string(thiscomputer);
    }
    int i,blockinterval;
    printf("blockinterval:");
    scanf("%d",&blockinterval);

    system("mkdir minerdir");
    for(i=0;i<miner;i++){
        s = "bitcoind -regtest -daemon -rpcpassword=123 -addnode="+to_string(ip[0])+"."+to_string(ip[1])+"."+to_string(ip[2])+"."+to_string(ip[3])+":"+to_string(ip[4])+" -port="+to_string(i+1501)+" -rpcport="+to_string(i+11501)+" -datadir=minerdir/"+to_string(i+1501);
        mkdir = "mkdir minerdir/"+to_string(i+1501);
        printf("mkdir: %s\n",mkdir.c_str());
        system(mkdir.c_str());
        printf("s: %s\n",s.c_str());
        system(s.c_str());
    }

    if(server==0){
        p = fopen(files.c_str(),"w");
        fclose(p);
        system(scp.c_str());
    }
    while(!exists_test("tmp/all")){
        printf("wait for all node addresses\n");
        sleep(1);
    }

    for(i=0;i<miner;i++){
        s="./miner.sh "+to_string(i+11501)+" "+to_string(blockinterval);
        printf("%s\n",s.c_str());
        pid_t pid;
        pid = fork();
        if(pid == 0){
           system(s.c_str());
           exit(0);
        }
        sleep(2);
    }

    int sudo;
    printf("enter any number to terminate bitcoind(in the docker, enter 0 please):");
    scanf("%d",&sudo);
    if(sudo){
    	printf("sudo pkill -9 -f miner.sh\n");
    	system("sudo pkill -9 -f miner.sh");
    }
    else{
    	printf("pkill -9 -f miner.sh\n");
    	system("pkill -9 -f miner.sh");
    }
    for(i=0;i<miner;i++){
        s="bitcoin-cli -regtest -rpcpassword=123 -rpcport="+to_string(i+11501)+" stop";
        printf("%s\n",s.c_str());
        system(s.c_str());
    }
} 
