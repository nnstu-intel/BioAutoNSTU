#include <string>
#include <iostream>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <json.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp> 

#include <base64.hpp>
#include <config.hpp>
#include <broker.hpp>
#include <globals.hpp>

using nlohmann::json;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace messages {
    std::string ok(const std::string& reason) {
        json body = json::object();
        body["type"] = std::string("OK_STATUS");
        body["payload"] = json::object();
        body["payload"]["for"] = reason;

        return body.dump();
    }

    std::string error(const std::string& reason) {
        json body = json::object();
        body["type"] = std::string("ERROR_STATUS");
        body["payload"] = json::object();
        body["payload"]["for"] = reason;

        return body.dump();
    }

    std::string connectPI() {
        json body = json::object();
        body["type"] = std::string("CONNECT_PI");
        body["payload"] = json::object();
        body["payload"]["pin"] = global_pi_configuration.devicePIN;
        body["payload"]["name"] = global_pi_configuration.deviceName;

        return body.dump();
    }

    std::string updatePIUsers() {
        json body = json::object();
        body["type"] = std::string("UPDATE_PI_USERS");
        body["payload"] = json::object();
        body["payload"]["users"] = json::array();

        for (const User& user: global_pi_users) {
            body["payload"]["users"].push_back(user.toJSON());
        }

        return body.dump();
    }
}

void create_user(
    json& payload,
    websocket::stream<tcp::socket>& websocket
 ) {
    const std::string decoded_image = base64_decode(payload.at("image").get<std::string>());
    const int size = int(decoded_image.size());
    char* data = new char[size];
    try {
        std::lock_guard<std::mutex> detector_guard(global_pi_face_detector_mutex);
        std::lock_guard<std::mutex> classifier_guard(global_pi_classifier_mutex);
        memcpy(data, decoded_image.c_str(), size);
        cv::Mat image = cv::imdecode(cv::Mat(1, int(decoded_image.size()), CV_8UC1, data), cv::IMREAD_COLOR);
        cv::Mat gray, face;
        std::vector<cv::Rect> faces;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        global_pi_face_detector.detectMultiScale(gray, faces, 1.5, 5, 0, cv::Size(150, 150));
        if (faces.size() < 1) {
            throw std::runtime_error("Faces was not found on the image");
        }

        if (faces.size() > 1) {
            throw std::runtime_error("Multiple faces found on the image");
        }

        face = image(faces[0]);
        cv::resize(face, face, cv::Size(160, 160));
        payload["descriptor"] = global_pi_classifier->embed(face);

        std::lock_guard<std::mutex> users_guard(global_pi_users_mutex);
        User new_user;
        new_user.parseJSON(payload);
        global_pi_users.push_back(new_user);
        update_users(
            global_pi_users,
            global_pi_configuration.dbFile,
            global_pi_configuration.networkVersion
        );

        std::string response = messages::ok(std::string("CREATE_PI_USER"));
        std::cout << "Sending " << response << std::endl;
        websocket.write(net::buffer(response));

        response = messages::updatePIUsers();
        std::cout << "Sending " << response << std::endl;
        websocket.write(net::buffer(response));
    } catch (std::exception& ex) {
        delete[] data;
        throw ex;
    }
    
    delete[] data;
}

void handle_message(
    const std::string& message,
    websocket::stream<tcp::socket>& websocket
) {
    json body = json::parse(message);
    const std::string type = body.at("type").get<std::string>();
    if (type == std::string("OK_STATUS") 
        && body["payload"].is_object() 
        && body["payload"]["for"].is_string()
    ) {
        if (body["payload"]["for"] == std::string("CONNECT_PI")) {
            std::lock_guard<std::mutex> guard(global_pi_users_mutex);
            const std::string response = messages::updatePIUsers();
            std::cout << "Sending " << response << std::endl;
            websocket.write(net::buffer(response));
            return;
        }
    } else if (type == std::string("ERROR_STATUS") 
        && body["payload"].is_object() 
        && body["payload"]["for"].is_string()
    ) {
        std::cout << "Got error response status for " << body["payload"]["for"] << " request" << std::endl;
    } else if (body["type"] == std::string("CREATE_PI_USER")) {
        try {
            std::lock_guard<std::mutex> guard(global_pi_users_mutex);
            create_user(body["payload"], websocket);
        } catch (std::exception& ex) {
            std::cout << "Could not create a user" << std::endl;
            const std::string response = messages::error(std::string("CREATE_PI_USER"));
            std::cout << "Sending " << response << std::endl;
            websocket.write(net::buffer(response));
        }
    } else if (body["type"] == std::string("REMOVE_PI_USER")) {
        // remove user
        // rewrite users
        // send ok
        // send update users
    }

    return;
}

void connect() {
    net::io_context ioc;
    websocket::stream<tcp::socket> websocket{ioc};
    
    while (true) {
        try {
            tcp::resolver resolver(ioc);
            boost::system::error_code ec;
            std::cout 
                << "Resolving "
                << global_pi_configuration.brokerHost
                << ":"
                << global_pi_configuration.brokerPort
                << std::endl;

            tcp::resolver::results_type results = resolver.resolve(
                global_pi_configuration.brokerHost,
                global_pi_configuration.brokerPort,
                ec
            );

            // Failed to resolve the DNS name. Breaking execution.
            if (ec.value() != 0) {
                std::string message = 
                    std::string("Failed to resolve a DNS name. Error code = ") 
                        + std::to_string(ec.value()) + std::string(". Message = ") + ec.message();
                throw std::runtime_error(message);
            }

            std::cout 
                << "Connecting "
                << global_pi_configuration.brokerHost
                << ":"
                << global_pi_configuration.brokerPort
                << std::endl;

            net::connect(
                websocket.next_layer(),
                results.begin(),
                results.end()
            );

            websocket.handshake(global_pi_configuration.brokerHost, "/");
            
            std::string out = messages::connectPI();
            std::cout 
                << "Sending "
                << out
                << std::endl;

            websocket.write(net::buffer(out));

            // read message in circle and make response depends on the message
            while (true) {
                beast::flat_buffer buffer;
                websocket.read(buffer);
                
                try {
                    std::string message = beast::buffers_to_string(buffer.data());
                    std::cout
                        << "Receiving "
                        << message
                        << std::endl;
                    handle_message(message, websocket);
                } catch (std::exception& ex) {
                    std::cout << ex.what() << std::endl;                   
                    continue;
                }
            }
        } catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
            if (websocket.is_open()) {
                websocket.close(websocket::close_code::normal);
            }
            
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    global_pi_configuration.reconnectTimeSec
                )
            );
        }
    }

    websocket.close(websocket::close_code::normal);
}
