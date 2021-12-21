/*
    Author: Boris Sekachev
    Email: b.sekachev@yandex.ru
    Organization: NNTU
    Year: 2019
*/

#include <memory>

#include "cwrapper.h"
#include "classifier.hpp"

std::shared_ptr<Classifier> classifier;
const char* exception_message = NULL;

EXTERN_C
    const char* receive_error() {
        const char* tmp = exception_message;
        exception_message = NULL;
        return tmp;
    }

    int init_ie_facenet_v1(const char* xml, const char* bin, const char* device) {
        // Do not pass any exceptions in C written application
        try {
            if (classifier) {
                throw new std::runtime_error("Classifier has already been initialized");
            }

            classifier = build_classifier(ClassifierType::IE_Facenet_V1, std::string(xml), std::string(bin), std::string(device));
        } catch(const std::exception& exception) {
            exception_message = exception.what();
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    int release_ie_facenet_v1() {
        if (classifier) {
            try {
                classifier.reset();
            } catch(const std::exception& exception) {
                exception_message = exception.what();
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }

    int compute_distance(
        const float* dist1,
        const float* dist2,
        const int size,
        float& result
    ) {
        try {
            if (!classifier) {
                throw new std::runtime_error("Classifier hasn't been initialized yet");
            }

            result = classifier->distance(
                FaceDescriptor(dist1, dist1 + size),
                FaceDescriptor(dist2, dist2 + size)
            );
        } catch(const std::exception& exception) {
            exception_message = exception.what();
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    int compute_embedding(
        const int height,
        const int width,
        const int type,
        const int step,
        const u_char* data,
        const int data_size,
        float *const result,
        int& result_size
    ) {
        try {
            if (!classifier) {
                throw new std::runtime_error("Classifier hasn't been initialized yet");
            }

            cv::Mat face(height, width, type, (void*)data, step);
            const FaceDescriptor desc = classifier->embed(face);
            memcpy(result, desc.data(), sizeof(float) * desc.size());
            result_size = desc.size();
        } catch(const std::exception& exception) {
            exception_message = exception.what();
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
EXTERN_C_END