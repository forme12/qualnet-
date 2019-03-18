#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <vector>
#include <queue>
#include <stack>


struct MessageCompare : std::binary_function<Message*, Message*, bool>
{
    bool operator()(const Message *m1, const Message *m2) const
    {
        return (m1->timerExpiresAt > m2->timerExpiresAt);
    }
};

class TimerManager 
{
public:
    TimerManager(Node *node);
    ~TimerManager();
    void schedule(Message *msg, clocktype delay);
    void cancel(Message *msg);
    void scheduleNextTimer();

protected:
    Node *node;
    Message *currentMessageScheduled;
    std::priority_queue<Message*, std::vector<Message *>, MessageCompare> *localTimerHeap;
    std::stack<Message*> *scheduledTimerStack;

};


#endif
