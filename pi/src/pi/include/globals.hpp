#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <thread>

#include <opencv2/objdetect/objdetect.hpp>

#include <config.hpp>
#include <users.hpp>
#include <classifier.hpp>

extern PIConfiguration global_pi_configuration;
extern std::vector<User> global_pi_users;
extern std::shared_ptr<Classifier> global_pi_classifier;
extern cv::CascadeClassifier global_pi_face_detector;
extern std::mutex global_pi_users_mutex;
extern std::mutex global_pi_classifier_mutex;
extern std::mutex global_pi_face_detector_mutex;

#endif