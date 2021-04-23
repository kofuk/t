#include <stdio.h>

#include <sys/time.h>

typedef struct {
    struct timeval tv;
} TimerState;

static TimerState global_timer_state = {0};

void timer_start(TimerState *state) {
    if (state == NULL) {
        state = &global_timer_state;
    }

    gettimeofday(&state->tv, NULL);
}

void timer_stop(TimerState *state) {
    if (state == NULL) {
        state = &global_timer_state;
    }

    struct timeval time;
    gettimeofday(&time, NULL);

    time_t s_diff = time.tv_sec - state->tv.tv_sec;
    int us_diff = (int)(time.tv_usec - state->tv.tv_usec);
    printf("%ldm%ld.%06ds\n", s_diff / 60, s_diff % 60, us_diff);
}


#include <unistd.h>

int main(void) {
    timer_start(NULL);
    sleep(3);
    timer_stop(NULL);
}
