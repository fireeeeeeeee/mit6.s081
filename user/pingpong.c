#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
	int p1[2],p2[2];
	pipe(p1);
	pipe(p2);
	char buf[105];
	if(fork()==0)
	{
		close(p1[0]);
		close(p2[1]);
		
		int t;
		for(int i=0;i<10;i++)
		{
			read(p2[0],&t,1);
			//int id = getpid();
			printf("%d: received %s\n",t,buf);			
		}

		write(p1[1],"pong",4);
	}
	else
	{
		close(p1[1]);
		close(p2[0]);
		int id = getpid();
		for(int i=0;i<10;i++)
		{
			write(p2[1],&id,4);
		}
		
		read(p1[0],buf,8192);
		printf("%d: received %s\n",id,buf);
		
	}
 
	exit(0);
}
