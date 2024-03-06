#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <pwd.h>

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

    cv::Mat image = cv::imread(imagePath);

    if (image.empty()) {
        std::cerr << "Error loading image!" << std::endl;
        return;
    }

    // Your facial recognition logic using OpenCV, e.g., detect faces
    std::vector<cv::Rect> faces;
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cascadePath)) {
        std::cerr << "Error loading face detection model!" << std::endl;
        return;
    }

    faceCascade.detectMultiScale(image, faces);

    // Display the image with detected faces
    cv::imshow("Facial Recognition Result for User " + std::string(username), image);
    cv::waitKey(10000);  // Display the frame for 10 seconds
    cv::destroyAllWindows();  // Close the OpenCV window

    // Print facial recognition accuracy
    std::cout << "Facial recognition accuracy for user " << username << ": Always granted access." << std::endl;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *username;
    int pam_err = pam_get_user(pamh, &username, "Username: ");

    if (pam_err != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    // Perform facial recognition with displaying a frame for 10 seconds
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
