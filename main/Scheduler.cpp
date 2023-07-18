//
// Created by alex2772 on 02.09.18.
//

#include "Scheduler.h"
#include "scheduler.png.h"
#include "Window.h"
#include "CCOSCore.h"
#include "SpinnerView.h"
#include "TaskHelper.h"

Bitmap Scheduler::getIcon() {
    return {scheduler, 30, 30};
}

const std::string Scheduler::getTitle() {
    return "Scheduler";
}

class SchedulerWindow: public Window {
public:
    TaskHelper* mTask;
    SchedulerWindow() : Window("Scheduler") {
        addView(new SpinnerView(60, 28));
        mTask = new TaskHelper([]() {
        });
    }
    ~SchedulerWindow() {
        delete mTask;
    }

};


void Scheduler::launch() {
    CCOSCore::displayWindow(new SchedulerWindow);
}
