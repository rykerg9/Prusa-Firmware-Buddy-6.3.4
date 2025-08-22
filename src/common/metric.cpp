#include "metric.h"
#include "cmsis_os.h"
#include "timing.h"
#include <logging/log.hpp>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <buddy/ccm_thread.hpp>
#include <cstring>
#include <atomic>

#include <buddy/priorities_config.h>
#include "metric_handlers.h"

static_assert(sizeof(metric_t) == 12);

extern metric_t __start_metric_definitions[]
#if __APPLE__
    __asm("section$start$__DATA$metric_definitions")
#endif
        ;

extern metric_t __end_metric_definitions[]
#if __APPLE__
    __asm("section$end$__DATA$metric_definitions")
#endif
        ;

static void metric_system_task_run(const void *);

// task definition
osThreadCCMDef(metric_system_task, metric_system_task_run, TASK_PRIORITY_METRIC_SYSTEM,
    0, 375);
static osThreadId metric_system_task;

// queue definition
#if PRINTER_IS_PRUSA_MINI()
static constexpr const size_t metric_system_queue_size = 50; ///< Not enough RAM, smaller buffer for metrics, sorry Mini
#else
static constexpr const size_t metric_system_queue_size = 100; ///< Size of metrics buffer
#endif /* PRINTER_IS_PRUSA_MINI() */
osMailQDef(metric_system_queue, metric_system_queue_size, metric_point_t);
static osMailQId metric_system_queue;

// internal variables
static std::atomic<bool> metric_system_initialized = false;
static std::atomic<uint16_t> dropped_points_count = 0;

// logging component
LOG_COMPONENT_DEF(Metrics, logging::Severity::info);

// internal metrics
METRIC_DEF(metric_dropped_points, "points_dropped", METRIC_VALUE_INTEGER, 1000, METRIC_ENABLED);

void metric_system_init() {
    if (metric_system_initialized) {
        return;
    }

    // first create mail queue, then thread, Note that we pass nullptr as thread_id to osMailCreate, but its unused so its fine.
    metric_system_queue = osMailCreate(osMailQ(metric_system_queue), nullptr);
    metric_system_task = osThreadCreate(osThread(metric_system_task), NULL);
    metric_system_initialized = true;
}

metric_t *metric_get_iterator_begin() {
    return __start_metric_definitions;
}

metric_t *metric_get_iterator_end() {
    return __end_metric_definitions;
}

static void metric_system_task_run(const void *) {
    for (;;) {
        osEvent event = osMailGet(metric_system_queue, osWaitForever);
        assert(event.status == osEventMail);
        metric_point_t *point = (metric_point_t *)event.value.p;
        metric_handler(point);
        osMailFree(metric_system_queue, point);
        metric_record_integer(&metric_dropped_points, dropped_points_count.load(std::memory_order::relaxed));
    }
}

static bool check_min_interval(metric_t *metric, uint32_t timestamp) {
    if (metric->min_interval_ms) {
        return ticks_diff(timestamp, metric->_last_update_timestamp) >= 1000 * (int32_t)metric->min_interval_ms;
    } else {
        return true;
    }
}

static metric_point_t *point_check_and_prepare(metric_t *metric, uint32_t timestamp, metric_value_type_t type) {
    if (!metric_system_initialized) {
        return NULL;
    }

    if (!check_min_interval(metric, timestamp)) {
        return NULL;
    }

    if (metric->type != type) {
        log_error(Metrics, "Attempt to record an invalid value type for metric %s", metric->name);
        metric_record_error(metric, "invalid type");
        return NULL;
    }

    if (!metric->enabled) {
        return NULL; // don't try to enqueue if nobody is listening
    }

    metric_point_t *point = (metric_point_t *)osMailAlloc(metric_system_queue, 0);
    if (!point) {
        dropped_points_count.fetch_add(1, std::memory_order::relaxed);
        return NULL;
    }

    point->metric = metric;
    point->error = false;
    point->timestamp = timestamp;
    return point;
}

static void point_enqueue(metric_point_t *recording) {
    metric_t *metric = recording->metric;
    if (osMailPut(metric_system_queue, (void *)recording) == osOK) {
        metric->_last_update_timestamp = ticks_us();
    } else {
        osMailFree(metric_system_queue, recording);
        dropped_points_count.fetch_add(1, std::memory_order::relaxed);
    }
}

void metric_record_float(metric_t *metric, float value) {
    metric_record_float_at_time(metric, ticks_us(), value);
}

void metric_record_float_at_time(metric_t *metric, uint32_t timestamp, float value) {
    metric_point_t *recording = point_check_and_prepare(metric, timestamp, METRIC_VALUE_FLOAT);
    if (!recording) {
        return;
    }
    recording->value_float = value;
    point_enqueue(recording);
}

void metric_record_string_at_time_v(metric_t *metric, uint32_t timestamp, const char *fmt, va_list args) {
    metric_point_t *recording = point_check_and_prepare(metric, timestamp, METRIC_VALUE_STRING);
    if (!recording) {
        return;
    }
    vsnprintf(recording->value_str, sizeof(recording->value_str), fmt, args);
    point_enqueue(recording);
}

void metric_record_string(metric_t *metric, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    metric_record_string_at_time_v(metric, ticks_us(), fmt, args);
    va_end(args);
}

void metric_record_string_at_time(metric_t *metric, uint32_t timestamp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    metric_record_string_at_time_v(metric, timestamp, fmt, args);
    va_end(args);
}

static void metric_record_custom_at_time_v(metric_t *metric, uint32_t timestamp, const char *fmt, va_list args) {
    metric_point_t *recording = point_check_and_prepare(metric, timestamp, METRIC_VALUE_CUSTOM);
    if (!recording) {
        return;
    }
    int length = vsnprintf(recording->value_custom, sizeof(recording->value_custom), fmt, args);
    if ((size_t)length >= sizeof(recording->value_custom)) {
        recording->error = true;
        strcpy(recording->error_msg, "value too long");
    }

    point_enqueue(recording);
}

void metric_record_custom(metric_t *metric, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    metric_record_custom_at_time_v(metric, ticks_us(), fmt, args);
    va_end(args);
}

void metric_record_custom_at_time(metric_t *metric, uint32_t timestamp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    metric_record_custom_at_time_v(metric, timestamp, fmt, args);
    va_end(args);
}

void metric_record_event(metric_t *metric) {
    metric_record_event_at_time(metric, ticks_us());
}

void metric_record_event_at_time(metric_t *metric, uint32_t timestamp) {
    metric_point_t *recording = point_check_and_prepare(metric, timestamp, METRIC_VALUE_EVENT);
    if (!recording) {
        return;
    }
    point_enqueue(recording);
}

void metric_record_integer(metric_t *metric, int value) {
    metric_record_integer_at_time(metric, ticks_us(), value);
}

void metric_record_integer_at_time(metric_t *metric, uint32_t timestamp, int value) {
    metric_point_t *recording = point_check_and_prepare(metric, timestamp, METRIC_VALUE_INTEGER);
    if (!recording) {
        return;
    }
    recording->value_int = value;
    point_enqueue(recording);
}

void metric_record_error(metric_t *metric, const char *fmt, ...) {
    // TODO: we might want separate throttling for errors
    metric_point_t *recording = point_check_and_prepare(metric, ticks_us(), metric->type);
    if (!recording) {
        return;
    }
    recording->error = true;
    va_list args;
    va_start(args, fmt);
    vsnprintf(recording->error_msg, sizeof(recording->error_msg), fmt, args);
    va_end(args);
    point_enqueue(recording);
}

bool metric_record_is_due(metric_t *metric) {
    return check_min_interval(metric, ticks_us());
}
