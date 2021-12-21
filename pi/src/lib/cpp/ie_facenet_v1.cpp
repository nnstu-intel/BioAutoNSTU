/*
    Author: Boris Sekachev
    Email: b.sekachev@yandex.ru
    Organization: NNTU
    Year: 2019
*/

#include <string>
#include <opencv2/imgproc/imgproc.hpp>

#include "ie_facenet_v1.hpp"

IEFacenet_V1::IEFacenet_V1(const std::string xml, const std::string bin, const std::string device) {
    using namespace InferenceEngine; 

    Core ie;
    // Reading a network
    CNNNetReader networkReader;
    networkReader.ReadNetwork(xml);
    networkReader.ReadWeights(bin);

    this->_network = networkReader.getNetwork();
    this->_network.setBatchSize(1);

    // Get information about topology
    InputsDataMap inputInfo(this->_network.getInputsInfo());
    OutputsDataMap outputInfo(this->_network.getOutputsInfo());

    this->_executable = ie.LoadNetwork(this->_network, device);
    this->_infer_request = this->_executable.CreateInferRequest();
    this->_input = this->_infer_request.GetBlob((*inputInfo.begin()).first);
    this->_output = this->_infer_request.GetBlob((*outputInfo.begin()).first);
};

FaceDescriptor IEFacenet_V1::embed(const cv::Mat& face) {
    using namespace InferenceEngine;

    const cv::Size expectedImageSize = cv::Size(160, 160);
    if (face.size() != expectedImageSize) {
        cv::resize(face, face, expectedImageSize);
    }

    cv::Mat floatFace;
    cv::cvtColor(face, floatFace, cv::COLOR_BGR2RGB);
    floatFace.convertTo(floatFace, CV_32FC3);

    // Prepare data
    auto data =
        this->_input->buffer().as<float*>();
    size_t num_channels = this->_input->getTensorDesc().getDims()[1];
    size_t width = this->_input->getTensorDesc().getDims()[2];
    size_t height = this->_input->getTensorDesc().getDims()[3];
    size_t image_size = width * height;

    for (size_t h = 0; h < height; h++) {
        for (size_t w = 0; w < width; w++) {
            for (size_t ch = 0; ch < num_channels; ch++) {
                data[ch * image_size + h * width + w] =
                    (floatFace.at<cv::Vec3f>(h, w)[num_channels + ch] - 127.5) / 128.0;
            }
        }
    }

    this->_infer_request.Infer();

    // get output
    const auto output_data = this->_output->buffer().as<float *>();
    size_t descriptor_size = this->_output->getTensorDesc().getDims().at(1);

    std::vector<float> result;
    result.reserve(descriptor_size);

    for (size_t id = 0; id < descriptor_size; id++) {
        result.push_back(output_data[id]);
    }

    return std::move(result);
};

float IEFacenet_V1::distance(const FaceDescriptor& desc1, const FaceDescriptor& desc2) {
    if (desc1.size() != desc2.size()) {
        throw std::invalid_argument("Both vectors must have the same size");
    }

    const size_t size = desc1.size();
    float dot = 0;
    float norm1 = 0;
    float norm2 = 0;

    for (size_t i = 0; i < size; i++) {
        dot += desc1[i] * desc2[i];
        norm1 += std::pow(desc1[i], 2);
        norm2 += std::pow(desc2[i], 2);
    }

    float similarity = dot / (std::sqrt(norm1) * std::sqrt(norm2));

    return std::acos(similarity);
};

IEFacenet_V1::~IEFacenet_V1() {
    // Reset executable network before plugin
    // There is segmentation fault if plugin had released before
    this->_executable.reset(nullptr);
}
