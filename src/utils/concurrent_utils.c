/*
 * =============================================================================
 *   HSA Runtime Conformance Release License
 * =============================================================================
 * The University of Illinois/NCSA
 * Open Source License (NCSA)
 *
 * Copyright (c) 2014, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Developed by:
 *
 *                 AMD Research and AMD HSA Software Development
 *
 *                 Advanced Micro Devices, Inc.
 *
 *                 www.amd.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in
 *    the documentation and/or other materials provided with the distribution.
 *  - Neither the names of <Name of Development Group, Name of Institution>,
 *    nor the names of its contributors may be used to endorse or promote
 *    products derived from this Software without specific prior written
 *    permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include "concurrent_utils.h"

/**
 * @brief worker function is invoked by each thread to execute tests
 * Initially, all threads are blocked to wait run_flag. After run_flag being
 * set up, the worker function begin to execute test function and change
 * the status of tests to TEST_RUNNING. After test function finish, the status
 * of tests will be changed to TEST_FINISHED, and worker function will be
 * blocked until run_flag being set up again.
 * @param input Pointer to thread_aux data structure, which contains test
 * function pointer and corresponding args for the test function, and other
 * auxiliary information, including status of test, number of running tests,
 * run_flag, exit_flag, etc.
 */

static void *worker(void *input) {
    void (*fun_prt)(void *input);
    struct thread_aux* thread = (struct thread_aux*)input;
    fun_prt = thread->test->fun_prt;
    int run_flag_l = 0;
    // While loop to repeatedly execute test function
    while (1) {
        pthread_mutex_lock(thread->test_mutex);
        // Blocked to wait run_flag or exit_flag being changed
        while (*thread->run_flag == run_flag_l && *thread->exit_flag == 0) {
            pthread_cond_wait(thread->test_cond, thread->test_mutex);
        }
        pthread_mutex_unlock(thread->test_mutex);

        // Reset run_flag
        run_flag_l = run_flag_l ^ 1;

        // If exit_flag is 0, run test function and set status of the test to
        // TEST_RUNNING
        if (*thread->exit_flag == 0) {
            thread->test->status = TEST_RUNNING;
            fun_prt(thread->test->data);

            // After test function finish, subtract the number of running tests via atomic operations
            // and check the number of running tests, if the number equal to 1,
            // it means all tests are finished, broadcast a signal to the wakeup master
            // thread.
            pthread_mutex_lock(thread->test_mutex);
            (*(thread->num_running_t))--;
            if ((*thread->num_running_t) == 0) {
                pthread_cond_broadcast(thread->test_cond);
            }
            pthread_mutex_unlock(thread->test_mutex);

            // Set status of the test to TEST_STOP
            thread->test->status = TEST_STOP;
        } else {
            // If exit_flag is no-zero, set status of the test to TEST_FINISHED
            thread->test->status = TEST_FINISHED;
            pthread_exit(NULL);
        }
    }
    return NULL;
}

/**
 * @brief create a test_group data structure, initialize variables in
 * the test_group structure, allocate a test_list of group_size and
 * return a pointer to the test_group.
 * @param group_size The size of test group, i.e., the size of test lists
 * @return Pointer to the new test_group
 */
struct test_group *test_group_create(size_t group_size) {
    struct test_group *new_group = malloc(sizeof(struct test_group));
    // initialize variables in the data structure
    new_group->group_size = group_size;
    new_group->n_threads = 0;
    new_group->num_test = 0;
    new_group->run_flag = 0;
    new_group->exit_flag = 0;
    new_group->num_running_t = 0;
    // malloc test_list array with group_size
    new_group->test_list = (struct test_aux *)malloc(sizeof(struct test_aux) * group_size);

    return new_group;
}

void test_group_wait(struct test_group *t_group) {
    pthread_mutex_lock(&t_group->test_mutex);
    while (t_group->num_running_t != 0) {
        pthread_cond_wait(&t_group->test_cond, &t_group->test_mutex);
    }
    pthread_mutex_unlock(&t_group->test_mutex);

    return;
}

void test_group_add(struct test_group *t_group, void *fun_prt, void *data, size_t num_copy) {
    if (t_group->group_size < (num_copy + t_group->num_test)) {
        fprintf(stderr, "Error beyound group size: %lu, please resize the test_group\n", t_group->group_size);
        return;
    }

    int num_test = t_group->num_test;
    struct test_aux *test_list = t_group->test_list;
    int ii;
    for (ii = 0; ii < num_copy; ii++) {
        test_list[num_test + ii].fun_prt = fun_prt;
        test_list[num_test + ii].data = data;
        test_list[num_test + ii].status = TEST_NOT_STARTED;
    }
    t_group->num_test = num_test + num_copy;

    return;
}

void test_group_resize(struct test_group *t_group, size_t new_group_size) {
    if (new_group_size < t_group->group_size) {
        fprintf(stderr, "Error new group_size is smaller than current group_size\n");
    }
    struct test_aux *new_test_list;
    new_test_list = (struct test_aux *)realloc(t_group->test_list, new_group_size * sizeof(struct test_aux));
    t_group->group_size = new_group_size;
    t_group->test_list = new_test_list;

    return;
}

// Create threads for tests
void test_group_thread_create(struct test_group *t_group) {
    pthread_mutex_init(&(t_group->test_mutex), NULL);
    pthread_cond_init(&(t_group->test_cond), NULL);
    pthread_attr_init(&(t_group->attr));
    pthread_attr_setdetachstate(&(t_group->attr), PTHREAD_CREATE_JOINABLE);

    int n_threads;
    int ii = 0;

    n_threads = t_group->n_threads = t_group->num_test;
    struct thread_aux *thread_list = t_group->thread_list =
        (struct thread_aux *)malloc(sizeof(struct thread_aux) * n_threads);
    t_group->tid = (pthread_t*)malloc(sizeof(pthread_t) * n_threads);

    for (ii = 0; ii < n_threads; ++ii) {
        // CPU_ZERO(&thread_list[ii].cpuset);
        thread_list[ii].tid = ii;
        thread_list[ii].test = t_group->test_list + ii;
        thread_list[ii].run_flag = &(t_group->run_flag);
        thread_list[ii].exit_flag = &(t_group->exit_flag);
        thread_list[ii].test_mutex = &(t_group->test_mutex);
        thread_list[ii].test_cond = &(t_group->test_cond);
        thread_list[ii].num_running_t = &(t_group->num_running_t);
        int status = pthread_create(t_group->tid + ii, &(t_group->attr), worker, thread_list + ii);
        if (status < 0) {
            perror("pthread_create failed");
        }
    }

    return;
}

// Return number of test
int test_group_num_tests(struct test_group *t_group) {
    return t_group->num_test;
}

// Set affinity of the specific test
void test_group_thread_affinity(struct test_group *t_group, int test_id, int cpu_id) {
/*  Setting CPU affinity isn't currently supported.
 *  CPU_SET(cpu_id, &t_group->thread_list[test_id].cpuset);
 *  int status;
 *  status = pthread_setaffinity_np(t_group->tid[test_id],
 *          sizeof(cpu_set_t), &t_group->thread_list[test_id].cpuset);
 *  if (status != 0) {
 *      perror("pthread_setaffinity_np error");
 *  }
 */
    return;
}

// Set run_flag to 1
void test_group_start(struct test_group *t_group) {
    int ii;
    if (t_group->num_running_t != 0) {
        printf("Error: %d tests are not finished\n", t_group->num_running_t);
        return;
    }

    pthread_mutex_lock(&t_group->test_mutex);
    t_group->run_flag = t_group->run_flag ^ 1;
    t_group->num_running_t = t_group->num_test;
    pthread_cond_broadcast(&t_group->test_cond);
    pthread_mutex_unlock(&t_group->test_mutex);

    return;
}

// Set exit_flag to 1, wait all threads finish and cleanup
void test_group_exit(struct test_group *t_group) {
    int ii = 0;
    int status;

    pthread_mutex_lock(&t_group->test_mutex);
    t_group->exit_flag = 1;
    pthread_cond_broadcast(&t_group->test_cond);
    pthread_mutex_unlock(&t_group->test_mutex);

    for (ii = 0; ii < t_group->n_threads; ++ii) {
        status = pthread_join(t_group->tid[ii], 0);
        if (status < 0) {
            perror("pthread_join failed");
            t_group->test_list[ii].status = TEST_ERROR;
        }
    }

    pthread_attr_destroy(&(t_group->attr));
    pthread_mutex_destroy(&(t_group->test_mutex));
    pthread_cond_destroy(&(t_group->test_cond));

    free(t_group->tid);
    free(t_group->thread_list);

    return;
}

void test_group_kill(struct test_group *t_group) {
    int ii = 0;
    int status;
    for (ii = 0; ii < t_group->n_threads; ++ii) {
        status = pthread_cancel(t_group->tid[ii]);
        if (status < 0) {
            perror("pthread_cancel failed");
            t_group->test_list[ii].status = TEST_ERROR;
        }
    }

    pthread_attr_destroy(&(t_group->attr));
    pthread_mutex_destroy(&(t_group->test_mutex));
    pthread_cond_destroy(&(t_group->test_cond));

    free(t_group->tid);
    free(t_group->thread_list);

    return;
}

void test_group_destroy(struct test_group *t_group) {
    free(t_group->test_list);
    free(t_group);

    return;
}

int test_group_test_status(struct test_group *t_group, int test_id) {
    if (test_id >= t_group->n_threads) {
        fprintf(stderr, "test_id: %d is larger than the number of test: %d\n", test_id, t_group->num_test);
    }

    if (t_group->test_list[test_id].status == TEST_RUNNING) {
        if (pthread_kill(t_group->tid[test_id], 0) == ESRCH) {
            t_group->test_list[test_id].status = TEST_ERROR;
        }
    }

    return t_group->test_list[test_id].status;
}
