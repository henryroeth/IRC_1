#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>

int main() {
    // loads the pre-trained face detection model
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cv::samples::findFile("haarcascade_frontalface_default.xml"))) {
        std::cerr << "Error loading face detection model!" << std::endl;
        return -1;
    }

    // loads the pre-trained face recognition model
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();

    // loads the training data 
    std::vector<cv::Mat> images;
    std::vector<int> labels;

    // adds sample data
    cv::Mat sampleImage = cv::imread("2024-03-04-131558.jpg", cv::IMREAD_GRAYSCALE);
    int sampleLabel = 1;  

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
        return -1;
    }

    // creates a window
    cv::namedWindow("Face Recognition", cv::WINDOW_AUTOSIZE);

    // main loop
    while (true) {
        // reads a frame from the camera
        cv::Mat frame;
        cap >> frame;

        // converts the frame to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // detects faces in the frame
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces);

        // recognizes faces and obtains confidence on facial landmarks
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

            // draws a rectangle around the detected face with confidence level
            std::string confidenceText = "Confidence: " + std::to_string(100 - confidence);
            cv::putText(frame, confidenceText, cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

            // draws dots on facial landmarks
            for (unsigned int i = 0; i < landmarks.num_parts(); ++i) {
                cv::circle(frame, cv::Point(landmarks.part(i).x(), landmarks.part(i).y()), 3, cv::Scalar(0, 0, 255), -1);
            }
        }

        // displays the frame
        cv::imshow("Face Recognition", frame);

        // checks for exit key press (ESC key)
        char key = cv::waitKey(10);
        if (key == 27) {  // ASCII code for ESC key
            break;
        }
    }

    // releases the camera and closes the window
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
