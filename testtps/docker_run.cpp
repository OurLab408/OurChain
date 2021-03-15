#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<string>
#include<sys/wait.h>
#include<sys/types.h>
using namespace std;
main(){
    int ip[4];
    int i,t;
    printf("What is your ip(XXX.XXX.XXX.XXX):");
    scanf("%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
    string sip = to_string(ip[0])+"."+to_string(ip[1])+"."+to_string(ip[2])+"."+to_string(ip[3]);
    printf("Have you build docker?(1 is yes, 0 is no):");
    scanf("%d",&i);
    if(i==0) system("sudo docker build -t ourchain . --no-cache");
    printf("How many docker do you want to be a trader:");
    scanf("%d",&t);
    for(i=0;i<t;i++){
        pid_t pid=fork();
        if(pid==0){
            string s,name;
            name = to_string(i+40001);
            s = "sudo docker run --name="+name+" -p "+sip+":"+name+":22 -d ourchain";
            system(s.c_str());
            s = "sudo docker exec "+name+" bash -c \"echo \\\""+to_string(ip[2]+ip[3])+name+" "+sip+" "+name+" 1 0 192 168 0 5 2500\\\" > /root/Desktop/test/testtps/input.txt\"";
            printf("%s\n",s.c_str());
            system(s.c_str());
            s = "sudo docker exec "+name+" bash -c \"cd /root/Desktop/test/testtps && ./tpstest < input.txt\"";
            system(s.c_str());
            exit(0);
        }
    }
    int ex;
    wait(&ex);
    system("sudo docker stop $(sudo docker ps -q)");
    system("sudo docker rm $(sudo docker ps -q -a)");
    exit(ex);
}
