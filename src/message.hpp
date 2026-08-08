#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

enum class Event {
    WallBounce,
    PaddleHit,
    Score,
    Win,
};

class MessageBus {
public:
    using Handler = std::function<void(Event)>;

    void subscribe(Event ev, Handler handler)
    {
        handlers_[static_cast<int>(ev)].push_back(std::move(handler));
    }

    void publish(Event ev) const
    {
        auto it = handlers_.find(static_cast<int>(ev));
        if (it != handlers_.end())
            for (const auto& h : it->second) h(ev);
    }

private:
    std::unordered_map<int, std::vector<Handler>> handlers_;
};
