//
// Created by devel on 19/05/28.
//

#ifndef PTHREADEX_QUEUE_H
#define PTHREADEX_QUEUE_H

// 構造体: queue_t
typedef struct queue {
    unsigned int    in;         // 次に入れるインデックス
    unsigned int    out;        // 次に出すインデックス
    unsigned int    size;       // キューのサイズ
    unsigned int    length;     // ** キューに格納されている要素数
    pthread_mutex_t mutex;
    pthread_cond_t  not_full;   // ** キューが満タンじゃないという条件(cond)
    pthread_cond_t  not_empty;  // ** キューが空じゃないという条件(cond)
    void*           buffer[1];  // バッファ
    // ここ以下のメモリは p_queue->buffer[3] などとして参照される
} queue_t;

queue_t* create_queue (size_t size);
void destroy_queue (queue_t* p_queue);
void enqueue (queue_t* p_queue, void* data);
void* dequeue (queue_t* p_queue);


#endif //PTHREADEX_QUEUE_H
