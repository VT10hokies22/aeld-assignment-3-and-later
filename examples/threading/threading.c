#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    int rc1;

    /* Use cast to obtain data*/
    struct thread_data *param2 = (struct thread_data *) thread_param;

    /* Wait */
    rc1 = usleep((*param2).wait_to_obtain_ms);
    /*printf("1: %d\n",rc1);*/

    if (rc1 !=0){
        return thread_param;
    }

    /*Mutex lock*/
    rc1 = pthread_mutex_lock((*param2).mutex);
    /*printf("2: %d\n",rc1);*/
    if (rc1 !=0){
        return thread_param;
    }

    /* Sleep */
    rc1 = usleep((*param2).wait_to_release_ms);
    /*printf("3: %d\n",rc1);*/
    if (rc1 !=0){
        /* If sleep failed still need to unlock before returning*/
        pthread_mutex_unlock((*param2).mutex);
        return thread_param;
    }

    /* Mutex unlock */
    rc1 = pthread_mutex_unlock((*param2).mutex);
    if (rc1 !=0){
        /* If unlock failed try again, ideally do while loop until succeeds*/
        pthread_mutex_unlock((*param2).mutex);
        return thread_param;
    }
    
    /* Completed thread */
    (*param2).thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * 
     * 
     * See implementation details in threading.h file comment block
     */

    /* So need to wait */

    /* Need to create thread_data struct in pthread_create. thread_data is the thread_param in threadfunc
    
    */
    
    
    /* Malloc is a void pointer, so must be cast to datatype thread data*/
    /* So ptr param1 of type thread data = pointer of type thread data casted as malloc which returns void pointer */
    
    /* Allocated memory for thread data */
    struct thread_data *param1 = (struct thread_data *) malloc(sizeof(struct thread_data));
    
    /* Setup mutex and wait arguments */
    (*param1).wait_to_obtain_ms = wait_to_obtain_ms;
    (*param1).wait_to_release_ms = wait_to_release_ms;
    (*param1).mutex = mutex;
    (*param1).thread_complete_success = false;

    /* pass thread_data to created thread
     * using threadfunc() as entry point*/
    int rc = pthread_create(thread,NULL,threadfunc,param1);

    if ( rc != 0 ) {
        return false;
    }
    return true;
}
