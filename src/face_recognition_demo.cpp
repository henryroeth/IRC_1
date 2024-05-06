#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <vector>
#include <filesystem> // Add this header for directory_iterator
#include <string>

using namespace cv;
using namespace cv::face;
using namespace std;
namespace fs = std::filesystem;

const string CASCADE_PATH = "/home/henry/Projects/IRC_1/data/haarcascade_frontalface_default.xml";
const string REFERENCE_FACES_DIR = "/home/henry/Projects/IRC_1/data/reference_faces";

const int MATCH_THRESHOLD = 30; // Adjust this value as needed

int main() {
    // Load Haar Cascade Classifier
    CascadeClassifier faceCascade;
    if (!faceCascade.load(CASCADE_PATH)) {
        cerr << "Error loading face cascade.\n";
        return -1;
    }

    // Load LBPH model and train with reference faces
    Ptr<LBPHFaceRecognizer> recognizer = LBPHFaceRecognizer::create();
    vector<Mat> images;
    vector<int> labels;

    // Read reference faces from directory
    try {
        // Load images and labels directly without using read_csv
        for (const auto& entry : fs::directory_iterator(REFERENCE_FACES_DIR)) {
            if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg") {
                Mat img = imread(entry.path().string(), IMREAD_GRAYSCALE);
                images.push_back(img);
                // Get label from filename by parsing
                string filename = entry.path().filename().stem();
                size_t pos = filename.find_last_of("_");
                int label = stoi(filename.substr(pos + 1));
                labels.push_back(label);
            }
        }
        
        // Convert labels to the correct format (CV_32SC1)
        Mat labelsMat(labels.size(), 1, CV_32SC1);
        for (int i = 0; i < labels.size(); ++i) {
            labelsMat.at<int>(i, 0) = labels[i];
        }

        recognizer->train(images, labelsMat);
    } catch (const cv::Exception& e) {
        cerr << "Error training LBPH model: " << e.what() << endl;
        return -1;
    }

    // Open camera
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "Error opening camera.\n";
        return -1;
    }

    Mat frame;
    int totalFaces = 0;
    int matchedFaces = 0;
    while (true) {
        capture >> frame;
        if (frame.empty()) {
            cerr << "No frame captured.\n";
            break;
        }

        // Convert frame to grayscale
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);

        // Detect faces using Haar Cascade Classifier
        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces);

        totalFaces = faces.size();
        matchedFaces = 0;

        for (const Rect& faceRect : faces) {
            // Extract face region for recognition
            Mat faceROI = grayFrame(faceRect);

            // Perform face recognition
            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);

            // Display recognition result
            string labelString = (confidence <= MATCH_THRESHOLD) ? "Match" : "No Match";

            if (confidence <= MATCH_THRESHOLD) {
                matchedFaces++;
            }

            Scalar color = (confidence <= MATCH_THRESHOLD) ? Scalar(0, 255, 0) : Scalar(0, 0, 255);

            // Draw rectangle around the face with matching color
            rectangle(frame, faceRect, color, 2);

            // Display label string
            putText(frame, labelString, Point(faceRect.x, faceRect.y - 5), FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
        }

        // Display frame
        string info = "Total Faces: " + to_string(totalFaces) + "   Matched Faces: " + to_string(matchedFaces);
        
        // Draw yellow box behind the text
        int baseline = 0;
        Size textSize = getTextSize(info, FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);
        rectangle(frame, Point(5, frame.rows - textSize.height - 10), Point(5 + textSize.width + 10, frame.rows - 5), Scalar(0, 255, 255), FILLED);
        putText(frame, info, Point(10, frame.rows - 10), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 255), 2);

        // Show frame
        imshow("Face Recognition", frame);

        // Check for ESC key press
        char c = (char)waitKey(10);
        if (c == 27) {
            break;
        }
    }

    capture.release();
    destroyAllWindows();
    return 0;
}
