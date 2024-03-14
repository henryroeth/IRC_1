#include <security/pam_appl.h>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

using namespace cv;
using namespace cv::face;
using namespace std;

#define PAM_EXTERN extern "C"

// program settings
const int FRAME_WIDTH = 1000;
const int FRAME_HEIGHT = 800;
const int COUNTDOWN_SECONDS = 5;
const int CONFIDENCE_THRESHOLD = 30;

// function to perform facial recognition and calculate confidence levels
// returns a double for the confidence level
double performFacialRecognition(Mat& frame, Ptr<LBPHFaceRecognizer>& recognizer) {
    CascadeClassifier faceDetector;
    if (!faceDetector.load("/etc/pam_security/haarcascade_frontalface_alt.xml")) {
        cerr << "Error: Could not load face detector." << endl;
        return -1;
    }

    vector<Rect> faces;
    faceDetector.detectMultiScale(frame, faces);

    double totalConfidence = 0.0;
    int recognitionAttempts = 0;

    for (const Rect& face : faces) {
        Mat detectedFace = frame(face);

        int predictedLabel = -1;
        double confidence = 0.0;
        recognizer->predict(detectedFace, predictedLabel, confidence);

        totalConfidence += confidence;
        recognitionAttempts++;
    }

    return recognitionAttempts > 0 ? totalConfidence / recognitionAttempts : -1;
}

// PAM authentication function
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    // loads reference image(s)
    Mat referenceImage = imread("/etc/pam_security/reference_face.jpg", IMREAD_GRAYSCALE);
    if (referenceImage.empty()) {
        cerr << "Error: Could not load reference image." << endl;
        return PAM_AUTH_ERR;
    }

    vector<Mat> referenceImages = { referenceImage };

    // creates LBPH recognizer and trains with reference image(s)
    Ptr<LBPHFaceRecognizer> recognizer = LBPHFaceRecognizer::create();
    recognizer->train(referenceImages, vector<int>(referenceImages.size(), 0));

    // opens the camera and catches any hardware exceptions
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open camera." << endl;
        return PAM_AUTH_ERR;
    }

    // counts down and notifies the user to hold still for recording
    for (int i = COUNTDOWN_SECONDS; i > 0; i--) {
        cout << "Please hold still. Recording will begin in " << i << " seconds." << endl;
        std::this_thread::sleep_for(chrono::seconds(1));
    }

    // records for 5 seconds and collects confidence levels
    auto start = chrono::steady_clock::now();
    int frameCount = 0;
    double totalConfidence = 0.0;

    while (chrono::steady_clock::now() - start < chrono::seconds(5)) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            cerr << "Error: Failed to capture frame." << endl;
            return PAM_AUTH_ERR;
        }

        // converts the frame to grayscale for image analysis
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // performs facial recognition on the frame
        double confidence = performFacialRecognition(grayFrame, recognizer);
        if (confidence < 0) {
            cerr << "Error: Facial recognition failed." << endl;
            return PAM_AUTH_ERR;
        }

        totalConfidence += confidence;
        frameCount++;
    }

    // calculates the average confidence
    double averageConfidence = totalConfidence / frameCount;
    cout << "Average confidence: " << averageConfidence << endl;

    // authenticate the user based on the average confidence level
    if (averageConfidence < CONFIDENCE_THRESHOLD) {
        return PAM_SUCCESS; 
    } else {
        return PAM_AUTH_ERR; 
    }
}

// PAM function for setting credentials 
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS; 
}

// PAM function for opening the session
PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

// PAM fucntion for closing/cleaning up the session
PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
