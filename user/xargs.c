#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int
main(int argc, char *argv[])
{

	char buf[512];
	char l=0;
	
	char* a[10];
	for(int i=1;i<argc;i++)
	{
		a[i-1]=argv[i];
	}
	a[argc-1]=buf;
	while(read(0,buf+l,1))
	{
		if(*(buf+l)=='\n')
		{
			*(buf+l)=0;
			if(fork()==0)
			{
				exec(argv[1],a);
				exit(0);
			}
			wait(0);
			l=0;
			continue;
		}
		
		l++;
	}
	if(l!=0)
	{
		*(buf+l)=0;
		if(fork()==0)
		{
			exec(argv[1],a);
			exit(0);
		}
		wait(0);
	}

    exit(0);
}

