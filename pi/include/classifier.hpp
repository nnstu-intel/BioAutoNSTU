#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include <vector>
#include <opencv2/core/mat.hpp>

#include "macros_defs.h"

// Supported classificators list
enum ClassifierType {
    IE_Facenet_V1,
};

typedef std::vector<float> FaceDescriptor;

// Interface of a classificator
class Classifier {
    public:
        virtual float distance(const FaceDescriptor& desc1, const FaceDescriptor& desc2) = 0;
        virtual FaceDescriptor embed(const cv::Mat& face) = 0;
        virtual ~Classifier() {}
};

// Classificator factory function
API std::shared_ptr<Classifier> build_classifier(ClassifierType type, const std::string xml, const std::string bin, const std::string device);

#endif