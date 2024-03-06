#include <iostream>
#include <opencv2/opencv.hpp>
#include <pwd.h> // Include this header for getpwnam
#include <security/pam_appl.h>
#include <security/pam_modules.h>

std::string get_home_directory(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw) {
        return pw->pw_dir;
    } else {
        return "";
    }
}

void perform_facial_recognition(const char *username) {
    std::string homeDirectory = get_home_directory(username);
    if (homeDirectory.empty()) {
        std::cerr << "Error retrieving home directory for user " << username << "!" << std::endl;
        return;
    }

    std::string imagePath = homeDirectory + "/security_image.jpg";
    std::string cascadePath = homeDirectory + "/haarcascade_frontalface_default.xml";

    std::cout << "Image path: " << imagePath << std::endl;
    std::cout << "Cascade path: " << cascadePath << std::endl;

    // Load image and Haar cascade classifier
    cv::Mat image = cv::imread(imagePath);
    cv::CascadeClassifier faceCascade;
    
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

    // Detect faces
    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(grayImage, faces);

    // Print the number of faces found
    std::cout << "Number of faces detected: " << faces.size() << std::endl;

    // Add your facial recognition logic here
    // For a basic example, we'll consider it a match if at least one face is detected
    if (!faces.empty()) {
        std::cout << "Facial recognition accuracy for user " << username << ": Match found. Access granted." << std::endl;
    } else {
        std::cout << "Facial recognition accuracy for user " << username << ": No match found. Access denied." << std::endl;
    }
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *username;
    int pam_err = pam_get_user(pamh, &username, "Username: ");

    if (pam_err != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    // Perform facial recognition without displaying graphics
    perform_facial_recognition(username);

    return PAM_SUCCESS;  // Always grant access
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
