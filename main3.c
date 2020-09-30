#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

ucontext_t uctx_main, uctx_func1, uctx_func2;

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)
   
#define CONTEXT_STACK (1024*64) // 64kB
typedef void (*context_func)(void);   

void func1(void);
void func2(void);

int ctx_create(ucontext_t 	*ctx,
				context_func func,
				ucontext_t 	*ctx_link,
				void 	*ss_sp,
				size_t 	ss_size);


int main(int argc, char *argv[])
{
   if(ctx_create(&uctx_func1,func1,&uctx_main,
				malloc(CONTEXT_STACK),CONTEXT_STACK) == 1)
		return EXIT_FAILURE;	 	 

		 		
	if(ctx_create(&uctx_func2,func2, 
				(argc > 1) ?
 NULL : &uctx_func1 ,   //&uctx_func1
				malloc(CONTEXT_STACK),CONTEXT_STACK) == 1)
	{
		free( uctx_func1.uc_stack.ss_sp );
		return EXIT_FAILURE;
	}
	 
	printf("main: swapcontext(&uctx_main, &uctx_func2)\n");	
	  
	if (swapcontext(&uctx_main, &uctx_func2) == -1) 
               handle_error("swapcontext");					

	free( uctx_func1.uc_stack.ss_sp );
	free( uctx_func2.uc_stack.ss_sp );
	
   printf("main: exiting\n");
   exit(EXIT_SUCCESS);
}


int ctx_create(ucontext_t 	*ctx,
				context_func func,
				ucontext_t 	*ctx_link,
				void 	*ss_sp,
				size_t 	ss_size)
{
	if(getcontext(ctx) == -1)
	{	 
		handle_error("getcontext");
		return 1;
	} 	   
    ctx->uc_link = ctx_link;
    ctx->uc_stack.ss_sp =  ss_sp; 
    ctx->uc_stack.ss_size = ss_size;
    ctx->uc_stack.ss_flags = 0;
	
	makecontext(ctx, func, 0); 
    return 0;
}


void func1(void)
{
   printf("func1: started\n");
   printf("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
   if (swapcontext(&uctx_func1, &uctx_func2) == -1)
	   handle_error("swapcontext");
   printf("func1: returning\n");
}

void func2(void)
{
   printf("func2: started\n");
   printf("func2: swapcontext(&uctx_func2, &uctx_func1)\n");
   if (swapcontext(&uctx_func2, &uctx_func1) == -1)
	   handle_error("swapcontext");
   printf("func2: returning\n");
}