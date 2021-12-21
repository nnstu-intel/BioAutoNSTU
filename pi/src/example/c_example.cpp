/*
    Author: Boris Sekachev
    Email: b.sekachev@yandex.ru
    Organization: NNTU
    Year: 2019
*/

#include <filesystem>
#include <iostream>
#include <memory>
#include <map>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

#include "cwrapper.h"

// Regardless of we write in C++,
// In this sample we use C interface of the library
int main() {
    // First of all we have to init the classififer
    int status = init_ie_facenet_v1(
        "../data/facenet.xml",
        "../data/facenet.bin",
        "CPU"
    );

    // We can't throw exceptions from CPP shared libraries into C code
    // So, we check if something was wrong and get a message
    // The same with other API methods except of receive_error()
    if (status == EXIT_FAILURE) {
        std::cout << receive_error() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<cv::Rect> faces;
    cv::VideoCapture capture(0);
    cv::CascadeClassifier cascade;
    cv::Mat image, gray, face_image;

    // Load face detector
    cascade.load("../data/haarcascade_frontalface_default.xml");

    // Find all people in the directory
    std::map<std::string, std::vector<float>> people;
    for (const auto &entry : std::filesystem::directory_iterator("../data/people")) {
        // Get person image
        image = cv::imread(entry.path(), cv::IMREAD_COLOR);

        // Find faces
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cascade.detectMultiScale(gray, faces, 1.5, 5, 0, cv::Size(150, 150));

        // There must be one face per image
        face_image = image(faces[0]);

        // Get and save embedding for a face
        // The library expects BGR image
        cv::resize(face_image, face_image, cv::Size(160, 160));

        // We have to manually allocate memory for the method
        float* result = new float[SIZE_OF_IEFACENET_V1];
        int result_size;
        status = compute_embedding(
            face_image.rows, face_image.cols,
            face_image.type(), face_image.step,
            face_image.data,
            face_image.total() * face_image.elemSize(),
            result, result_size
        );

        if (status == EXIT_FAILURE) {
            std::cout << receive_error() << std::endl;
            std::exit(EXIT_FAILURE);
        } else {
            std::vector<float> reference(result, result + SIZE_OF_IEFACENET_V1);
            people.insert(std::pair<std::string, std::vector<float>>(entry.path().filename(), reference));
        }
    }

    // Now run webcam stream
    while (true) {
        std::chrono::high_resolution_clock::time_point t1 =
            std::chrono::high_resolution_clock::now();

        // Get frame and detect faces
        capture >> image;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cascade.detectMultiScale(gray, faces, 1.5, 5, 0, cv::Size(150, 150));

        for (cv::Rect &face : faces) {
            // Get ROI
            face_image = image(face);

            // Get embedding
            cv::resize(face_image, face_image, cv::Size(160, 160));

            float* result = new float[SIZE_OF_IEFACENET_V1];
            int result_size;
            status = compute_embedding(
                face_image.rows, face_image.cols,
                face_image.type(), face_image.step,
                face_image.data,
                face_image.total() * face_image.elemSize(),
                result, result_size
            );

            if (status == EXIT_FAILURE) {
                std::cout << receive_error() << std::endl;
                std::exit(EXIT_FAILURE);
            } else {
                std::vector<float> result_v(result, result + SIZE_OF_IEFACENET_V1);
                cv::rectangle(image, face, cv::Scalar(255, 0, 255));

                // Find it's across saved people
                float minDistance = 100;
                std::string minKey;
                for (const std::pair<std::string, std::vector<float>> &pair : people) {
                    float distance;
                    status = compute_distance(
                        result_v.data(),
                        pair.second.data(),
                        result_v.size(),
                        distance
                    );

                    if (status == EXIT_FAILURE) {
                        std::cout << receive_error() << std::endl;
                        std::exit(EXIT_FAILURE);
                    } else {
                        if (distance < minDistance) {
                            minDistance = distance;
                            minKey = pair.first;
                        }
                    }
                }

                // Approximate threshold
                if (minDistance > 1) {
                    cv::putText(image, "unknown", cv::Point(face.tl()),
                        cv::FONT_HERSHEY_COMPLEX_SMALL, 1.5, cv::Scalar(0, 0, 255));
                } else {
                    std::string text =
                        minKey + std::string(": ") + std::to_string(minDistance);
                    cv::putText(image, text, cv::Point(face.tl()),
                        cv::FONT_HERSHEY_COMPLEX_SMALL, 1.5, cv::Scalar(0, 0, 255));
                }
            }

            delete[] result;
        }

        // Compute FPS
        std::chrono::high_resolution_clock::time_point t2 =
            std::chrono::high_resolution_clock::now();
        auto difference =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        cv::putText(image, std::to_string(1000 / difference), cv::Point(50, 50),
            cv::FONT_HERSHEY_COMPLEX_SMALL, 2.0, cv::Scalar(0, 255, 255)
        );


        imshow("frames", image);

        if (cv::waitKey(1) >= 0) {
            break;
        }
    }

    release_ie_facenet_v1();
}
