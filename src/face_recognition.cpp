#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <numeric>

int main() {
    // loads the pre-trained face detection model
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        std::cerr << "Error loading face detection model!" << std::endl;
        return 1;  // Exit with error code
    }

    // loads the pre-trained face recognition model
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();

    // loads the training data (replace with your own dataset)
    std::vector<cv::Mat> images;
    std::vector<int> labels;

    // adds sample data (replace this with your training dataset)
    cv::Mat sampleImage = cv::imread("2024-03-04-131558.jpg", cv::IMREAD_GRAYSCALE);
    int sampleLabel = 1;  // replace with the corresponding label for this image

    images.push_back(sampleImage);
    labels.push_back(sampleLabel);

    // trains the face recognition model
    recognizer->train(images, labels);

    // loads Dlib's facial landmarks predictor
    dlib::shape_predictor landmarksPredictor;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> landmarksPredictor;

    // opens the default camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error opening camera!" << std::endl;
        return 1; 
    }

    // creates a window
    cv::namedWindow("Face Recognition", cv::WINDOW_AUTOSIZE);

    // countdown for the user
    for (int i = 5; i > 0; --i) {
        std::cout << "Hold still. Gathering data in " << i << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Gathering data..." << std::endl;

    // record start time for a duration of 2 seconds
    auto startTime = std::chrono::steady_clock::now();
    std::vector<double> confidences;  // Store confidences for all recognized faces

    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(2)) {
        // reads a frame from the camera
        cv::Mat frame;
        cap >> frame;

        // converts the frame to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // detects faces in the frame
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces);

        // recognizes faces and draws dots on facial landmarks
        for (const auto& face : faces) {
            cv::Mat faceROI = gray(face);

            // performs face recognition
            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);

            // gets facial landmarks
            dlib::cv_image<dlib::bgr_pixel> dlibImage(frame);
            dlib::rectangle faceRect(face.x, face.y, face.x + face.width, face.y + face.height);
            dlib::full_object_detection landmarks = landmarksPredictor(dlibImage, faceRect);

            // displays individual face statistics
            std::cout << "Face recognized: " << label << " (Confidence: " << 100 - confidence << ")" << std::endl;

            // stores confidence levels for later analysis
            confidences.push_back(confidence);
        }

        // displays the frame
        cv::imshow("Face Recognition", frame);

        // checks for exit key press (ESC key)
        char key = cv::waitKey(10);
        if (key == 27) {  // ASCII code for ESC key
            break;
        }
    }

    // closes the window
    cv::destroyAllWindows();

    // releases the camera
    cap.release();

    // calculates and displays average confidence
    if (!confidences.empty()) {
        double averageConfidence = std::accumulate(confidences.begin(), confidences.end(), 0.0) / confidences.size();
        std::cout << "Average Confidence: " << 100 - averageConfidence << std::endl;

        if (averageConfidence < 40.0) {
            std::cout << "Authentication successful!" << std::endl;
        } else {
            std::cout << "Authentication failed!" << std::endl;
        }
    } else {
        std::cout << "No faces detected during the gathering period." << std::endl;
    }

    return 0; 
}
