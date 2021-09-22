//
// Created by alex2772 on 21.04.18.
//

#include "TaskHelper.h"

TaskHelper::TaskHelper() {

}

void _task_helper(void* t) {
    TaskHelper* th = reinterpret_cast<TaskHelper*>(t);
    th->mCallable();
    while (1)
        vTaskDelay(portMAX_DELAY);
    vTaskDelete(0);
}

TaskHelper::TaskHelper(std::function<void()> callable): mCallable(callable) {
    char buf[32];
    static size_t counter = 0;
    sprintf(buf, "TaskHelper #%ud", counter++);
    xTaskCreate(_task_helper, buf, 8 * 2048, this, 1, &mTask);
}

TaskHelper::~TaskHelper() {
    if (mTask) {
        vTaskDelete(mTask);
    }
}
