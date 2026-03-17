#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <map>

#include <memory>
#include <algorithm>

// // 1. Интерфейс подписчика (наблюдателя)
class AbstractSubscriber {
public:
    // virtual ~ISubscriber() = default;
    virtual void onCoroWaiting() = 0;
    virtual void onCoroCompleted() = 0;
};

// 2. Издатель (Publisher)
class Publisher {
public:
    void subscribeEvents(std::shared_ptr<AbstractSubscriber> subscriber);
    void notifyEventWaiting();
    void notifyEventCompleted();
private:
    // std::map<ObservationEvents, std::set<std::shared_ptr<AbstractSubscriber>>> subscriptions;
    std::vector<std::weak_ptr<AbstractSubscriber>> subscriptions;    
};
