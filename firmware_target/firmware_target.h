#pragma once

typedef struct {
    const uint8_t *data;
    uint32_t size;
    uint32_t addr;
} partition_attr_t;

typedef struct {
    partition_attr_t boot;
    partition_attr_t part;
    partition_attr_t app;
    partition_attr_t ota;
} target_binaries_t;


void get_binaries(target_chip_t target, target_binaries_t *binaries);
esp_loader_error_t connect_to_target(uint32_t higrer_baudrate);
esp_loader_error_t flash_binary(const uint8_t *bin, size_t size, size_t address);