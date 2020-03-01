#include <chrono>
#include <thread>
#include <iostream>
#include <functional>
#include <wiringPi.h>

#include <broker.hpp>
#include <globals.hpp>


// create classifier
// create detector
// add mutex detector
// add mutex classifier

PIConfiguration global_pi_configuration;
std::vector<User> global_pi_users;
std::shared_ptr<Classifier> global_pi_classifier;
cv::CascadeClassifier global_pi_face_detector;
std::mutex global_pi_users_mutex;
std::mutex global_pi_classifier_mutex;
std::mutex global_pi_face_detector_mutex;

int main() {
    global_pi_configuration = initialize_config(std::string("config.json"));
    print_config(global_pi_configuration);

    global_pi_users = read_users(global_pi_configuration.dbFile, global_pi_configuration.networkVersion);
    global_pi_face_detector.load(global_pi_configuration.faceHaarCascade);
    // global_pi_classifier = build_classifier(
    //     ClassifierType::IE_Facenet_V1,
    //     global_pi_configuration.network.xml,
    //     global_pi_configuration.network.bin,
    //     global_pi_configuration.inferenceBackend
    // );


    std::thread socket_thread(connect);

    wiringPiSetup();    
    pinMode(global_pi_configuration.redDiodeGPIO, OUTPUT);
    pinMode(global_pi_configuration.greenDiodeGPIO, OUTPUT);
    pinMode(global_pi_configuration.hcSR501GPIO, INPUT);

    bool movement = false;
    while(true) {
        if (digitalRead(global_pi_configuration.hcSR501GPIO)) {
            digitalWrite(global_pi_configuration.redDiodeGPIO, 1); 
            digitalWrite(global_pi_configuration.greenDiodeGPIO, 0); 
        } else {
            digitalWrite(global_pi_configuration.redDiodeGPIO, 1); 
            digitalWrite(global_pi_configuration.greenDiodeGPIO, 1); 
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                global_pi_configuration.readSensorTimeMs
            )
        );
    }

    socket_thread.join();
    return EXIT_SUCCESS;
}
