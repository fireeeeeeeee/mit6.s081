#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



void subP(int pfa[])
{
	close(pfa[1]);
	int t;
	int firstNum=1;
	int firstPush=1;
	int pri;
	int pson[2];
	while(read(pfa[0],&t,1))
	{
		if(firstNum)
		{
			printf("prime %d\n",t);
			firstNum=0;
			pri=t;
			continue;
		}
		if(t%pri!=0)
		{
			if(firstPush)
			{
				pipe(pson);
				
				if(fork()==0)
				{
					subP(pson);
					return;
				}
				close(pson[0]);
				firstPush=0;
			}
			write(pson[1],&t,1);
		}
	}
	close(pson[1]);
	wait(0);
	exit(0);
}


int
main(int argc, char *argv[])
{	
	int pfirst[2];
	pipe(pfirst);
	
	if(fork()==0)
	{	
		subP(pfirst);
		exit(0);
	}
	else
	{
		close(pfirst[0]);
		for(int i=2;i<=35;i++)
		{
			write(pfirst[1],&i,1);
		}		
		close(pfirst[1]);
		wait(0);
	}

	 
	exit(0);
}
