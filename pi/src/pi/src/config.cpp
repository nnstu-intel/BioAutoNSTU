#include <config.hpp>
#include <json.hpp>

using nlohmann::json;

PIConfiguration defaultPIConfiguration = {
    "Raspberry PI 4 Model B", 
    "0000",
    "facenet128",
    "MYRIAD",
    "cascade.xml",
    "people.json",
    "localhost",
    "8080",
    10,  // reconnect time
    100, // read sensor period  
    29,  // red 
    23,  // green
    3,
    false,
    {
        "facenet.bin",
        "facenet.xml"
    }
};

PIConfiguration initialize_config(const std::string& filename) {
    PIConfiguration piConfiguration;
    std::ifstream config_file(filename, std::ios::in);
    if (config_file.is_open()) {
        try {
            std::stringstream buffer;
            buffer << config_file.rdbuf();
            const std::string content = buffer.str();
            json config = json::parse(content);
            if (config["deviceName"].is_string()) {
                piConfiguration.deviceName = config["deviceName"].get<std::string>();
            } else {
                piConfiguration.deviceName = defaultPIConfiguration.deviceName;
            }

            if (config["devicePIN"].is_string()) {
                piConfiguration.devicePIN = config["devicePIN"].get<std::string>();
            } else {
                piConfiguration.devicePIN = defaultPIConfiguration.devicePIN;
            }

            if (config["networkVersion"].is_string()) {
                piConfiguration.networkVersion = config["networkVersion"].get<std::string>();
            } else {
                piConfiguration.networkVersion = defaultPIConfiguration.networkVersion;
            }

            if (config["faceHaarCascade"].is_string()) {
                piConfiguration.faceHaarCascade = config["faceHaarCascade"].get<std::string>();
            } else {
                piConfiguration.faceHaarCascade = defaultPIConfiguration.faceHaarCascade;
            }

            if (config["inferenceBackend"].is_string()) {
                piConfiguration.inferenceBackend = config["inferenceBackend"].get<std::string>();
            } else {
                piConfiguration.inferenceBackend = defaultPIConfiguration.inferenceBackend;
            }

            if (config["dbFile"].is_string()) {
                piConfiguration.dbFile = config["dbFile"].get<std::string>();
            } else {
                piConfiguration.dbFile = defaultPIConfiguration.dbFile;
            }

            if (config["brokerHost"].is_string()) {
                piConfiguration.brokerHost = config["brokerHost"].get<std::string>();
            } else {
                piConfiguration.brokerHost = defaultPIConfiguration.brokerHost;
            }

            if (config["brokerPort"].is_string()) {
                piConfiguration.brokerPort = config["brokerPort"].get<std::string>();
            } else {
                piConfiguration.brokerPort = defaultPIConfiguration.brokerPort;
            }

            if (config["reconnectTimeSec"].is_number()) {
                piConfiguration.reconnectTimeSec = config["reconnectTimeSec"].get<uint>();
            } else {
                piConfiguration.reconnectTimeSec = defaultPIConfiguration.reconnectTimeSec;
            }

            if (config["readSensorTimeMs"].is_number()) {
                piConfiguration.readSensorTimeMs = config["readSensorTimeMs"].get<uint>();
            } else {
                piConfiguration.readSensorTimeMs = defaultPIConfiguration.readSensorTimeMs;
            }

            if (config["redDiodeGPIO"].is_number()) {
                piConfiguration.redDiodeGPIO = config["redDiodeGPIO"].get<uint>();
            } else {
                piConfiguration.redDiodeGPIO = defaultPIConfiguration.redDiodeGPIO;
            }

            if (config["greenDiodeGPIO"].is_number()) {
                piConfiguration.greenDiodeGPIO = config["greenDiodeGPIO"].get<uint>();
            } else {
                piConfiguration.greenDiodeGPIO = defaultPIConfiguration.greenDiodeGPIO;
            }

            if (config["hcSR501GPIO"].is_number()) {
                piConfiguration.hcSR501GPIO = config["hcSR501GPIO"].get<uint>();
            } else {
                piConfiguration.hcSR501GPIO = defaultPIConfiguration.hcSR501GPIO;
            }

            if (config["UI"].is_boolean()) {
                piConfiguration.UI = config["UI"].get<bool>();
            } else {
                piConfiguration.UI = defaultPIConfiguration.UI;
            }

            if (config["network"].is_object()) {
                if (config["network"]["xml"].is_string()) {
                    piConfiguration.network.xml = config["network"]["xml"].get<std::string>();
                } else {
                    piConfiguration.network.xml = defaultPIConfiguration.network.xml;
                }
                if (config["network"]["bin"].is_string()) {
                    piConfiguration.network.bin = config["network"]["bin"].get<std::string>();
                } else {
                    piConfiguration.network.bin = defaultPIConfiguration.network.bin;
                }
            } else {
                piConfiguration.network.xml = defaultPIConfiguration.network.xml;
                piConfiguration.network.bin = defaultPIConfiguration.network.bin;
            }
        } catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }

        config_file.close();
    } else {
        piConfiguration = defaultPIConfiguration;
        std::cout << "Could not open config file " << filename << std::endl;
        std::cout << "Default configuration was setup" << std::endl;
    }

    return piConfiguration;
}

void print_config(const PIConfiguration& configuration) {
    std::cout << "\n\n\nConfiguration: " << std::endl;
    std::cout << "\tDevice name: " << configuration.deviceName << std::endl;
    std::cout << "\tDevice PIN: " << configuration.devicePIN << std::endl;
    std::cout << "\tNetwork version: " << configuration.networkVersion << std::endl;
    std::cout << "\tNeural backend: " << configuration.inferenceBackend << std::endl;
    std::cout << "\tHaar cascade: " << configuration.faceHaarCascade << std::endl;
    std::cout << "\tDatabase file: " << configuration.dbFile << std::endl;
    std::cout << "\tBroker host: " << configuration.brokerHost << std::endl;
    std::cout << "\tBroker port: " << configuration.brokerPort << std::endl;
    std::cout << "\tReconnect to broker time (sec): " << configuration.reconnectTimeSec << std::endl;
    std::cout << "\tRead sensors period (ms): " << configuration.readSensorTimeMs << std::endl;
    std::cout << "\tRed diode GPIO number: " << configuration.redDiodeGPIO << std::endl;
    std::cout << "\tGreen diode GPIO number: " << configuration.greenDiodeGPIO << std::endl;
    std::cout << "\tSensor HC SR-501 GPIO number: " << configuration.hcSR501GPIO << std::endl;
    std::cout << "\tWith UI: " << (configuration.UI ? "yes" : "no") << std::endl;
    std::cout << "\tModel: " << std::endl;
    std::cout << "\t\tXML: " << configuration.network.xml << std::endl;
    std::cout << "\t\tBIN: " << configuration.network.bin << "\n\n\n" << std::endl;
}
