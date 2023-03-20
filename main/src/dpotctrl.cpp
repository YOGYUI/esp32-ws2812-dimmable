/**
 * @file dpotctrl.cpp
 * @author yogyui
 * @brief PmodDPOT(Digilent) AD5160 digital potentiometer control module
 * @version 0.1
 * @date 2023-02-16
 * 
 * @copyright Copyright (c) 2023
 */
#include "dpotctrl.h"
#include "definition.h"
#include "logger.h"
#include "driver/gpio.h"

#define DPOT_INIT_VALUE 128

CDpotCtrl* CDpotCtrl::_instance = nullptr;

CDpotCtrl::CDpotCtrl()
{
    m_handle = nullptr;
    m_value = 0;
    memset(&m_spi_transaction, 0, sizeof(m_spi_transaction));
    m_spi_transaction.length = 8;
    m_spi_transaction.rxlength = 0;
    m_spi_transaction.user = nullptr;
}

CDpotCtrl::~CDpotCtrl()
{
}

CDpotCtrl* CDpotCtrl::Instance()
{
    if (!_instance) {
        _instance = new CDpotCtrl();
    }

    return _instance;
}

bool CDpotCtrl::initialize()
{
    esp_err_t ret;

    // SPI Bus 초기화    
    spi_bus_config_t cfg_bus;
    memset(&cfg_bus, 0, sizeof(cfg_bus));
    cfg_bus.mosi_io_num = PIN_DPOT_SPI_MOSI;
    cfg_bus.miso_io_num = PIN_DPOT_SPI_MISO;
    cfg_bus.sclk_io_num = PIN_DPOT_SPI_SCLK;
    cfg_bus.quadwp_io_num = -1;
    cfg_bus.quadhd_io_num = -1;
    cfg_bus.data4_io_num = -1;
    cfg_bus.data5_io_num = -1;
    cfg_bus.data6_io_num = -1;
    cfg_bus.data7_io_num = -1;
    cfg_bus.max_transfer_sz = 4092;
    // cfg_bus.isr_cpu_id = INTR_CPU_ID_AUTO;   // esp-idf >= v5.0
    cfg_bus.intr_flags = 0;
    ret = spi_bus_initialize(DPOT_SPI_HOST, &cfg_bus, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to initialize spi bus (ret: %d)", ret);
        return false;
    }

    // SPI Bus에 디바이스 부착
    spi_device_interface_config_t cfg_dev_if;
    memset(&cfg_dev_if, 0, sizeof(cfg_dev_if));
    // cfg_dev_if.clock_source = SPI_CLK_SRC_DEFAULT;   // esp-idf >= v5.0
    cfg_dev_if.clock_speed_hz = 10 * 1000 * 1000;    // Max 25MHz
    cfg_dev_if.mode = 0;
    cfg_dev_if.spics_io_num = PIN_DPOT_SPI_CS;
    cfg_dev_if.pre_cb = nullptr;
    cfg_dev_if.post_cb = nullptr;
    cfg_dev_if.queue_size = 7;
    ret = spi_bus_add_device(DPOT_SPI_HOST, &cfg_dev_if, &m_handle);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to add device to spi bus (ret: %d)", ret);
        return false;
    }

    return set_raw_value(DPOT_INIT_VALUE);
}

bool CDpotCtrl::set_raw_value(uint8_t value)
{
    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("spi handle is not initialized");
        return false;
    }

    m_value = value;
    m_spi_transaction.tx_buffer=&value;
    esp_err_t ret = spi_device_polling_transmit(m_handle, &m_spi_transaction);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to set potentiometer value");
        return false;
    }

    float rwb = (float)value / 256.f * DPOT_RAB_RESISTANCE + DPOT_RW_RESISTANCE;
    if (rwb < 1000)
        GetLogger(eLogType::Info)->Log("set dpot value: %d, expected resistance Rwb=%g", value, rwb);
    else
        GetLogger(eLogType::Info)->Log("set dpot value: %d, expected resistance Rwb=%gK", value, rwb / 1000.f);
    return true;
}

bool CDpotCtrl::set_resistance_wb(float res)
{
    float ctrl_val = (res - DPOT_RW_RESISTANCE) * 256.f / DPOT_RAB_RESISTANCE;
    return set_raw_value((uint8_t)ctrl_val);
}
