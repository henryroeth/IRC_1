#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <numeric>

#define PAM_SM_AUTH

extern "C" {

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        std::cerr << "Error loading face detection model!" << std::endl;
        return PAM_AUTH_ERR;
    }

    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();

    std::vector<cv::Mat> images;
    std::vector<int> labels;

    cv::Mat sampleImage = cv::imread("2024-03-04-131558.jpg", cv::IMREAD_GRAYSCALE);
    int sampleLabel = 1;

    images.push_back(sampleImage);
    labels.push_back(sampleLabel);

    recognizer->train(images, labels);

    dlib::shape_predictor landmarksPredictor;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> landmarksPredictor;

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error opening camera!" << std::endl;
        return PAM_AUTH_ERR;
    }

    cv::namedWindow("Face Recognition", cv::WINDOW_AUTOSIZE);

    for (int i = 5; i > 0; --i) {
        std::cout << "Hold still. Gathering data in " << i << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Gathering data..." << std::endl;

    auto startTime = std::chrono::steady_clock::now();
    std::vector<double> confidences;

    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(2)) {
        cv::Mat frame;
        cap >> frame;

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces);

        for (const auto& face : faces) {
            cv::Mat faceROI = gray(face);

            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);

            dlib::cv_image<dlib::bgr_pixel> dlibImage(frame);
            dlib::rectangle faceRect(face.x, face.y, face.x + face.width, face.y + face.height);
            dlib::full_object_detection landmarks = landmarksPredictor(dlibImage, faceRect);

            std::cout << "Face recognized: " << label << " (Confidence: " << 100 - confidence << ")" << std::endl;

            confidences.push_back(confidence);
        }

        cv::imshow("Face Recognition", frame);

        char key = cv::waitKey(10);
        if (key == 27) {
            break;
        }
    }

    cv::destroyAllWindows();
    cap.release();

    if (!confidences.empty()) {
        double averageConfidence = std::accumulate(confidences.begin(), confidences.end(), 0.0) / confidences.size();
        std::cout << "Average Confidence: " << 100 - averageConfidence << std::endl;

        if (averageConfidence < 40.0) {
            return PAM_SUCCESS;
        } else {
            return PAM_AUTH_ERR;
        }
    } else {
        std::cout << "No faces detected during the gathering period." << std::endl;
        return PAM_AUTH_ERR;
    }
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

} // extern "C"
