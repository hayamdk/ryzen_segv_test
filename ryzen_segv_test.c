#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/mman.h>

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

typedef uint32_t (*func_t)(func_set_t*, ...);

//func_set_t func_set;
func_set_t *func_set;
atomic_int flg;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef int (*printf_ptr_t)(const char *fmt, ...);
typedef int (*enter_func_t)(uint32_t);
typedef void (*leave_func_t)(int);

static void lock_enter()
{
	while( pthread_mutex_lock(&mutex) ) { }
}

static void lock_leave()
{
	while( pthread_mutex_unlock(&mutex) ) { }
}

static void serialize()
{
	//asm volatile("mfence":::"memory");
	
	unsigned int t[4];
	asm volatile ("cpuid"
			:"=a"(t[0]), "=b"(t[1]), "=c"(t[2]), "=d"(t[3])
			: "0" (0) );
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

void thread1(int64_t *loops)
{
	int64_t i;
	uint32_t ret1, ret2, t1, t2, should;
	func_t pf;

	for(i=0; i < *loops || (*loops < 0); i++) {
		lock_enter();
		serialize();
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

void thread2(int x)
{
	uint8_t offset;
	uint32_t randval;

	while(atomic_load(&flg)) {
		offset = random() % 256;
		randval = random();
		lock_enter();
		memset(func_set, 0, sizeof(func_set_t));
		memcpy(&func_set->func[offset], func_base, FUNC_BYTES);
		func_set->offset = offset;
		func_set->ret = randval;
		lock_leave();
	}
}

int main(int argc, const char *argv[])
{
	pthread_t t1, t2;
	int64_t loops;
	
	if(argc > 1) {
		loops = atoll(argv[1]);
	} else {
		loops = -1;
	}

	func_set = mmap (NULL, sizeof(func_set_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	atomic_store(&flg, 1);
	srandom(time(NULL));
	func_set->offset = random() % 256;
	func_set->ret = random();
	memcpy(&func_set->func[func_set->offset], func_base, FUNC_BYTES);
	memset(&func_set->dummy, 0x00, 64);
	pthread_create(&t1, NULL, (void*)thread1, &loops);
	pthread_create(&t2, NULL, (void*)thread2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

