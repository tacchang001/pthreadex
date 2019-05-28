#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <PCUnit/PCUnit.h>

#include "pcunit_unittests.h"

#include "ele_mempool.h"
#include "ele_error.h"

static unsigned int seed = 1;

static void test_example01(void)
{
	const size_t page_size = 0x200;
	const size_t page_num = 10;
	const size_t max_size = page_size * page_num;

	ele_result_t result = ele_mempool_create(max_size);
	PCU_ASSERT_TRUE(result == ELE_SUCCESS);

	int j;
	for (j=0; j<30; j++) {
		printf("--- %d ---\n", j);
//		mempool_print_pool();
		int i;
		void * m[page_num];
		for (i=0; i<page_num; i++) {
			printf("%d ", i);
			m[i] = ele_mempool_calloc(page_size);
			PCU_ASSERT_PTR_NOT_NULL(m[i]);
		}
		putchar('\n');
//		for (i=0; i<page_num; i++) printf("%d: %p\n", i, m[i]);
//		mempool_print_pool();
		for (i=0; i<page_num; i++) {
			printf("%d ", i);
			ele_mempool_free(m[i]);
		}
		putchar('\n');
	}

	ele_mempool_destroy();
}

static void test_example02(void)
{
	const size_t page_size = 0x200;
	const size_t page_num = 10;
	const size_t max_size = page_size * page_num;
	ele_result_t result =	ele_mempool_create(max_size);
	PCU_ASSERT_TRUE(result == ELE_SUCCESS);

	void * m[page_num];
	int k;
	for (k=0; k<page_num-1; k++) {
		m[k] = ele_mempool_calloc(page_size);
		PCU_ASSERT_PTR_NOT_NULL(m[k]);
	}
	for (k=0; k<page_num/2; k++) {
		ele_mempool_free(m[k]);
	}
	int j;
	for (j=0; j<10; j++) {
		printf("--- %d ---\n", j);
		mempool_print_pool();
		int i;
		for (i=0; i<page_num/2; i++) {
			printf("%d ", i);
			m[i] = ele_mempool_calloc(page_size);
			PCU_ASSERT_PTR_NOT_NULL(m[i]);
		}
		putchar('\n');
		for (i=0; i<page_num/2; i++) {
			printf("%d ", i);
			ele_mempool_free(m[i]);
		}
		putchar('\n');
	}

	ele_mempool_destroy();
}

#define LOOP_SIZE 8
#define PAGE_SIZE 1024
#define CELL_SIZE (PAGE_SIZE / LOOP_SIZE)

static void * test_pool_user_thread(void * arg) {
	const int loop = LOOP_SIZE;
	void * ptr[loop];
	int i;
	for (i=0; i<loop; i++) {
		ptr[i] = ele_mempool_calloc(CELL_SIZE);
		PCU_ASSERT_PTR_NOT_NULL(ptr[i]);
		int waittm = random();
		waittm %= 1000;
		usleep(waittm * 10);
	}
	for (i=0; i<loop; i++) {
		int waittm = random();
		waittm %= 1000;
		usleep(waittm * 10);
		ele_mempool_free(ptr[i]);
	}
	return arg;
}

static void test_example03(void)
{
	const int THREAD_NUM = 20;
	pthread_t id[THREAD_NUM];
	size_t max_size = PAGE_SIZE * THREAD_NUM;

	ele_result_t result = ele_mempool_create(max_size);
	PCU_ASSERT_TRUE(result == ELE_SUCCESS);

	int i;
    for (i=0; i<THREAD_NUM; i++) {
    	if (pthread_create(&id[i], NULL, test_pool_user_thread, &i) != 0) {
    		perror("pthread_create");
    		PCU_ASSERT_TRUE(false);
    	}
    }
    for (i=0; i<THREAD_NUM; i++) {
    	pthread_join(id[i], NULL);
    	printf("%d ", i);
    }
    putchar('\n');

	ele_mempool_destroy();

	PCU_ASSERT_TRUE(true);
}

PCU_Suite *MemoryPoolTest_suite(void)
{
	rand_r(&seed);

	static PCU_Test tests[] = {
		{ "memory pool 01", test_example01 },
		{ "memory pool 02", test_example02 },
		{ "memory pool 03", test_example03 },
	};
	static PCU_Suite suite = {
		"Memory Pool UnitTest", tests, sizeof tests / sizeof tests[0] };
	return &suite;
}
