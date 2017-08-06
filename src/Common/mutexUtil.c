#include "mutexUtil.h"

#ifdef __cplusplus
extern "C"{
#endif

int mutex_init(mutex_t* lock)
{
#ifdef WIN32
#if USE_CS
    InitializeCriticalSection(&lock->mutex);
#else
	static char mutexName[] = "topvdn_win32_";
	static int mutexCount = 0;
	char name[512] = {0};
	memset(name,0,512);
	sprintf(name,"%s%d",mutexName,mutexCount++);
	lock->mutex = CreateMutex(NULL, FALSE, name);
#endif
#else
	return pthread_mutex_init(&lock->mutex,NULL);
#endif
	return 0;
}

void mutex_lock(mutex_t* lock)
{
#ifdef WIN32
#if USE_CS
	EnterCriticalSection(&lock->mutex);
#else
	WaitForSingleObject(lock->mutex, INFINITE);
#endif
#else
    pthread_mutex_lock(&lock->mutex);
#endif
	return;
}

void mutex_unlock(mutex_t* lock)
{
#ifdef WIN32
#if USE_CS
	LeaveCriticalSection(&lock->mutex);
#else
	ReleaseMutex(lock->mutex);
#endif
#else
	pthread_mutex_unlock(&lock->mutex);		
#endif
	return;
}

void mutex_destroy(mutex_t* lock)
{
#ifdef WIN32
#if USE_CS
	DeleteCriticalSection(&lock->mutex);
#else
	CloseHandle(lock->mutex);
#endif
#else
	pthread_mutex_destroy(&lock->mutex);		
#endif
	return;
}


int semaphore_init(sem_t* s)
{
	int ret = 0;
#if defined(WIN32) || defined(_WIN32)
	s->sem = CreateSemaphore(NULL,0,1000,NULL);
	if(NULL == s->sem)
		return -1;
#else
	if (s->sem)
	{
		if (sem_init(*semaphore, 0, 0) != 0)
			return -1;
	}
#endif
	return 0;
}

void semaphore_destroy(sem_t* s,char* name)
{
#if defined(WIN32) || defined(_WIN32)
	CloseHandle(s->sem);
#elif defined(TOPVDN_IOS_SEMAPHORE)
	sem_close(s->sem);
	sem_unlink(name);
#else
	sem_destroy(s->sem);
#endif
}

int semaphore_wait(sem_t *s,int blockTime)
{
#if defined(WIN32) || defined(_WIN32)
	if (blockTime > 0)
		return WaitForSingleObject(s->sem, blockTime);
	else
		return WaitForSingleObject(s->sem,INFINITE);
//#elif defined(TOPVDN_IOS_SEMAPHORE)
#else
	if (blockTime > 0)
	{
		if (-1 == sem_wait(s->sem))
			return -1;
	}
	else
	{
		if (-1 == sem_trywait(q->sem) && errno != EAGAIN)
			return -1;
	}
	return 0;
#endif
}

int semaphore_post(sem_t *s)
{
#if defined(WIN32) || defined(_WIN32)
	return ReleaseSemaphore(s->sem,1,NULL)?0:-1;
#else
	return sem_post(s->sem);
#endif
}

thread_t create_thread(threadProc routine, void* args)
{
#ifdef WIN32
	thread_t thd = NULL;
	thd = (thread_t)_beginthread(routine, 0, args);
	//thd = (HANDLE)_beginthreadex(NULL,0,routine,args,0, NULL);//for __stdcall
	if (thd == -1L)
		printf("%s, _beginthread failed with %d\n", __FUNCTION__, errno);
	return thd;
#else
	pthread_t id = NULL;
	pthread_attr_t attributes;
	int ret;

	pthread_attr_init(&attributes);
	pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&id, &attributes, routine, args);
	if (ret != 0)
		printf("%s, pthread_create failed with %d\n", __FUNCTION__, ret);
	return id;
#endif
}

void destroy_thread(thread_t hnd)
{
#ifdef WIN32
#else
	pthread_join(hnd,NULL);
#endif
}

#ifdef __cplusplus
}
#endif