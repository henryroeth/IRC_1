#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>

int main() {
    // Load the pre-trained face detection model
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cv::samples::findFile("haarcascade_frontalface_default.xml"))) {
        std::cerr << "Error loading face detection model!" << std::endl;
        return -1;
    }

    // Load the pre-trained face recognition model
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();

    // Load the training data (replace with your own dataset)
    std::vector<cv::Mat> images;
    std::vector<int> labels;

    // Add sample data (replace this with your training dataset)
    cv::Mat sampleImage = cv::imread("2024-03-04-131558.jpg", cv::IMREAD_GRAYSCALE);
    int sampleLabel = 1;  // replace with the corresponding label for this image

    images.push_back(sampleImage);
    labels.push_back(sampleLabel);

    // Train the face recognition model
    recognizer->train(images, labels);

    // Open the default camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error opening camera!" << std::endl;
        return -1;
    }

    // Create a window
    cv::namedWindow("Face Recognition", cv::WINDOW_AUTOSIZE);

    while (true) {
        // Read a frame from the camera
        cv::Mat frame;
        cap >> frame;

        // Convert the frame to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Detect faces in the frame
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces);

        // Recognize faces
        for (const auto& face : faces) {
            cv::Mat faceROI = gray(face);

            // Perform face recognition
            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);

            // Display results on the frame
            std::string result = (confidence < 50.0) ?
                                 "Face recognized: " + std::to_string(label) + " (Confidence: " + std::to_string(confidence) + ")" :
                                 "Unknown face (Confidence: " + std::to_string(confidence) + ")";
            
            cv::putText(frame, result, cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

            // Draw a rectangle around the detected face
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }

        // Display the frame
        cv::imshow("Face Recognition", frame);

        // Check for exit key press (ESC key)
        char key = cv::waitKey(10);
        if (key == 27) {  // ASCII code for ESC key
            break;
        }
    }

    // Release the camera and close the window
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
