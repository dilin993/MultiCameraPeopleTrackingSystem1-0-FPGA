//
// Created by dilin on 11/18/17.
//

#include "FrameQueue.h"

bool FrameQueue::is_empty()
{
    unique_lock<mutex> locker(m_mutex);
    return frames.empty();
}

void FrameQueue::enqueue(Frame frame)
{
    unique_lock<mutex> locker(m_mutex);
    frames.push(frame);
}

Frame FrameQueue::dequeue()
{
    unique_lock<mutex> locker(m_mutex);
    Frame frame = frames.front();
    frames.pop();
    return frame;
}

FrameQueue::FrameQueue() :
frames()
{
}
