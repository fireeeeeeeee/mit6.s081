#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  if(argc < 2 ){
    fprintf(2, "Missing parameter sleep time \n");
    exit(1);
  }
  int sTime=atoi(argv[1]);
  sleep(sTime);
  exit(0);
}
