#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<dirent.h>
#include<string>
using namespace std;
bool exists_test(const string& name){
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
main(){
    int thiscomputer,computernum,testtime,i,j,trader,ipport,port,server = 1;
    char k[200],cip[30],ip[30]="127.0.0.1";
    string scp[1000],scp_R[1000],sip,s,mkdir,computers[20],files[20],nodes[3000],thisfile;
    FILE *p;

    printf("this computer's name:");
    scanf("%d",&thiscomputer);
    printf("this computer's ip(XXX.XXX.XXX.XXX):");
    scanf("%s",cip);
    printf("this computer's port(if this is not a docker, enter 22):");
    scanf(" %d",&port);
    ipport=2500;
    printf("trader number:");
    scanf("%d",&trader);
    printf("Is there server?(No enter 0,Yes enter 1):");
    scanf("%d",&server);
    system("mkdir traderdir");
    if(server){
        system("mkdir traderdir/2500");
        printf("bitcoind -regtest -port=2500 -rpcport=12500 -rpcpassword=123 -datadir=traderdir/2500 -walletrejectlongchains -checkmempool=0 -daemon\n");
        system("bitcoind -regtest -port=2500 -rpcport=12500 -rpcpassword=123 -datadir=traderdir/2500 -walletrejectlongchains -checkmempool=0 -daemon");
	printf("How many computers have node:(include this computer)");
	scanf("%d",&computernum);
        if(trader) files[computernum+1]="tmp/"+to_string(thiscomputer);
        sip.assign(ip);
    }
    else{
        printf("enter the node's IP and port which you want to connect:(XXX.XXX.XXX.XXX XXXXX)");
        scanf("%s %d",ip,&ipport);
        sip.assign(ip);
        scp[0] = "sshpass -p asdf scp -P "+to_string(ipport)+" tmp/"+to_string(thiscomputer)+" "+sip+":~/Desktop/test/testtps/tmp/"+to_string(thiscomputer);
    }

    for(i=0;i<trader;i++){
        s = "bitcoind -regtest -daemon -blocksonly -walletbroadcast -checkmempool=0 -rpcpassword=123 -walletrejectlongchains -addnode="+sip+":2500 -port="+to_string(i+2501)+" -rpcport="+to_string(i+12501)+" -datadir=traderdir/"+to_string(i+2501);
        mkdir = "mkdir traderdir/"+to_string(i+2501);
        printf("mkdir: %s\n",mkdir.c_str());
        system(mkdir.c_str());
        printf("s: %s\n",s.c_str());
        system(s.c_str());
    }
    sleep(10);

    thisfile="tmp/"+to_string(thiscomputer);
    p=fopen(thisfile.c_str(),"w");
    fprintf(p,"%s %d\n",cip,port);
    fclose(p);
    for(i=0;i<trader;i++){  
        s = "bitcoin-cli -regtest -rpcport="+to_string(i+12501)+" -rpcpassword=123 getnewaddress 1 >> tmp/"+to_string(thiscomputer);
        printf("s: %s\n",s.c_str());
        system(s.c_str());
    }
    if(server==0) system(scp[0].c_str());

    if(server){
        int u,ok,filenum = 0;
        DIR* dir;
        struct dirent* sdir;
        printf("How many UTXO for a node(1~24):");
        scanf("%d",&u);
        for(i=0;i<3;i++){
            printf("bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 generate 1\n");
            system("bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 generate 1");
            sleep(3);
        }
        while(filenum!=computernum){
            dir=opendir("tmp");
            printf("wait for all other computer's node addresses\n");
            while((sdir=readdir(dir))!=NULL){
                ok=0;
                if(sdir->d_name[0]!='.'){
                    ok=1;
                    for(i=0;i<filenum;i++){
                        if(strcmp(&files[i][4],sdir->d_name)==0){
                            ok=0;
                            break;
                        }
                    }
                }
                if(ok){
                    string newfile(sdir->d_name);
                    files[filenum]="tmp/"+newfile;
                    filenum++;
                }
            }
            closedir(dir);
            sleep(1);
        }

        int nodenum = 0;
        for(i=0;i<computernum;i++){
            p = fopen(files[i].c_str(),"r");
            fscanf(p,"%s %d",cip,&port);
            sip.assign(cip);
            scp[i]="sshpass -p asdf scp -P "+to_string(port)+" tmp/all "+sip+":~/Desktop/test/testtps/tmp/all";
            scp_R[i]="ssh-keygen -R ["+sip+"]:"+to_string(port);
            while(fscanf(p,"%s",k)!=EOF){
                nodes[nodenum].assign(k);
                nodenum++;
            }
            fclose(p);
        }
/*        if(trader){
            p = fopen(files[computernum+1].c_str(),"r");
            while(fscanf(p,"%s",k)!=EOF){
                nodes[nodenum].assign(k);
                nodenum++;
            }
            fclose(p);
        }*///this computer's node
        p = fopen("tmp/tmpall","w");
        for(i=0;i<nodenum;i=i+(24/u)){
            for(j=0;(i+j<nodenum)&&(j<(24/u));j++){
	        fprintf(p,"%s\n",nodes[i+j].c_str());
                s = "bitcoin-cli -regtest -rpcpassword=123 -rpcport=12500 sendtoaddress "+nodes[i+j]+" 2";
                printf("%d times : %s\n",u,s.c_str());
                int ii;
                for(ii=0;ii<u;ii++){
                    system(s.c_str());
                }
            }
            printf("bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 generate 1\n");
            system("bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 generate 1"); 
            sleep(3);           
        }
        fclose(p);
        system("cp tmp/tmpall tmp/all");
        for(i=0;i<computernum;i++){
            system(scp[i].c_str());
            system(scp_R[i].c_str());
        }
    }
    
    while(!exists_test("tmp/all")){
        printf("wait for all node addresses\n");
        sleep(1);
    }
    for(i=0;i<trader;i++){
        s = "./trader.sh "+to_string(i+12501);
        printf("%s\n",s.c_str());
        pid_t pid;
        pid = fork();
        if(pid == 0){
            system(s.c_str());
            exit(0);
        }
    }
    if(server){
        printf("Is server the only miner and how long the block interval is:\n");
        int automine;
        scanf("%d",&automine);
        printf("How much time do you want to test(second)\n");
        scanf("%d",&testtime);
        if(automine){
            s = "./miner.sh 12500 "+to_string(automine)+" "+to_string(testtime/automine);
            printf("%s\n",s.c_str());
            pid_t pid;
            pid = fork();
            if(pid == 0){
                system(s.c_str());
                exit(0);
            }
        }
        i=0;
        while(testtime > 0){
            int c,t[3];
            t[0] = 11;
            testtime--;
            system("bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 getblockcount > tmp/blockcount.txt");
            p = fopen("tmp/blockcount.txt","r");
            fscanf(p,"%d",&c);
            string h="bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 getblock `bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 getblockhash "+to_string(c)+" ` > tmp/tmp.txt";
            system(h.c_str());
            
            h="bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 getblock `bitcoin-cli -regtest -rpcport=12500 -rpcpassword=123 getblockhash "+to_string(c-1)+" ` > tmp/tmp2.txt";
            system(h.c_str());
            
            p = fopen("tmp/tmp.txt","r");
            double tps = 0;
            char hh;
            while(t[0]){
                fscanf(p,"%c",&hh);
                if(hh=='"') tps++;
                else if(hh==':') t[0]--;
            }
            fscanf(p,"%d",&t[1]);
            fclose(p);
            tps = tps - 28;
            tps = tps/2;
            
            p = fopen("tmp/tmp2.txt","r");
            t[0] = 11;
            while(t[0]){
                fscanf(p,"%c",&hh);
                if(hh==':') t[0]--;
            }
            fscanf(p,"%d",&t[2]);
            fclose(p);
            
            t[0] = t[1] - t[2];
            tps = tps / t[0];
            p = fopen("result.txt","a");
            fprintf(p,"%lf\n",tps);
            fclose(p);
            sleep(1);
        }
    }
    printf("use ./clean.sh ( or ./docker_clean.sh ) to terminate bitcoind\n");
/*///////////////////////////////////////////////////////////
    printf("enter any number to terminate bitcoind:");
    int sudo;
    scanf("%d",&sudo);
    if(sudo){
        printf("sudo ./clean.sh\n");
        system("sudo ./clean.sh");
    }
    else{
         printf("./clean.sh\n");
         system("./clean.sh");
    }
*/////////////////////////////////////////////////////////////////
}
