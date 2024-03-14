#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>

using namespace cv;
using namespace cv::face;
using namespace std;

int main() {
    Mat referenceImage = imread("reference_face.jpg", IMREAD_GRAYSCALE);
    if (referenceImage.empty()) {
        cerr << "Error: Could not load reference image." << endl;
        return -1;
    }

    Ptr<LBPHFaceRecognizer> recognizer = LBPHFaceRecognizer::create();
    vector<Mat> images;
    images.push_back(referenceImage);
    vector<int> labels;
    labels.push_back(0); 
    recognizer->train(images, labels);

    CascadeClassifier faceDetector;
    if (!faceDetector.load("haarcascade_frontalface_alt.xml")) {
        cerr << "Error: Could not load face detector." << endl;
        return -1;
    }

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "Error: Unable to access camera." << endl;
        return -1;
    }

    namedWindow("Camera Feed", WINDOW_AUTOSIZE);

    while (true) {
        Mat frame;
        capture >> frame;

        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        vector<Rect> faces;
        faceDetector.detectMultiScale(grayFrame, faces);

        for (const Rect& face : faces) {
            rectangle(frame, face, Scalar(255, 0, 0), 2);

            Mat detectedFace = grayFrame(face);

            int predictedLabel = -1;
            double confidence = 0.0;
            recognizer->predict(detectedFace, predictedLabel, confidence);

            string label;
            if (predictedLabel == 0) {
                label = "Reference Face";
            } else {
                label = "Unknown";
            }

            stringstream confidenceText;
            confidenceText << "Confidence: " << confidence;
            putText(frame, label, Point(face.x, face.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
            putText(frame, confidenceText.str(), Point(face.x, face.y + face.height + 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
        }

        imshow("Camera Feed", frame);

        if (waitKey(30) == 27) {
            break;
        }
    }

    capture.release();
    destroyAllWindows();

    return 0;
}
