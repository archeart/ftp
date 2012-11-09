#define MAX_ERR 10

#define kicked {put_err(2);exit(1);}
#define plost {put_err(8);close(fd);thread_end(args);}

void error_init();
void inline put_err(char);
