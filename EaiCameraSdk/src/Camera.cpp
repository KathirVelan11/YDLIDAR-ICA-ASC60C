/**
 * @file      Camera.cpp
 * @brief     angstrong Camera.
 *
 * Copyright (c) 2023 Angstrong Tech.Co.,Ltd
 *
 * @author    Angstrong SDK develop Team
 * @date      2023/05/12
 * @version   1.0

 */
#include <algorithm>
#ifdef __linux__
#include <unistd.h>
#endif
#include "Camera.h"

Camera::Camera(AS_CAM_PTR pCamera, const AS_SDK_CAM_MODEL_E &cam_type)
{
    int ret = 0;
    m_handle = pCamera;
    m_cam_type = cam_type;
    m_check_fps = new CheckFps(pCamera);
    ret = AS_SDK_GetCameraAttrs(m_handle,  m_attr);
    if (ret != 0) {
        LOG(WARN) << "get camera attrs failed" << std::endl;
    }
    memset(&m_cam_parameter, 0, sizeof(AS_CAM_Parameter_s));
}

Camera::~Camera()
{
    if (m_check_fps != nullptr) {
        delete m_check_fps;
        m_check_fps = nullptr;
    }
    if (m_is_thread) {
        m_is_thread = false;
    }
    if (m_backgroundThread.joinable()) {
        m_backgroundThread.join();
    }
}

int Camera::init()
{
    int ret = 0;
    char sn_buff[64] = {0};
    ret = AS_SDK_GetSerialNumber(m_handle, sn_buff, sizeof(sn_buff));
    if (ret != 0) {
        LOG(ERROR) << "get camera serial number failed" << std::endl;
        return -1;
    }
    m_serialno = std::string(sn_buff);

    char fwVersion[100] = {0};
    ret = AS_SDK_GetFwVersion(m_handle, fwVersion, sizeof(fwVersion));
    if (ret == 0) {
        LOG(INFO) << "#camera[" << m_handle << "] SN[" << m_serialno << "]'s firmware version:" << fwVersion << std::endl;
    }
    m_is_thread = true;
    m_backgroundThread = std::thread(&Camera::backgroundThread, this);
    return ret;
}

double Camera::checkFps()
{
    std::string Info = "";
    switch (m_attr.type) {
    case AS_CAMERA_ATTR_LNX_USB:
        Info = (std::to_string(m_attr.attr.usbAttrs.bnum) + ":" + m_attr.attr.usbAttrs.port_numbers);
        break;
    case AS_CAMERA_ATTR_NET:
        Info = (std::to_string(m_attr.attr.netAttrs.port) + ":" + m_attr.attr.netAttrs.ip_addr);
        break;
    case AS_CAMERA_ATTR_WIN_USB:
        Info = std::string(m_attr.attr.winAttrs.symbol_link) + ":" + std::string(m_attr.attr.winAttrs.location_path);
        break;
    default:
        LOG(ERROR) << "attr type error" << std::endl;
        break;
    }
    return m_check_fps->checkFps(m_serialno, Info);
}

int Camera::enableSaveImage(bool enable)
{
    m_save_img = enable;
    if (m_cam_type == AS_SDK_CAM_MODEL_KUNLUN_A) {
        m_save_merge_img = enable;
    }
    return 0;
}

int Camera::enableDisplay(bool enable)
{
    m_display = enable;
    if (m_cam_type == AS_SDK_CAM_MODEL_KUNLUN_A) {
        m_display_merge = enable;
    }
    return 0;
}

int Camera::getSerialNo(std::string &sn)
{
    sn = m_serialno;
    return 0;
}

int Camera::getCameraAttrs(AS_CAM_ATTR_S &info)
{
    info = m_attr;
    return 0;
}

int Camera::backgroundThread()
{
    int ret = 0;
    while (m_is_thread) {
        // KONDYOR not support to get CamParameter, KUNLUN A don't need to get CamParameter
        if ((m_cam_type != AS_SDK_CAM_MODEL_KONDYOR_NET) && (m_cam_type != AS_SDK_CAM_MODEL_KONDYOR)) {
            ret = AS_SDK_GetCamParameter(m_handle, &m_cam_parameter);
            if (ret == 0) {
                LOG(INFO) << "SN [ " << m_serialno << " ]'s parameter:" << std::endl;
                LOG(INFO) << "irfx: " << m_cam_parameter.fxir << std::endl;
                LOG(INFO) << "irfy: " << m_cam_parameter.fyir << std::endl;
                LOG(INFO) << "ircx: " << m_cam_parameter.cxir << std::endl;
                LOG(INFO) << "ircy: " << m_cam_parameter.cyir << std::endl;
                LOG(INFO) << "rgbfx: " << m_cam_parameter.fxrgb << std::endl;
                LOG(INFO) << "rgbfy: " << m_cam_parameter.fyrgb << std::endl;
                LOG(INFO) << "rgbcx: " << m_cam_parameter.cxrgb << std::endl;
                LOG(INFO) << "rgbcy: " << m_cam_parameter.cyrgb << std::endl << std::endl;

                LOG(INFO) << "R00: " << m_cam_parameter.R00 << std::endl;
                LOG(INFO) << "R01: " << m_cam_parameter.R01 << std::endl;
                LOG(INFO) << "R02: " << m_cam_parameter.R02 << std::endl;
                LOG(INFO) << "R10: " << m_cam_parameter.R10 << std::endl;
                LOG(INFO) << "R11: " << m_cam_parameter.R11 << std::endl;
                LOG(INFO) << "R12: " << m_cam_parameter.R12 << std::endl;
                LOG(INFO) << "R20: " << m_cam_parameter.R20 << std::endl;
                LOG(INFO) << "R21: " << m_cam_parameter.R21 << std::endl;
                LOG(INFO) << "R22: " << m_cam_parameter.R22 << std::endl;
                LOG(INFO) << "T1: " << m_cam_parameter.T1 << std::endl;
                LOG(INFO) << "T2: " << m_cam_parameter.T2 << std::endl;
                LOG(INFO) << "T3: " << m_cam_parameter.T3 << std::endl << std::endl;
                m_is_thread = false;
                break;
            }
        } else {
            m_is_thread = false;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}

void Camera::saveImage(const AS_SDK_Data_s *pstData)
{
    if (!m_save_img) {
        m_cnt = 0;
        return;
    }

    if (m_cam_type == AS_SDK_CAM_MODEL_KUNLUN_A) {
        if (m_cnt >= 1) {
            m_save_img = false;
        }
        m_cnt++;
    } else {
        m_save_img = false;
    }

    // ── DEPTH ────────────────────────────────────────────────────────────────
    if (pstData->depthImg.size > 0) {
        cv::Mat depthImage;
        if (pstData->depthImg.size == pstData->depthImg.width * pstData->depthImg.height * 2) {
            depthImage = cv::Mat(pstData->depthImg.height, pstData->depthImg.width, CV_16UC1, pstData->depthImg.data);
        } else {
            depthImage = cv::Mat(pstData->depthImg.height, pstData->depthImg.width, CV_32FC1, pstData->depthImg.data);
        }

        // FIX: Save depth as a proper colorized (JET) image instead of a
        //      flat 8-bit grayscale, so it matches what is shown on screen.
        double minVal, maxVal;
        cv::minMaxIdx(depthImage, &minVal, &maxVal);

        cv::Mat depth8;
        if (maxVal > minVal)
            depthImage.convertTo(depth8, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        else
            depthImage.convertTo(depth8, CV_8UC1);

        cv::Mat depthColor;
        cv::applyColorMap(depth8, depthColor, cv::COLORMAP_JET);   // ← matches displayImage()

        std::string path = "C:/Users/ok/Desktop/IIT KGP/YDLIDAR-ICA-ASC60C/Output/Depth/";
        std::string filename = path + "depth_" + std::to_string(m_depthindex++) + ".png";
        cv::imwrite(filename, depthColor);
        LOG(INFO) << "Saved Depth: " << filename << std::endl;
    }

    // ── RGB ──────────────────────────────────────────────────────────────────
    if (pstData->rgbImg.size > 0) {
        // FIX: Camera outputs BGR natively (raw data displays correctly in
        // imshow without any conversion). The original COLOR_RGB2BGR swap was
        // erroneous -- it flipped R and B channels making saved images blue/cold.
        // Save the raw mat directly; cv::imwrite expects BGR which is what we have.
        cv::Mat rgbImage = cv::Mat(pstData->rgbImg.height, pstData->rgbImg.width, CV_8UC3, pstData->rgbImg.data);

        std::string path = "C:/Users/ok/Desktop/IIT KGP/YDLIDAR-ICA-ASC60C/Output/RGB/";
        std::string filename = path + "rgb_" + std::to_string(m_rgbindex++) + ".png";
        cv::imwrite(filename, rgbImage);
        LOG(INFO) << "Saved RGB: " << filename << std::endl;
    }

    if (pstData->yuyvImg.size > 0) {
        std::string yuyvName(std::string(m_serialno + "_yuyv_") + std::to_string(pstData->yuyvImg.width) + "x" +
                             std::to_string(pstData->yuyvImg.height) + "_" + std::to_string(m_yuyvindex++) + ".yuv");
        if (saveYUVImg(yuyvName.c_str(), pstData->yuyvImg.data, pstData->yuyvImg.size) != 0) {
            LOG(ERROR) << "save yuyv image failed!" << std::endl;
        } else {
            LOG(INFO) << "save yuyv image success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << yuyvName << std::endl;
#endif
        }
    }

    if (pstData->pointCloud.size > 0) {
        std::string path = "C:/Users/ok/Desktop/IIT KGP/YDLIDAR-ICA-ASC60C/Output/PointCloud/";
        std::string pointCloudName(path + std::string(m_serialno + "_PointCloud_"  + std::to_string(
                pstData->pointCloud.width) + "x" + std::to_string(pstData->pointCloud.height)
                                               + "_"  + std::to_string(m_pointCloudIndex++) + ".pcd"));
        if (savePointCloudWithPcdFormat(pointCloudName.c_str(), static_cast<float *>(pstData->pointCloud.data),
                                        pstData->pointCloud.size / sizeof(float)) != 0) {
            LOG(ERROR) << "save point cloud failed!" << std::endl;
        } else {
            LOG(INFO) << "save point cloud success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << pointCloudName << std::endl;
#endif
        }
    }

    if (pstData->irImg.size > 0) {
        std::string irimgName(std::string(m_serialno + "_ir_" + std::to_string(pstData->irImg.width) + "x" +
                                          std::to_string(pstData->irImg.height) + "_" + std::to_string(m_irindex++) + ".yuv"));
        if (saveYUVImg(irimgName.c_str(), pstData->irImg.data, pstData->irImg.size) != 0) {
            LOG(ERROR) << "save ir image failed!" << std::endl;
        } else {
            LOG(INFO) << "save ir image success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << irimgName << std::endl;
#endif
        }
    }

    if (pstData->peakImg.size > 0) {
        std::string peakimgName(std::string(m_serialno + "_peak_") + std::to_string(
                                    pstData->peakImg.width) + "x" + std::to_string(pstData->peakImg.height)
                                + "_" + std::to_string(m_peakindex++) + ".yuv");
        if (saveYUVImg(peakimgName.c_str(), pstData->peakImg.data, pstData->peakImg.size) != 0) {
            LOG(ERROR) << "save peak image failed!" << std::endl;
        } else {
            LOG(INFO) << "save peak image success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << peakimgName << std::endl;
#endif
        }
    }

    if (pstData->mjpegImg.size > 0) {
        std::string mjpegimgName(std::string(m_serialno + "_mjpeg_") + std::to_string(
                                     pstData->mjpegImg.width) + "x" + std::to_string(pstData->mjpegImg.height)
                                 + "_" + std::to_string(m_mjpegindex++) + ".jpg");
        if (saveYUVImg(mjpegimgName.c_str(), pstData->mjpegImg.data, pstData->mjpegImg.size) != 0) {
            LOG(ERROR) << "save mjpeg image failed!" << std::endl;
        } else {
            LOG(INFO) << "save mjpeg image success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << mjpegimgName << std::endl;
#endif
        }
    }

    return;
}

void Camera::saveMergeImage(const AS_SDK_MERGE_s *pstData)
{
    if (!m_save_merge_img) {
        return;
    }
    m_save_merge_img = false;
    if (pstData->depthImg.size > 0) {
        std::string depthimgName(std::string(m_serialno + "_depth_merge_") + std::to_string(
                                     pstData->depthImg.width) + "x" + std::to_string(pstData->depthImg.height)
                                 + "_" + std::to_string(m_depthindex++) + ".yuv");
        if (saveYUVImg(depthimgName.c_str(), pstData->depthImg.data, pstData->depthImg.size) != 0) {
            LOG(ERROR) << "save depth image failed!" << std::endl;
        } else {
            LOG(INFO) << "save depth image success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << depthimgName << std::endl;
#endif
        }
    }

    if (pstData->pointCloud.size > 0) {
        std::string pointCloudName(std::string(m_serialno + "_PointCloud_merge_"  + std::to_string(
                pstData->pointCloud.width) + "x" + std::to_string(pstData->pointCloud.height)
                                               + "_"  + std::to_string(m_pointCloudIndex++) + ".pcd"));
        if (savePointCloudWithPcdFormat(pointCloudName.c_str(), static_cast<float *>(pstData->pointCloud.data),
                                        pstData->pointCloud.size / sizeof(float)) != 0) {
            LOG(ERROR) << "save point cloud failed!" << std::endl;
        } else {
            LOG(INFO) << "save point cloud success!" << std::endl;
#ifdef __linux__
            LOG(INFO) << "location: " << getcwd(nullptr, 0) << "/" << pointCloudName << std::endl;
#endif
        }
    }

    return;
}

void Camera::displayImage(const std::string &serialno, const std::string &info, const AS_SDK_Data_s *pstData)
{
#ifdef CFG_OPENCV_ON
    if (m_display) {
        if (pstData->irImg.size > 0) {
            cv::Mat IrImage = cv::Mat(pstData->irImg.height, pstData->irImg.width, CV_8UC1,
                                      pstData->irImg.data);
            cv::imshow(serialno + info + "_ir_" + std::to_string(pstData->irImg.width) + "x" + std::to_string(
                           pstData->irImg.height), IrImage);
        }

        if (pstData->depthImg.size > 0) {
            cv::Mat depthImage;
            if (pstData->depthImg.size == pstData->depthImg.width * pstData->depthImg.height * 2) {
                depthImage = cv::Mat(pstData->depthImg.height, pstData->depthImg.width, CV_16UC1,
                                     pstData->depthImg.data);
            } else {
                depthImage = cv::Mat(pstData->depthImg.height, pstData->depthImg.width, CV_32FC1, pstData->depthImg.data);
            }
            cv::Mat depth_img_pseudo_color;

            double minVal;
            double maxVal;
            cv::minMaxIdx(depthImage, &minVal, &maxVal);
            depth2color(depth_img_pseudo_color, depthImage, maxVal, minVal);
            cv::imshow(serialno + info + "_depth_" + std::to_string(pstData->depthImg.width) + "x" + std::to_string(
                           pstData->depthImg.height), depth_img_pseudo_color);
        }

        if (pstData->rgbImg.size > 0) {
            // Camera outputs BGR natively — pass raw data directly to imshow.
            cv::Mat rgbImage = cv::Mat(pstData->rgbImg.height, pstData->rgbImg.width, CV_8UC3,
                                       pstData->rgbImg.data);
            cv::imshow(serialno + info + "_rgb_" + std::to_string(pstData->rgbImg.width) + "x" + std::to_string(
                           pstData->rgbImg.height), rgbImage);

            // NOTE: Saving is handled exclusively in saveImage(). The duplicate
            // save block that was here has been removed to avoid double index
            // increments and conflicting writes.
        }

        if (pstData->yuyvImg.size > 0) {
            cv::Mat yuyv = cv::Mat(pstData->yuyvImg.height, pstData->yuyvImg.width, CV_8UC2,
                                   pstData->yuyvImg.data);
            cv::Mat yuyvImg = yuyv2bgr(yuyv);
            cv::imshow(serialno + "_yuyv_" + std::to_string(pstData->yuyvImg.width) + "x" + std::to_string(
                           pstData->yuyvImg.height), yuyvImg);
        }

        if (pstData->peakImg.size > 0) {
            cv::Mat peakImg = cv::Mat(pstData->peakImg.height, pstData->peakImg.width, CV_8UC1, pstData->peakImg.data);
            cv::imshow(serialno + info + "_peak_" + std::to_string(pstData->peakImg.width) + "x" + std::to_string(
                           pstData->peakImg.height), peakImg);
        }

        if (pstData->mjpegImg.size > 0) {
            std::vector<unsigned char> mjpegImgVec(static_cast<unsigned char *>(pstData->mjpegImg.data),
                                                   static_cast<unsigned char *>(pstData->mjpegImg.data) + pstData->mjpegImg.size);
            cv::Mat mjpegImg = cv::imdecode(mjpegImgVec, cv::IMREAD_COLOR);
            if ((m_cam_type == AS_SDK_CAM_MODEL_HP60C) || (m_cam_type == AS_SDK_CAM_MODEL_HP60CN)) {
                cv::flip(mjpegImg, mjpegImg, 0);
            }
            if (mjpegImg.empty()) {
                LOG(ERROR) << "Failed to decode MJPEG data." << std::endl;
            } else {
                cv::imshow(serialno + info + "_mjpeg_" + std::to_string(pstData->mjpegImg.width) + "x" + std::to_string(
                               pstData->mjpegImg.height), mjpegImg.clone());
            }
        }

        cv::waitKey(3);
    } else {
        cv::destroyAllWindows();
    }
#endif
    return;
}

void Camera::displayMergeImage(const std::string &serialno, const std::string &info, const AS_SDK_MERGE_s *pstData)
{
#ifdef CFG_OPENCV_ON
    if (m_display_merge) {

        if (pstData->depthImg.size > 0) {
            cv::Mat depthImage = cv::Mat(pstData->depthImg.height, pstData->depthImg.width, CV_16UC1,
                                         pstData->depthImg.data);
            cv::Mat depth_img_pseudo_color;

            double minVal;
            double maxVal;
            cv::minMaxIdx(depthImage, &minVal, &maxVal);
            depth2color(depth_img_pseudo_color, depthImage, maxVal, minVal);
            cv::imshow(serialno + "_" + info + "_depth_merge_" + std::to_string(pstData->depthImg.width) + "x" + std::to_string(
                           pstData->depthImg.height), depth_img_pseudo_color);
        }

        cv::waitKey(3);
    } else {
        cv::destroyAllWindows();
    }
#endif
    return;
}

#ifdef CFG_OPENCV_ON
void Camera::depth2color(cv::Mat &color, const cv::Mat &depth, const double max, const double min)
{
    cv::Mat grayImage;
    double alpha = 255.0 / (max - min);
    depth.convertTo(grayImage, CV_8UC1, alpha, -alpha * min);
    cv::applyColorMap(grayImage, color, cv::COLORMAP_JET);
}

cv::Mat Camera::yuyv2bgr(const cv::Mat &yuyv)
{
    CV_Assert(yuyv.type() == CV_8UC2);
    cv::Mat bgr;
    cv::cvtColor(yuyv, bgr, cv::COLOR_YUV2BGR_YUYV);
    return bgr;
}
#endif

bool Camera::getDisplayStatus()
{
    if (m_cam_type == AS_SDK_CAM_MODEL_KUNLUN_A) {
        if ((m_display == false) && (m_display_merge == false)) {
            return false;
        } else if ((m_display == true) && (m_display_merge == true)) {
            return true;
        } else {
            return true;
        }
    } else {
        return m_display;
    }

}

void Camera::YV16toBGR(unsigned char *yv16Data, unsigned char *bgrData, unsigned int width, unsigned int height)
{
    int uv_offset = width * height;
    int y, u, v, r, g, b;

    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            y = yv16Data[i * width + j];
            u = yv16Data[uv_offset + (i / 2) * (width / 2) + j / 2];
            v = yv16Data[uv_offset + (height / 2) * (width / 2) + (i / 2) * (width / 2) + j / 2];

            r = y + 1.402 * (v - 128);
            g = y - 0.344136 * (u - 128) - 0.714136 * (v - 128);
            b = y + 1.772 * (u - 128);

            r = std::min(255, std::max(0, r));
            g = std::min(255, std::max(0, g));
            b = std::min(255, std::max(0, b));

            bgrData[(i * width + j) * 3] = b;
            bgrData[(i * width + j) * 3 + 1] = g;
            bgrData[(i * width + j) * 3 + 2] = r;
        }
    }
}