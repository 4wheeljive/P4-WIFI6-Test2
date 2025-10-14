/// @file lcd_driver_i80_impl.h
/// @brief Implementation of ESP32 I80/LCD_CAM parallel LED driver template methods
///
/// NOTE: This file should only be included from lcd_driver_i80.h
///       Do not include this file directly in other translation units.

#pragma once

#include "esp_log.h"
#include "platforms/assert_defs.h"

#define LCD_TAG "FastLED_LCD"

namespace fl {

template <typename LED_CHIPSET>
void LcdI80Driver<LED_CHIPSET>::generateTemplates() {
    // 3-word encoding:
    // Bit-0: [HIGH, LOW, LOW]   (1 slot HIGH, 2 slots LOW)
    // Bit-1: [HIGH, HIGH, LOW]  (2 slots HIGH, 1 slot LOW)

    // All lanes transmit bit 0
    template_bit0_[0] = 0xFFFF;  // Slot 0: HIGH
    template_bit0_[1] = 0x0000;  // Slot 1: LOW
    template_bit0_[2] = 0x0000;  // Slot 2: LOW

    // All lanes transmit bit 1
    template_bit1_[0] = 0xFFFF;  // Slot 0: HIGH
    template_bit1_[1] = 0xFFFF;  // Slot 1: HIGH
    template_bit1_[2] = 0x0000;  // Slot 2: LOW
}

template <typename LED_CHIPSET>
bool LcdI80Driver<LED_CHIPSET>::begin(const LcdDriverConfig& config, int leds_per_strip) {
    config_ = config;
    num_leds_ = leds_per_strip;

    // Use chipset default reset time if not specified
    if (config_.latch_us == 0) {
        config_.latch_us = LED_CHIPSET::RESET();
    }

    // Validate configuration
    if (config_.num_lanes < 1 || config_.num_lanes > 16) {
        ESP_LOGE(LCD_TAG, "Invalid num_lanes: %d (must be 1-16)", config_.num_lanes);
        return false;
    }

    if (num_leds_ < 1) {
        ESP_LOGE(LCD_TAG, "Invalid leds_per_strip: %d", num_leds_);
        return false;
    }

    // Validate GPIO pins
    for (int i = 0; i < config_.num_lanes; i++) {
        int pin = config_.gpio_pins[i];

        // Check for invalid USB-JTAG pins
        if (pin == 19 || pin == 20) {
            ESP_LOGE(LCD_TAG, "GPIO%d is reserved for USB-JTAG and cannot be used for LED output", pin);
            return false;
        }

        // Check for Flash/PSRAM pins
        if (pin >= 26 && pin <= 32) {
            ESP_LOGE(LCD_TAG, "GPIO%d is reserved for SPI Flash/PSRAM and cannot be used for LED output", pin);
            return false;
        }

        // Warning for strapping pins
        if (pin == 0 || pin == 3 || pin == 45 || pin == 46) {
            ESP_LOGW(LCD_TAG, "GPIO%d is a strapping pin - use with caution, may affect boot behavior", pin);
        }

        #if defined(CONFIG_SPIRAM_MODE_OCT) || defined(CONFIG_ESPTOOLPY_FLASHMODE_OPI)
        // Check for Octal Flash/PSRAM pins
        if (pin >= 33 && pin <= 37) {
            ESP_LOGE(LCD_TAG, "GPIO%d is reserved for Octal Flash/PSRAM and cannot be used for LED output", pin);
            return false;
        }
        #endif
    }

    // Generate bit templates
    generateTemplates();

    // Log timing information
    ESP_LOGI(LCD_TAG, "Chipset: %s", LED_CHIPSET::name());
    ESP_LOGI(LCD_TAG, "Target timing: T1=%u ns, T2=%u ns, T3=%u ns",
             LED_CHIPSET::T1(), LED_CHIPSET::T2(), LED_CHIPSET::T3());
    ESP_LOGI(LCD_TAG, "Optimized PCLK: %u Hz (%u MHz)", PCLK_HZ, PCLK_HZ / 1000000);
    ESP_LOGI(LCD_TAG, "Slot duration: %u ns", SLOT_NS);
    ESP_LOGI(LCD_TAG, "Slots per bit: %u", N_BIT);

    uint32_t T1_act, T2_act, T3_act;
    getActualTiming(T1_act, T2_act, T3_act);
    ESP_LOGI(LCD_TAG, "Actual timing: T1=%u ns, T1+T2=%u ns, T3=%u ns",
             T1_act, T1_act + T2_act, T3_act);

    // Calculate buffer size
    // Data: num_leds * 24 bits * N_BIT words * 2 bytes
    // Latch: latch_us converted to slots, then to bytes
    size_t data_size = num_leds_ * 24 * N_BIT * 2;
    size_t latch_slots = (config_.latch_us * 1000) / SLOT_NS;
    size_t latch_size = latch_slots * 2;
    buffer_size_ = data_size + latch_size;

    ESP_LOGI(LCD_TAG, "Buffer size: %u bytes (%u KB)", buffer_size_, buffer_size_ / 1024);
    ESP_LOGI(LCD_TAG, "Frame time (estimated): %u us", getFrameTimeUs());

    // Allocate double buffers
    // LCD peripheral requires DMA-capable memory
    // - Internal DMA RAM: Always works but limited size (~160KB on ESP32-S3)
    // - PSRAM: Requires EDMA (cache-DMA) support, available on ESP32-S3 with proper config
    uint32_t alloc_caps;
    if (config_.use_psram) {
        // Try PSRAM with DMA capability (EDMA on ESP32-S3)
        // MALLOC_CAP_SPIRAM alone isn't DMA-capable, need DMA flag too
        alloc_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT;
    } else {
        // Internal DMA RAM
        alloc_caps = MALLOC_CAP_DMA | MALLOC_CAP_8BIT;
    }

    for (int i = 0; i < 2; i++) {
        buffers_[i] = (uint16_t*)heap_caps_aligned_alloc(
            LCD_DRIVER_PSRAM_DATA_ALIGNMENT,
            buffer_size_,
            alloc_caps
        );

        // Fallback: If PSRAM+DMA allocation failed, try internal DMA RAM only
        if (buffers_[i] == nullptr && config_.use_psram) {
            ESP_LOGW(LCD_TAG, "PSRAM+DMA allocation failed for buffer %d, falling back to internal DMA RAM", i);
            buffers_[i] = (uint16_t*)heap_caps_aligned_alloc(
                LCD_DRIVER_PSRAM_DATA_ALIGNMENT,
                buffer_size_,
                MALLOC_CAP_DMA | MALLOC_CAP_8BIT
            );
        }

        if (buffers_[i] == nullptr) {
            ESP_LOGE(LCD_TAG, "Failed to allocate buffer %d (%u bytes)", i, buffer_size_);
            ESP_LOGE(LCD_TAG, "Free DMA heap: %u bytes, largest block: %u bytes",
                     heap_caps_get_free_size(MALLOC_CAP_DMA),
                     heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
            end();
            return false;
        }

        // Initialize buffer with zeros (latch gap pre-filled)
        memset(buffers_[i], 0, buffer_size_);
    }

    ESP_LOGI(LCD_TAG, "Allocated 2 buffers at %p, %p", buffers_[0], buffers_[1]);

    // Create semaphore for DMA synchronization
    xfer_done_sem_ = xSemaphoreCreateBinary();
    if (xfer_done_sem_ == nullptr) {
        ESP_LOGE(LCD_TAG, "Failed to create semaphore");
        end();
        return false;
    }
    xSemaphoreGive(xfer_done_sem_);  // Start in "transfer done" state

    // Initialize I80 bus
    esp_lcd_i80_bus_config_t bus_config = {};
    bus_config.clk_src = LCD_CLK_SRC_PLL160M;
    bus_config.dc_gpio_num = -1;  // Not used
    bus_config.wr_gpio_num = -1;  // Not used - WS2812 doesn't need external WR signal
    bus_config.bus_width = 16;
    bus_config.max_transfer_bytes = buffer_size_;

    for (int i = 0; i < 16; i++) {
        if (i < config_.num_lanes) {
            bus_config.data_gpio_nums[i] = config_.gpio_pins[i];
        } else {
            bus_config.data_gpio_nums[i] = -1;  // Unused lanes
        }
    }

    // IDF 5.4 and earlier use deprecated alignment fields
    #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 0)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    bus_config.psram_trans_align = LCD_DRIVER_PSRAM_DATA_ALIGNMENT;
    bus_config.sram_trans_align = 4;
    #pragma GCC diagnostic pop
    #else
    // IDF 5.5+ uses new dma_burst_size field (replaces both psram/sram alignment)
    bus_config.dma_burst_size = 64;
    #endif

    esp_err_t bus_err = esp_lcd_new_i80_bus(&bus_config, &bus_handle_);
    if (bus_err != ESP_OK) {
        ESP_LOGE(LCD_TAG, "Failed to create I80 bus: %d", bus_err);
        end();
        return false;
    }

    // Initialize panel IO
    esp_lcd_panel_io_i80_config_t io_config = {};
    io_config.cs_gpio_num = -1;  // Not used
    io_config.pclk_hz = PCLK_HZ;
    io_config.trans_queue_depth = 1;  // Critical: single transfer to avoid gaps
    io_config.dc_levels = {
        .dc_idle_level = 0,
        .dc_cmd_level = 0,
        .dc_dummy_level = 0,
        .dc_data_level = 1,
    };
    io_config.lcd_cmd_bits = 0;
    io_config.lcd_param_bits = 0;
    io_config.on_color_trans_done = dmaCallback;
    io_config.user_ctx = this;

    esp_err_t panel_err = esp_lcd_new_panel_io_i80(bus_handle_, &io_config, &io_handle_);
    if (panel_err != ESP_OK) {
        ESP_LOGE(LCD_TAG, "Failed to create panel IO: %d", panel_err);
        end();
        return false;
    }

    ESP_LOGI(LCD_TAG, "LCD driver initialized successfully");
    return true;
}

template <typename LED_CHIPSET>
void LcdI80Driver<LED_CHIPSET>::end() {
    // Wait for any pending transfer
    if (dma_busy_) {
        wait();
    }

    // Release ESP-LCD resources
    if (io_handle_ != nullptr) {
        esp_lcd_panel_io_del(io_handle_);
        io_handle_ = nullptr;
    }

    if (bus_handle_ != nullptr) {
        esp_lcd_del_i80_bus(bus_handle_);
        bus_handle_ = nullptr;
    }

    // Free buffers
    for (int i = 0; i < 2; i++) {
        if (buffers_[i] != nullptr) {
            heap_caps_free(buffers_[i]);
            buffers_[i] = nullptr;
        }
    }

    // Delete semaphore
    if (xfer_done_sem_ != nullptr) {
        vSemaphoreDelete(xfer_done_sem_);
        xfer_done_sem_ = nullptr;
    }
}

template <typename LED_CHIPSET>
void LcdI80Driver<LED_CHIPSET>::encodeFrame(int buffer_index) {
    uint16_t* output = buffers_[buffer_index];
    uint8_t pixel_bytes[16];  // One byte per lane for current color component
    uint16_t lane_bits[8];    // Transposed bits (8 words for 8 bit positions)

    // Encode all LEDs
    for (int led_idx = 0; led_idx < num_leds_; led_idx++) {
        // Process color components in GRB order (WS28xx standard)
        const int color_order[3] = {1, 0, 2};  // G, R, B indices into CRGB

        for (int color = 0; color < 3; color++) {
            int component = color_order[color];

            // Gather bytes for this color component across all lanes
            for (int lane = 0; lane < config_.num_lanes; lane++) {
                if (strips_[lane] != nullptr) {
                    pixel_bytes[lane] = strips_[lane][led_idx].raw[component];
                } else {
                    pixel_bytes[lane] = 0;
                }
            }

            // Fill unused lanes with zeros
            for (int lane = config_.num_lanes; lane < 16; lane++) {
                pixel_bytes[lane] = 0;
            }

            // Transpose 16 bytes into 8 words (one bit per lane)
            LcdDriverBase::transpose16x1(pixel_bytes, lane_bits);

            // Encode each bit (MSB first: bit 7 down to bit 0)
            for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
                uint16_t current_bit_mask = lane_bits[bit_idx];

                // Apply templates with bit masking
                // For each slot, select bit0 or bit1 template based on lane bits
                for (uint32_t slot = 0; slot < N_BIT; slot++) {
                    output[slot] = (template_bit0_[slot] & ~current_bit_mask) |
                                   (template_bit1_[slot] & current_bit_mask);
                }

                output += N_BIT;  // Advance to next bit
            }
        }
    }

    // Latch gap already pre-filled with zeros during buffer allocation
}

template <typename LED_CHIPSET>
bool LcdI80Driver<LED_CHIPSET>::show() {
    // Check if previous transfer is still running
    if (dma_busy_) {
        return false;  // Busy, cannot start new transfer
    }

    // Take semaphore (should be available immediately if not busy)
    if (xSemaphoreTake(xfer_done_sem_, 0) != pdTRUE) {
        return false;  // Race condition, still busy
    }

    // Encode frame into back buffer
    int back_buffer = 1 - front_buffer_;
    encodeFrame(back_buffer);

    // Mark as busy before starting transfer
    dma_busy_ = true;

    // Start DMA transfer
    // Note: tx_color expects (panel_io, command, color_data, color_size)
    // We use command=-1 for no command and pass entire buffer as color data
    esp_err_t err = esp_lcd_panel_io_tx_color(
        io_handle_,
        -1,  // No command byte
        buffers_[back_buffer],
        buffer_size_
    );

    if (err != ESP_OK) {
        ESP_LOGE(LCD_TAG, "DMA transfer failed: %d", err);
        dma_busy_ = false;
        xSemaphoreGive(xfer_done_sem_);  // Release semaphore
        return false;
    }

    // Swap buffers
    front_buffer_ = back_buffer;
    frame_counter_++;

    return true;
}

// IRAM_ATTR is safe on template function because we use explicit instantiation in .cpp
// The explicit instantiation forces compilation and proper IRAM placement
template <typename LED_CHIPSET>
bool IRAM_ATTR LcdI80Driver<LED_CHIPSET>::dmaCallback(esp_lcd_panel_io_handle_t panel_io,
                                                      esp_lcd_panel_io_event_data_t* edata,
                                                      void* user_ctx) {
    LcdI80Driver<LED_CHIPSET>* driver = static_cast<LcdI80Driver<LED_CHIPSET>*>(user_ctx);

    // Mark transfer as complete
    driver->dma_busy_ = false;

    // Signal semaphore
    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(driver->xfer_done_sem_, &higher_priority_task_woken);

    return higher_priority_task_woken == pdTRUE;
}

}  // namespace fl
