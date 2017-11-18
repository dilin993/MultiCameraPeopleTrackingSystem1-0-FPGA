//
// Created by dilin on 11/18/17.
//

#ifndef SERVER_FRAMEQUEUE_H
#define SERVER_FRAMEQUEUE_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include "Frame.h"

using namespace std;

class FrameQueue
{
public:
    FrameQueue();
    void enqueue(Frame frame);
    Frame dequeue();
    bool is_empty();
private:
    queue<Frame> frames;
    mutex m_mutex;
};


#endif //SERVER_FRAMEQUEUE_H
