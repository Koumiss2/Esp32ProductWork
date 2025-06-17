#pragma once

class Task {
public:
    virtual ~Task() = default;

    virtual void run(void* pvParameters) = 0;
};
