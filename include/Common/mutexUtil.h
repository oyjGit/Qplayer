#ifndef __MUTEX_UTIL_H__
#define __MUTEX_UTIL_H__

#if defined(WIN32) || defined(_WIN32)
#include <process.h>
#include <WinSock2.h>
#include <windows.h>
#else 
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#endif

#ifdef WIN32
//Ê¹ÓÃÁÙ½çÇøËøÐ§ÂÊ¸ß£¬×ÊÔ´ÏûºÄÐ¡.
#define USE_CS 1
#endif

typedef struct basic_mutex {
#if WIN32
#if USE_CS
	CRITICAL_SECTION mutex;
#else
	HANDLE mutex;
#endif
#else // pthread mutex
	pthread_mutex_t mutex;
#endif
} mutex_t;

typedef struct basic_sem {
#ifdef WIN32
	HANDLE sem;
#else // pthread sem
	sem_t sem;
#endif
} sem_t;

#if 0
typedef struct basic_thread {
#ifdef WIN32
	HANDLE thread;
#else // pthread sem
	pthread_t thread;
#endif
} thread_t;
#endif

#ifdef WIN32
#define TFTYPE void
#define thread_t HANDLE
#else
#define TFTYPE void*
#define thread_t pthread_t
#endif

#ifdef __cplusplus
extern "C"{
#endif

typedef TFTYPE (threadProc)(void* arg);

int mutex_init(mutex_t* m);

void mutex_lock(mutex_t* m);

void mutex_unlock(mutex_t* m);

void mutex_destroy(mutex_t* m);

int semaphore_init(sem_t* s);

void semaphore_destroy(sem_t* s,char* name);

int semaphore_wait(sem_t *s, int blockTime);

int semaphore_post(sem_t *s);

thread_t create_thread(threadProc,void* arg);

void destroy_thread(thread_t hnd);


#ifdef __cplusplus
}
#endif

#endif