#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#ifdef _MSC_VER

#include <Windows.h>
#include <intrin.h>
#include <process.h>

//Note: Normally, volatile qualifier is not suitable for ensuring multithread atomicity,
//       but below definition works properly specially in x86 MSVC.
//See: https://msdn.microsoft.com/library/12a04hfd.aspx#Anchor_4
//See: https://msdn.microsoft.com/library/1s26w950.aspx

typedef volatile long atomic_int;
typedef int pid_t;
typedef HANDLE pthread_t;
#define getpid	my_getpid
#define MAP_FAILED (NULL)

static int atomic_exchange(atomic_int *ptr, int val)
{
	return (int)_InterlockedExchange(ptr, (long)val);
}

static void atomic_store(atomic_int *ptr, int val)
{
	*ptr = (long)val;
}

static int atomic_load(atomic_int *ptr)
{
	return (int)*ptr;
}

static pid_t my_getpid()
{
	return _getpid();
}

int pthread_create(pthread_t *thread, void *dummy, LPTHREAD_START_ROUTINE func, void *param)
{
	DWORD tid;
	HANDLE h;
	h = CreateThread(NULL, 0, func, param, 0, &tid);
	if (h == NULL) {
		return 0;
	}
	*thread = h;
	return 1;
}

int pthread_join(pthread_t thread, void **retval)
{
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	return 0;
}

#else

#include <stdatomic.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#endif

/*
#include <sys/syscall.h>

pid_t gettid(void)
{
    return syscall(SYS_gettid);
}
*/

#define FUNC_BYTES (256-5)

typedef struct {
	uint8_t dummy[64];
	uint8_t func[FUNC_BYTES+256];
	uint8_t offset;
	uint32_t ret;
} func_set_t;

#define RAND_STEP(y) do { \
		(y) = (y) ^ ((y) << 13); (y) = (y) ^ ((y) >> 17); \
		(y) = (y) ^ ((y) << 5); \
} while(0)

#ifdef _MSC_VER

uint32_t randseed;
__declspec(thread) int randinited = 0;
__declspec(thread) uint32_t randval;

void srandom(uint32_t seed)
{
	randseed = seed;
}

uint32_t random()
{
	if (!randinited) {
		randinited = 1;
		randval = randseed;
	}
	RAND_STEP(randval);
	return randval;
}

#endif

typedef uint32_t (*func_t)(func_set_t*);

//func_set_t func_set;
func_set_t *func_set;
atomic_int flg;
atomic_int locked;

typedef int (*printf_ptr_t)(const char *fmt, ...);
typedef int (*enter_func_t)(uint32_t);
typedef void (*leave_func_t)(int);

static void lock_enter()
{
	while( atomic_exchange(&locked,1) ) { }
}

static void lock_leave()
{
	int ret = atomic_exchange(&locked,0);
	if(!ret) {
		/* should never happed */
		fprintf(stderr, "lock inconsistency!!\n");
	}
}

static void mfence()
{
#ifdef _MSC_VER
	_mm_mfence();
#else
	asm volatile("mfence":::"memory");
#endif
}

static void serialize()
{
#ifdef _MSC_VER
	int t[4];
	__cpuid(t, 0);
#else
	unsigned int t[4];
	asm volatile ("cpuid"
			:"=a"(t[0]), "=b"(t[1]), "=c"(t[2]), "=d"(t[3])
			: "0" (0) );
#endif
}

/*
uint32_t func_base(func_set_t *p)
{
	uint32_t t1 = p->ret, t2 = 12345;
	RAND_STEP(t1);
	RAND_STEP(t2);
	if(t1 < t2) {
		t1 ^= t2;
	} else {
		RAND_STEP(t2);
		return (t1 + t2);
	}
	RAND_STEP(t1);
	return t1;
}
*/

#ifdef _MSC_VER

/* compiled x86_64 binary of above func_base (for MSVC) */
uint8_t func_base[FUNC_BYTES] = {
	/*1060*/ 0x8b, 0x91, 0x3c, 0x02, 0x00, 0x00, //mov    0x23c(%rcx),%edx
	/*1066*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*1068*/ 0xc1, 0xe0, 0x0d,                   //shl    $0xd,%eax
	/*106b*/ 0x33, 0xd0,                         //xor    %eax,%edx
	/*106d*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*106f*/ 0xc1, 0xe8, 0x11,                   //shr    $0x11,%eax
	/*1072*/ 0x33, 0xd0,                         //xor    %eax,%edx
	/*1074*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*1076*/ 0xc1, 0xe0, 0x05,                   //shl    $0x5,%eax
	/*1079*/ 0x33, 0xd0,                         //xor    %eax,%edx
	/*107b*/ 0x81, 0xfa, 0x7a, 0x74, 0xe5, 0xc6, //cmp    $0xc6e5747a,%edx
	/*1081*/ 0x73, 0x1c,                         //jae    109f
	/*1083*/ 0x81, 0xf2, 0x7a, 0x74, 0xe5, 0xc6, //xor    $0xc6e5747a,%edx
	/*1089*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*108b*/ 0xc1, 0xe0, 0x0d,                   //shl    $0xd,%eax
	/*108e*/ 0x33, 0xd0,                         //xor    %eax,%edx
	/*1090*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*1092*/ 0xc1, 0xe8, 0x11,                   //shr    $0x11,%eax
	/*1095*/ 0x33, 0xd0,                         //xor    %eax,%edx
	/*1097*/ 0x8b, 0xc2,                         //mov    %edx,%eax
	/*1099*/ 0xc1, 0xe0, 0x05,                   //shl    $0x5,%eax
	/*109c*/ 0x33, 0xc2,                         //xor    %edx,%eax
	/*109e*/ 0xc3,                               //retq
	/*109f*/ 0x8d, 0x82, 0xaf, 0x09, 0x2a, 0x65, //lea    0x652a09af(%rdx),%eax
	/*10a5*/ 0xc3,                               //retq
};

#else

/* compiled x86_64 binary of above func_base */
uint8_t func_base[FUNC_BYTES] = {
	/*ab0:*/ 0x8b, 0x97, 0x3c, 0x02, 0x00, 0x00, //mov    0x23c(%rdi),%edx
	/*ab6:*/ 0x89, 0xd0,                         //mov    %edx,%eax
	/*ab8:*/ 0xc1, 0xe0, 0x0d,                   //shl    $0xd,%eax
	/*abb:*/ 0x31, 0xc2,                         //xor    %eax,%edx
	/*abd:*/ 0x89, 0xd0,                         //mov    %edx,%eax
	/*abf:*/ 0xc1, 0xe8, 0x11,                   //shr    $0x11,%eax
	/*ac2:*/ 0x31, 0xc2,                         //xor    %eax,%edx
	/*ac4:*/ 0x89, 0xd0,                         //mov    %edx,%eax
	/*ac6:*/ 0xc1, 0xe0, 0x05,                   //shl    $0x5,%eax
	/*ac9:*/ 0x31, 0xc2,                         //xor    %eax,%edx
	/*acb:*/ 0x81, 0xfa, 0x79, 0x74, 0xe5, 0xc6, //cmp    $0xc6e57479,%edx
	/*ad1:*/ 0x8d, 0x82, 0xaf, 0x09, 0x2a, 0x65, //lea    0x652a09af(%rdx),%eax
	/*ad7:*/ 0x76, 0x07,                         //jbe    ae0 <func_base+0x30>
	/*ad9:*/ 0xf3, 0xc3,                         //repz retq
	/*adb:*/ 0x0f, 0x1f, 0x44, 0x00, 0x00,       //nopl   0x0(%rax,%rax,1)
	/*ae0:*/ 0x89, 0xd0,                         //mov    %edx,%eax
	/*ae2:*/ 0x35, 0x7a, 0x74, 0xe5, 0xc6,       //xor    $0xc6e5747a,%eax
	/*ae7:*/ 0x89, 0xc2,                         //mov    %eax,%edx
	/*ae9:*/ 0xc1, 0xe2, 0x0d,                   //shl    $0xd,%edx
	/*aec:*/ 0x31, 0xd0,                         //xor    %edx,%eax
	/*aee:*/ 0x89, 0xc2,                         //mov    %eax,%edx
	/*af0:*/ 0xc1, 0xea, 0x11,                   //shr    $0x11,%edx
	/*af3:*/ 0x31, 0xd0,                         //xor    %edx,%eax
	/*af5:*/ 0x89, 0xc2,                         //mov    %eax,%edx
	/*af7:*/ 0xc1, 0xe2, 0x05,                   //shl    $0x5,%edx
	/*afa:*/ 0x31, 0xd0,                         //xor    %edx,%eax
	/*afc:*/ 0xc3,                               //retq
	/*afd:*/ 0x0f, 0x1f, 0x00,                   //nopl   (%rax)
};

#endif


void thread1(int64_t *loops)
{
	int64_t i;
	uint32_t ret1, ret2, t1, t2, should;
	func_t pf;
	
	//usleep(1000);

	for(i=0; i < *loops || (*loops < 0); i++) {
		lock_enter();
		// You should confirm assembly of generated code, just in case the compiler reorders mfence/cpuid instruction.
		mfence();
		serialize();
		//volatile int t;
		//for(t=0; t<1000; t++) { }
		ret1 = func_set->ret;
		pf = (func_t)(&func_set->func[ func_set->offset ]);
		ret2 = pf(func_set);
		lock_leave();

		t1 = ret1, t2 = 12345;
		RAND_STEP(t1);
		RAND_STEP(t2);
		if(t1 < t2) {
			t1 ^= t2;
			RAND_STEP(t1);
			should = t1;
		} else {
			RAND_STEP(t2);
			should = (t1 + t2);
		}

		if(ret2 != should) {
			fprintf(stderr, "mismatch!!!!!!!!!!! %u %u\n", ret2, should);
		}
		//printf("%u\n", ret);
	}
	atomic_store(&flg, 0);
}

void threadx_core()
{
	uint8_t offset;
	uint32_t randval;

	offset = random() % 256;
	randval = random();
	memset(func_set, 0, sizeof(func_set_t));
	memcpy(&func_set->func[offset], func_base, FUNC_BYTES);
	func_set->offset = offset;
	func_set->ret = randval;
}

void threadx(void *p)
{
	uint8_t offset;
	uint32_t randval;
	
	//usleep(1000);

	while(atomic_load(&flg)) {
		offset = random() % 256;
		randval = random();
		lock_enter();
		// threadx_core();
		memset(func_set, 0, sizeof(func_set_t));
		memcpy(&func_set->func[offset], func_base, FUNC_BYTES);
		func_set->offset = offset;
		func_set->ret = randval;
		// You should confirm assembly of generated code, just in case the compiler reorders mfence instruction
		mfence(); // Assure that modified code is stored
		lock_leave();
	}
}

int n_cpus = 16;

int main(int argc, const char *argv[])
{
	int64_t loops;
	pthread_t t1, t2, t3;
#if !defined(_MSC_VER) && !defined(__FreeBSD__)
	cpu_set_t cpuset;
	int cpu;
#endif
	pid_t pid = getpid();
	
	if(argc > 1) {
		loops = atoll(argv[1]);
	} else {
		loops = -1;
	}

#ifdef _MSC_VER
	func_set = VirtualAlloc(NULL, sizeof(func_set_t), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
	n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	func_set = mmap (NULL, sizeof(func_set_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
	if(func_set == MAP_FAILED) {
		fprintf(stderr, "mmap returns MAP_FAILED!\n");
		exit (1);
	}

	atomic_store(&flg, 1);
	atomic_store(&locked, 1);
	
	srandom(time(NULL) + pid);
	// You should confirm assembly of generated code, just in case the compiler reorders mfence instruction
	threadx_core();
	mfence(); // Assure that flags are stored properly
	pthread_create(&t1, NULL, (void*)thread1, &loops);
	pthread_create(&t2, NULL, (void*)threadx, NULL);
	pthread_create(&t3, NULL, (void*)threadx, NULL);
	
#if !defined(_MSC_VER) && !defined(__FreeBSD__)
	cpu = random() % n_cpus;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	fprintf(stderr, "PID:%d CPU:%d\n", (int)pid, cpu);
#endif
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	return 0;
}

