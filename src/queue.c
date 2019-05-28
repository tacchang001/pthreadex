#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "queue.h"

// 関数名: create_queue
//  size で指定されたサイズのバッファを持つ queue_t を生成&初期化し、そのポインタを返す
//  destroy_queue で解放する必要がある
//  メモリの確保に失敗した場合は NULL を返す
queue_t* create_queue (size_t size) {
    queue_t* p_queue;
    int memsize = sizeof(queue_t) + (size - 1) * sizeof(void*);

    // メモリの確保
    // struct queue_t のサイズは buffer サイズとして 1 を含んでいるので 1 を引く
    p_queue = (queue_t*)malloc(memsize);

    if (p_queue != NULL) {

        // ** index の初期化
        p_queue->in = 0;
        p_queue->out = 0;

        // バッファのサイズ
        p_queue->size = size;

        // 要素数の初期化
        p_queue->length = 0;

        // pthread_mutex_t の初期化
        pthread_mutex_init(&p_queue->mutex, NULL);
        // ** 以下のように、定数を使った初期化をすることも出来る。
        // p_queue->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

        // pthread_cond_t の初期化
        pthread_cond_init(&p_queue->not_full, NULL);
        pthread_cond_init(&p_queue->not_empty, NULL);
        // ** 以下のように、定数を使った初期化をすることも出来る。
        // p_queue->not_full  = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
        // p_queue->not_empty = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    }

    return p_queue;
}

// 関数名: destroy_queue
//  create_queue で確保されたメモリを解放する
//  初期化されている mutex や cond も破棄する
void destroy_queue (queue_t* p_queue) {
    int rv;

    assert(p_queue != NULL);

    // pthread_mutex_t の破棄
    rv = pthread_mutex_destroy(&p_queue->mutex);
    assert(rv == 0);

    // pthread_mutex_t の破棄
    rv = pthread_cond_destroy(&p_queue->not_full);
    assert(rv == 0);
    rv = pthread_cond_destroy(&p_queue->not_empty);
    assert(rv == 0);

    // メモリの解放
    free(p_queue);
}

// 関数名: enqueue
//  キューにデータを入れる
//  データが満タンな場合は、ブロックします
void enqueue (queue_t* p_queue, void* data) {
    int rv;
    rv = pthread_mutex_lock(&p_queue->mutex);
    assert(rv == 0);
    // -- ここから、クリティカルセクション --

    // 満タンじゃなくなるまで待つ
    while (p_queue->length == p_queue->size) {
        rv = pthread_cond_wait(&p_queue->not_full, &p_queue->mutex);
        assert(rv == 0);
    }

    // データを入れる
    p_queue->buffer[p_queue->in] = data;

    // 次にデータを入れる場所をインクリメント
    p_queue->in++;
    p_queue->in %= p_queue->size;

    // 要素数の更新
    p_queue->length++;

    // -- ここまで、クリティカルセクション --
    pthread_mutex_unlock(&p_queue->mutex);
    pthread_cond_signal(&p_queue->not_empty);
}

// 関数名: dequeue
//  キューにデータを入れる
//  データが満タンな場合は、ブロックします
void* dequeue (queue_t* p_queue) {
    void* result = NULL;

    int rv;
    rv = pthread_mutex_lock(&p_queue->mutex);
    assert(rv == 0);
    // -- ここから、クリティカルセクション --

    // 空っぽじゃなくなるまで待つ
    while (!p_queue->length) {
        rv = pthread_cond_wait(&p_queue->not_empty, &p_queue->mutex);
        assert(rv == 0);
    }

    // データを取り出す
    result = p_queue->buffer[p_queue->out];

    // 次にデータを取り出す場所をインクリメント
    p_queue->out++;
    p_queue->out %= p_queue->size;

    // 要素数の更新
    p_queue->length--;

    // -- ここまで、クリティカルセクション --
    pthread_mutex_unlock(&p_queue->mutex);
    pthread_cond_signal(&p_queue->not_full);

    return result;
}

void inspect_queue(queue_t* p_queue) {
    int rv;
    rv = pthread_mutex_lock(&p_queue->mutex);
    assert(rv == 0);
    printf( "queue(%p) {\n"
            "   size:   %d;\n"
            "   length: %d;\n"
            "}\n\n", p_queue, p_queue->size, p_queue->length);
    pthread_mutex_unlock(&p_queue->mutex);
}
