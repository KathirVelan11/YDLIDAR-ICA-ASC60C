/**
 * @file      Demo.h
 * @brief     angstrong camera service header.
 *
 * Copyright (c) 2023 Angstrong Tech.Co.,Ltd
 *
 * @author    Angstrong SDK develop Team
 * @date      2023/05/12
 * @version   1.0

 */
#pragma once
#include <list>
#include <functional>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>
#include "as_camera_sdk_api.h"
#include "as_camera_sdk_def.h"
#include "Logger.h"

#include "CameraSrv.h"
#include "Camera.h"

class Demo : public ICameraStatus
{
public:
    Demo();
    ~Demo();

public:
    int start();
    void stop();
    void saveImage();
    void display(bool enable);
    bool getDisplayStatus();
    void logFps(bool enable);
    bool getLogFps();
    void logCfgParameter();

private:
    virtual int onCameraAttached(AS_CAM_PTR pCamera, const AS_SDK_CAM_MODEL_E &cam_type) override;
    virtual int onCameraDetached(AS_CAM_PTR pCamera) override;
    virtual int onCameraOpen(AS_CAM_PTR pCamera) override;
    virtual int onCameraClose(AS_CAM_PTR pCamera) override;
    virtual int onCameraStart(AS_CAM_PTR pCamera) override;
    virtual int onCameraStop(AS_CAM_PTR pCamera) override;
    virtual void onCameraNewFrame(AS_CAM_PTR pCamera, const AS_SDK_Data_s *pstData) override;
    virtual void onCameraNewMergeFrame(AS_CAM_PTR pCamera, const AS_SDK_MERGE_s *pstData) override;

#ifdef __linux__
    bool virtualMachine();
#endif

private:
    CameraSrv *server = nullptr;
    /* log the average frame rate */
    bool m_logfps = false;
    std::unordered_map<AS_CAM_PTR, std::shared_ptr<Camera>> m_camera_map;
};
