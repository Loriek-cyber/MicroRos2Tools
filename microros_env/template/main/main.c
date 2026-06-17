#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include <uros_network_interfaces.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <sensor_msgs/msg/range.h>

#ifdef CONFIG_MICRO_ROS_ESP_NETIF_WIFI
#include "esp_netif.h"
#endif

// --- Configuration ---
#define TAG "UWB_SIM"
#define UWB_TOPIC "uwb_range"

// --- Global Variables ---
rcl_publisher_t publisher;
sensor_msgs__msg__Range msg;

// --- UWB Simulation Task ---
void uwb_simulator_task(void * arg) {
    float distance = 1.0;
    float direction = 0.05;

    while(1) {
        // Simple triangular wave simulation for distance
        distance += direction;
        if (distance > 10.0 || distance < 0.5) {
            direction *= -1;
        }

        // Add some "UWB noise"
        float noise = ((float)rand() / (float)RAND_MAX) * 0.05;
        msg.range = distance + noise;

        // ROS Header
        int64_t time = esp_timer_get_time();
        msg.header.stamp.sec = time / 1000000;
        msg.header.stamp.nanosec = (time % 1000000) * 1000;

        // Publish
        rcl_ret_t rc = rcl_publish(&publisher, &msg, NULL);
        if (rc != RCL_RET_OK) {
            ESP_LOGE(TAG, "Error publishing UWB data: %d", rc);
        } else {
            ESP_LOGI(TAG, "UWB Range: %.2f m", msg.range);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz
    }
}

void app_main(void) {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Networking (micro-ROS component helper)
    #if defined(CONFIG_MICRO_ROS_ESP_NETIF_WIFI) || defined(CONFIG_MICRO_ROS_ESP_NETIF_ETHERNET)
    ESP_ERROR_CHECK(uros_network_interface_initialize());
    #endif

    // 3. micro-ROS Setup
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    rcl_ret_t rc;

    // Wait for agent connection
    rc = rclc_support_init(&support, 0, NULL, &allocator);
    while (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "Failed to connect to agent, retrying...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        rc = rclc_support_init(&support, 0, NULL, &allocator);
    }

    // Create Node
    rcl_node_t node;
    rc = rclc_node_init_default(&node, "esp32_uwb_node", "", &support);

    // Create Publisher
    rc = rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
        UWB_TOPIC
    );

    // Initialize Message
    msg.header.frame_id.data = "uwb_link";
    msg.header.frame_id.size = strlen(msg.header.frame_id.data);
    msg.header.frame_id.capacity = msg.header.frame_id.size + 1;
    msg.radiation_type = sensor_msgs__msg__Range__ULTRASOUND; // Using ULTRASOUND as proxy for UWB
    msg.field_of_view = 3.14;
    msg.min_range = 0.1;
    msg.max_range = 50.0;

    // 4. Start UWB Simulator Task
    xTaskCreate(uwb_simulator_task, "uwb_task", 4096, NULL, 5, NULL);

    // 5. Initialize Bluetooth (Placeholder for efficiency)
    ESP_LOGI(TAG, "Bluetooth initialized (base).");

    // 6. Main Loop (Executor)
    rclc_executor_t executor;
    rc = rclc_executor_init(&executor, &support.context, 1, &allocator);
    
    while(1){
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Cleanup (never reached in this example)
    rc = rcl_publisher_fini(&publisher, &node);
    rc = rcl_node_fini(&node);
    rc = rclc_support_fini(&support);
}
