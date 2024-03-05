#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main() {
    // Load the pre-trained face detection model
    CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        printf("Error loading face cascade.\n");
        return -1;
    }

    // Open a video capture object (assumes default camera)
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        printf("Error opening the camera.\n");
        return -1;
    }

    namedWindow("Face Detection", WINDOW_NORMAL);

    while (true) {
        Mat frame;
        capture >> frame; // Capture a frame from the camera

        // Convert the frame to grayscale for face detection
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Detect faces
        std::vector<Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.3, 5);

        // Draw rectangles around detected faces
        for (const auto& face : faces) {
            rectangle(frame, face, Scalar(255, 0, 0), 2);
        }

        // Display the frame with detected faces
        imshow("Face Detection", frame);

        // Break the loop if 'ESC' key is pressed
        if (waitKey(30) == 27) {
            break;
        }
    }

    // Release the video capture object
    capture.release();

    // Destroy the OpenCV windows
    destroyAllWindows();

    return 0;
}
