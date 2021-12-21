#ifndef PI_CONFIG_CPP
#define PI_CONFIG_CPP

#include <string>
#include <iostream>
#include <fstream>

struct PIConfiguration {
    std::string deviceName;
    std::string devicePIN;
    std::string networkVersion;
    std::string inferenceBackend;
    std::string faceHaarCascade;
    std::string dbFile;
    std::string brokerHost;
    std::string brokerPort;
    uint reconnectTimeSec;
    uint readSensorTimeMs;
    uint redDiodeGPIO;
    uint greenDiodeGPIO;
    uint hcSR501GPIO;
    bool UI;
    struct {
        std::string bin;
        std::string xml;
    } network;
};

PIConfiguration initialize_config(const std::string& filename = std::string());
void print_config(const PIConfiguration& configuration);

#endif 