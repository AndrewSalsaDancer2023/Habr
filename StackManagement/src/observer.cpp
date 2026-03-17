#include "observer.h"

void Publisher::subscribeEvents(std::shared_ptr<AbstractSubscriber> subscriber)
{
    subscriptions.push_back(subscriber);
}

void Publisher::notifyEventWaiting()
{
    for(const auto& weakSub : subscriptions)
        if (auto strongSub = weakSub.lock())
        {
            strongSub->onCoroWaiting();
            strongSub->onCoroCompleted();
        }
}

void Publisher::notifyEventCompleted()
{
    for(const auto& weakSub : subscriptions)
        if (auto strongSub = weakSub.lock())
            strongSub->onCoroCompleted();
}