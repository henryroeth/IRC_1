#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <pwd.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

// Function to retrieve the home directory of a user based on the username
std::string get_home_directory(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw) {
        return pw->pw_dir;
    } else {
        return "";
    }
}

// Function to check if a camera is available (using OpenCV VideoCapture)
bool is_camera_available() {
    cv::VideoCapture cap(0);  // Open default camera (camera index 0)
    if (!cap.isOpened()) {
        return false;
    }
    return true;
}

// Placeholder logic for confidence level calculation
double calculate_confidence_level(const std::vector<cv::Rect>& faces) {
    if (faces.empty()) {
        return 0.0;
    } else {
        double totalArea = 0.0;
        for (const auto& face : faces) {
            totalArea += face.width * face.height;
        }
        double averageArea = totalArea / faces.size();
        return std::min(1.0, averageArea / 5000.0);  // Normalize to a range between 0 and 1
    }
}

// Function to perform facial recognition
void perform_facial_recognition(const char *username) {
    // Check if a camera is available
    if (is_camera_available()) {
        // Get the home directory of the user
        std::string homeDirectory = get_home_directory(username);
        if (homeDirectory.empty()) {
            std::cerr << "Error retrieving home directory for user " << username << "!" << std::endl;
            return;
        }

        // Specify the paths for the image and Haar cascade classifier
        std::string imagePath = homeDirectory + "/security_image.jpg";
        std::string cascadePath = homeDirectory + "/haarcascade_frontalface_default.xml";

        // Print paths for debugging purposes
        std::cout << "Image path: " << imagePath << std::endl;
        std::cout << "Cascade path: " << cascadePath << std::endl;

        // Load the image and Haar cascade classifier
        cv::Mat image = cv::imread(imagePath);
        cv::CascadeClassifier faceCascade;

        // Check if the image and cascade model were loaded successfully
        if (image.empty()) {
            std::cerr << "Error loading image!" << std::endl;
            return;
        }

        if (!faceCascade.load(cascadePath)) {
            std::cerr << "Error loading face detection model!" << std::endl;
            return;
        }

        // Convert the image to grayscale for face detection
        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

        // Detect faces using the Haar cascade
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayImage, faces);

        // Print the number of faces found
        std::cout << "Number of faces detected: " << faces.size() << std::endl;

        // Display countdown message
        std::cout << "Hold still! Time remaining: 10 seconds" << std::endl;
        sleep(10);  // Sleep for 10 seconds

        // Calculate and print facial recognition statistics
        double confidenceLevel = calculate_confidence_level(faces);
        if (!faces.empty()) {
            std::cout << "Facial recognition statistics for user " << username << ":" << std::endl;
            std::cout << " - Confidence level: " << std::fixed << std::setprecision(2) << confidenceLevel * 100.0 << "%" << std::endl;
        } else {
            std::cout << "No faces detected." << std::endl;
        }
    } else {
        // No camera detected, print a message
        std::cout << "No camera detected! Authentication without facial recognition." << std::endl;
    }
}

// PAM authentication function
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *username;
    int pam_err = pam_get_user(pamh, &username, "Username: ");

    if (pam_err != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    // Perform facial recognition without displaying graphics
    perform_facial_recognition(username);

    // Check if facial recognition failed (no camera detected)
    if (!is_camera_available()) {
        return PAM_AUTH_ERR;  // Authentication failure
    }

    return PAM_SUCCESS;  // Authentication success
}

// PAM credential setting function
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

// PAM account management function
PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

// PAM session opening function
PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

// PAM session closing function
PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
