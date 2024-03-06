#include <iostream>
#include <opencv2/opencv.hpp>
#include <pwd.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

std::string get_home_directory(const char *username) {
    struct passwd *pw = getpwnam(username);
    return pw ? pw->pw_dir : "";
}

bool is_camera_available() {
    cv::VideoCapture cap(0);
    return cap.isOpened();
}

double calculate_confidence_level(const std::vector<cv::Rect>& faces) {
    if (faces.empty()) {
        return 0.0;
    } else {
        double totalArea = 0.0;
        for (const auto& face : faces) {
            totalArea += face.width * face.height;
        }
        double averageArea = totalArea / faces.size();
        return std::min(1.0, averageArea / 5000.0);
    }
}

void perform_facial_recognition(const char *username) {
    if (is_camera_available()) {
        std::string homeDirectory = get_home_directory(username);
        if (homeDirectory.empty()) {
            std::cerr << "Error retrieving home directory for user " << username << "!" << std::endl;
            return;
        }

        std::string imagePath = homeDirectory + "/security_image.jpg";
        std::string cascadePath = homeDirectory + "/haarcascade_frontalface_default.xml";

        cv::Mat image = cv::imread(imagePath);
        cv::CascadeClassifier faceCascade;

        if (image.empty() || !faceCascade.load(cascadePath)) {
            std::cerr << "Error loading image or face detection model!" << std::endl;
            return;
        }

        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayImage, faces);

        double confidenceLevel = calculate_confidence_level(faces);
        if (!faces.empty()) {
            std::cout << "Facial recognition statistics for user " << username << ":" << std::endl;
            std::cout << " - Confidence level: " << std::fixed << std::setprecision(2) << confidenceLevel * 100.0 << "%" << std::endl;
        } else {
            std::cout << "No faces detected." << std::endl;
        }
    } else {
        std::cout << "No camera detected! Authentication without facial recognition." << std::endl;
        exit(PAM_AUTH_ERR);
    }
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *username;
    int pam_err = pam_get_user(pamh, &username, "Username: ");

    if (pam_err != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    perform_facial_recognition(username);

    if (!is_camera_available()) {
        return PAM_AUTH_ERR;
    }

    return PAM_SUCCESS;
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
