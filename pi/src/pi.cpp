#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>

#include <json.hpp>
#include <wiringPi.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

using nlohmann::json;

struct PIConfiguration {
    std::string deviceName;
    std::string devicePIN;
    std::string inferenceBackend;
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

PIConfiguration piConfiguration;
PIConfiguration defaultPIConfiguration = {
    "Raspberry PI 4 Model B", 
    "0000",
    "MYRIAD",
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
        "", // bin
        ""  // xml
    }
};


void initialize_config(const std::string& filename = std::string("")) {
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
}

void print_config() {
    std::cout << "\n\n\nConfiguration: " << std::endl;
    std::cout << "\tDevice name: " << piConfiguration.deviceName << std::endl;
    std::cout << "\tDevice PIN: " << piConfiguration.devicePIN << std::endl;
    std::cout << "\tNeural backend: " << piConfiguration.inferenceBackend << std::endl;
    std::cout << "\tDatabase file: " << piConfiguration.dbFile << std::endl;
    std::cout << "\tBroker host: " << piConfiguration.brokerHost << std::endl;
    std::cout << "\tBroker port: " << piConfiguration.brokerPort << std::endl;
    std::cout << "\tReconnect to broker time (sec): " << piConfiguration.reconnectTimeSec << std::endl;
    std::cout << "\tRead sensors period (ms): " << piConfiguration.readSensorTimeMs << std::endl;
    std::cout << "\tRed diode GPIO number: " << piConfiguration.redDiodeGPIO << std::endl;
    std::cout << "\tGreen diode GPIO number: " << piConfiguration.greenDiodeGPIO << std::endl;
    std::cout << "\tSensor HC SR-501 GPIO number: " << piConfiguration.hcSR501GPIO << std::endl;
    std::cout << "\tWith UI: " << (piConfiguration.UI ? "yes" : "no") << std::endl;
    std::cout << "\tModel: " << std::endl;
    std::cout << "\t\tXML: " << piConfiguration.network.xml << std::endl;
    std::cout << "\t\tBIN: " << piConfiguration.network.bin << "\n\n\n" << std::endl;
}

const int RED_DIODE_PIN = 29;
const int GREEN_DIODE_PIN = 23;
const int HC_SR501_PIN = 3;

const std::chrono::milliseconds WAIT_TIME(100);


unsigned int id_generator(unsigned int initial_low_bound = 0) {
    static unsigned int id = 0;
    if (initial_low_bound) {
        id = initial_low_bound;
        return id;
    }

    return ++id;
}

// все храним в JSON файле
// при старте программы считываем его
// изменяем при прослушивании веб сокета (так же читаем по новой из файла после каждого изменения)

// при получении дескриптора мы можем посмотреть на его размер. 
// Если размер дескриптора на совпадает с тем размером, что в базе данных, просто игнорируем его




class User {
    private:
        unsigned int _id = 0;
        std::string _firstname;
        std::string _secondname;
        std::string _patronymic;
        std::string _passport;
        std::vector<float> _descriptor;
    public:
        User(
            const std::string firstname,
            const std::string secondname,
            const std::string patronymic,
            const std::string passport
        ) {
            this->_id = id_generator();
            this->_firstname = firstname;
            this->_secondname = secondname;
            this->_patronymic = patronymic;
            this->_passport = passport;
        }

        void embed(const std::vector<float> descriptor) {
            this->_descriptor.resize(descriptor.size());
            std::copy(descriptor.begin(), descriptor.end(), this->_descriptor.begin());
        }

        json toJSON() const {
            json result;
            result["firstname"] = this->_firstname;
            result["secondname"] = this->_secondname;
            if (!this->_patronymic.empty()) {
                result["patronymic"] = this->_patronymic;
            }
            result["passport"] = this->_passport;
            result["id"] = this->_id;
            if (this->_descriptor.size()) {
                result["descriptor"] = json(this->_descriptor);
            }
            return result;
        }

        void parseJSON(const json& source) {
            const json& firstname = source["firstname"];
            const json& secondname = source["secondname"];
            const json& patronymic = source["patronymic"];
            const json& passport = source["passport"];
            const json& id = source["id"];
            const json& descriptor = source["descriptor"];

            this->_firstname = firstname.get<std::string>();
            this->_secondname = secondname.get<std::string>();
            this->_passport = passport.get<std::string>();
            this->_id = id.get<unsigned int>();
            if (patronymic.is_string()) {
                this->_patronymic = patronymic.get<std::string>();
            }   

            if (descriptor.is_array()) {
                this->_descriptor = descriptor.get<std::vector<float>>();
            }
        }

        ~User() {}
};

const std::vector<User> users;


// class Session {
//     private:
//         websocket::stream<tcp::socket> _ws;
//         boost::asio::ip::tcp::resolver _resolver;
//         const std::string _host;
//     public:
//         Session(boost::asio::io_context& ioc, const std::string& host): _host(host), _ws(ioc), _resolver(ioc) {
            
//         }

//         void connect() {
//             //boost::asio::async_connect(this->_ws, )
//         }

//         void on_connect(boost::system::error_code ec) {

//         }

//         void on_close(boost::system::error_code ec) {

//         }

//         void on_read(boost::system::error_code ec,
//             std::size_t bytes_transferred) {
            
//         }

//         void on_write(boost::system::error_code ec,
//             std::size_t bytes_transferred) {

//         }
// };

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

std::string connectPI() {
    const std::string name("Raspberry PI 4B");
    const std::string type("CONNECT_PI");
    const std::string pin("0000");
    json body;
    body["type"] = type;
    json payload;
    payload["pin"] = pin;
    payload["name"] = name;
    body["payload"] = payload;
    return body.dump();
}

void connectToBroker() {
    const std::string host("localhost");
    const std::string port("8080");
    const std::chrono::seconds WAIT_FOR_RECONNECT(10);
    net::io_context ioc;
    websocket::stream<tcp::socket> websocket{ioc};
    
    while (true) {
        try {
            tcp::resolver resolver(ioc);
            boost::system::error_code ec;
            std::cout << "Resolving " << host << ":" << port << std::endl;
            tcp::resolver::results_type results = resolver.resolve(host, port, ec);

            // Failed to resolve the DNS name. Breaking execution.
            if (ec.value() != 0) {
                std::string message = 
                    std::string("Failed to resolve a DNS name. Error code = ") 
                        + std::to_string(ec.value()) + std::string(". Message = ") + ec.message();
                throw std::runtime_error(message);
            }

            std::cout << "Connecting " << host << ":" << port << std::endl;
            net::connect(websocket.next_layer(), results.begin(), results.end());
            websocket.handshake(host, "/");
            
            std::string out = connectPI();
            std::cout << "Sending " << out << std::endl;
            websocket.write(net::buffer(out));

            // read message in circle and make response depends on the message
            while (true) {
                beast::flat_buffer buffer;
                websocket.read(buffer);
                std::string type;
                
                try {
                    std::string message = beast::buffers_to_string(buffer.data());
                    json body = json::parse(message);
                    type = body.at("type").get<std::string>();
                    std::cout << type << std::endl;
                } catch (std::exception& ex) {
                    std::cout << ex.what() << std::endl;
                    // if (!websocket.is_open()) {
                    //     break;
                    // }
                    
                    continue;
                }
            }
        } catch (std::exception& ex) {
            std::cout << "handler";
            std::cout << ex.what() << std::endl;
            if (websocket.is_open()) {
                websocket.close(websocket::close_code::normal);
            }
            
            std::this_thread::sleep_for(WAIT_FOR_RECONNECT);
        }
    }
    
    std::cout << "end of function";
    websocket.close(websocket::close_code::normal);
}

int main() {
    initialize_config();
    print_config();

    std::thread socket_thread(connectToBroker);


    wiringPiSetup();    
    pinMode(RED_DIODE_PIN, OUTPUT);
    pinMode(GREEN_DIODE_PIN, OUTPUT);
    pinMode(HC_SR501_PIN, INPUT);

    bool status = false;
    bool movement = false;
    while(true) {
        // no movement: both are lighting
        // familiar face: 
        if (digitalRead(HC_SR501_PIN)) {
            digitalWrite(RED_DIODE_PIN, 1); 
            digitalWrite(GREEN_DIODE_PIN, 0); 
            // 
        } else {
            digitalWrite(RED_DIODE_PIN, 1); 
            digitalWrite(GREEN_DIODE_PIN, 1); 
        }

        std::this_thread::sleep_for(WAIT_TIME);
    }

    socket_thread.join();
    return EXIT_SUCCESS;
}

// Add file with users. Add method to read it and update

// Add database file
// Implement UPDATE_PI_USERS, CREATE_PI_USER, REMOVE_PI_USER requests (decode base64, decode jpeg and open it)


// pi
    // src: user.cpp, config.cpp, main.cpp, connect.cpp
    // include: user.hpp => User, read_users, update_users
    //          config.hpp => PIConfiguration, read_config, print_config
    //          connect.hpp => connect_broker


