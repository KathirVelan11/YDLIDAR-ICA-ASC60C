# YDLIDAR ICA ASC60C

![Camera box](images/Box.jpg)

> **Community setup and development guide for the YDLIDAR ICA ASC60C 3D depth camera.**

---

## 📌 A note on naming

| Physical label | Software identity |
|---------------|------------------|
| **ICA ASC60C** | **YDLIDAR HP60C** (model type 9) |

The device is labelled **"ICA ASC60C"** on its casing. However, all official software — the
EAIViewer GUI, the `ascamera.exe` command‑line tool, and the `EaiCameraSdk` — recognise it
as the **YDLIDAR HP60C**.

The two names refer to the **same 3D depth camera**.  
Throughout this guide we use **HP60C** whenever we talk about software, drivers, or tool
behaviour, and **ICA ASC60C** only when referring to the physical unit.

Official product page: [YDLIDAR HP60C](https://www.ydlidar.com/product/ydlidar-hp60c)

---

## 📷 Camera Specifications

| Parameter | Value |
|-----------|-------|
| **Model** | ICA ASC60C (detected as HP60C / model type 9) |
| **Technology** | Structured‑light 3D imaging |
| **Depth Resolution** | 640 × 480 @ 20 fps |
| **RGB Resolution** | 640 × 480 @ 20 fps (hardware supports up to 1920 × 1080) |
| **Range** | 0.2 – 4 m (optimal within 2 m) |
| **FOV** | 73.8° (H) × 58.8° (V) × 86.4° (D) |
| **RGB Format** | RGB888 (921 600 bytes per frame) |
| **Depth Format** | 16‑bit RAW (614 400 bytes per frame) |
| **Interface** | USB 2.0 (Type‑C), UVC protocol |
| **Power** | < 2 W |
| **Dimensions** | 89.8 × 19.0 × 25.0 mm |

![Physical device label](images/asc60c.jpeg)

---

## 📚 Documentation

Official PDFs shipped with the camera are available in the `docs/` folder:

| Document | File |
|----------|------|
| Data Sheet | [YDLIDAR HP60C Data Sheet.pdf](docs/YDLIDAR%20HP60C%20Data%20Sheet.pdf) |
| User Manual | [YDLDIAR HP60C User Manual.pdf](docs/YDLDIAR%20HP60C%20User%20Manual.pdf) |
| SDK & ROS Manual | [YDLIAR HP60C SDK ROS DEVELOPMENT AND USE MANUAL.pdf](docs/YDLIAR%20HP60C%20SDK%20ROS%20DEVELOPMENT%20AND%20USE%20MANUAL.pdf) |
| SDK Reference | [YDLIDAR HP60C-SDK.pdf](docs/YDLIDAR%20HP60C-SDK.pdf) |

They contain detailed technical parameters, mechanical drawings, and operating instructions
directly from the manufacturer.

---

## 🚀 Quick Start — Choose Your Platform

We provide **two complete setup guides** for different platforms:

### 🪟 Windows 11
- **HP60CClientViewer** — Official GUI (no build required)
- **EaiCameraSdk** — C++ command-line tool (build from source)
- **Python Frame Capture** — Automated livestream recording

📖 **Full guide:** [Windows-11/README.md](Windows-11/README.md)

**Hardware:** Any Windows 11 PC with USB 2.0 Type-C port

---

### 🐧 Ubuntu 24.04 Headless (Raspberry Pi 5)
- **Linux SDK** — Build and run on ARM64
- **Python Converter** — Auto-convert raw frames to JPEG
- **SSH capture** — No display needed

📖 **Full guide:** [Ubuntu-24.04-Headless/README.md](Ubuntu-24.04-Headless/README.md)

**Hardware:**
- Raspberry Pi 5 (8GB RAM)
- 32GB microSD card
- Ubuntu Server 24.04 LTS (or 22.04 for live preview)

---

## ⚠️ Important Notes

### RGB-Only Output
During testing, we collected **RGB frames only** as the primary output. Depth maps and point clouds are extracted from the SDK data but are secondary to the RGB stream.

### HP60CClientViewer is Windows-Only
The official GUI viewer (`AngstrongViewer.exe`) is compiled for Windows x86_64 with Qt DLLs that don't exist for ARM Linux. For Linux, use the command-line `ascamera` tool instead.

---

## 📁 Repository Structure

```
YDLIDAR-ICA-ASC60C/
├── Windows-11/                     ← Windows 11 setup guide & tools
│   ├── README.md                   (HP60CClientViewer + EaiCameraSdk)
│   ├── HP60CClientViewer/          (Official GUI, pre-compiled)
│   ├── EaiCameraSdk/               (C++ SDK, build from source)
│   ├── grab_frames.py              (Python livestream capture)
│   └── Output/                     (Saved frames)
│
├── Ubuntu-24.04-Headless/          ← Ubuntu/RPi5 setup guide & tools
│   ├── README.md                   (Linux SDK build & Python converter)
│   ├── convert_frames.py           (Auto-convert .yuv → .jpg)
│   ├── unpack_linux_ros.sh         (Unpacks Linux SDK)
│   └── output/                     (Converted JPEG frames)
│
├── docs/                           (Official YDLIDAR PDFs)
│   ├── YDLIDAR HP60C Data Sheet.pdf
│   ├── YDLDIAR HP60C User Manual.pdf
│   ├── YDLIAR HP60C SDK ROS DEVELOPMENT AND USE MANUAL.pdf
│   └── YDLIDAR HP60C-SDK.pdf
│
├── images/                         (Hardware photos & diagrams)
│   ├── Box.jpg
│   ├── asc60c.jpeg
│   ├── ascamera.png
│   ├── eaiviewer.png
│   └── rpi.jpeg
│
└── README.md                       (This file)
```

---

## Known Limitation: 640 × 480 Resolution Lock

Despite the datasheet stating a maximum RGB resolution of 1920 × 1080, the camera always streams at 640 × 480. This is a **firmware-level limitation** on both Windows 11 and Ubuntu 24.04 / Raspberry Pi 5.

See the detailed breakdown of attempts in:
- [Windows-11/README.md](Windows-11/README.md#known-limitation-640--480-resolution-lock) — Hex-patching experiments on Windows
- [Ubuntu-24.04-Headless/README.md](Ubuntu-24.04-Headless/README.md#known-issues) — Software-level attempts on Linux

If you have experience with USB video device firmware or UVC driver development, we'd love to collaborate. Open an issue or pull request!

---

## 📡 Official Source

All software and SDK files in this repository were obtained from the official YDLIDAR downloads page:

https://www.ydlidar.cn/download/category/tool-sdk-ros-lidarclient

They are organised and documented here for convenience and community usage.

---

## 🙏 Acknowledgements

- **Shenzhen EAI Technology Co., Ltd.** (ydlidar.com) — for the hardware, SDK, and EAIViewer software
- **Angstrong Tech** — for the underlying AngstrongCameraSdk
- **Raspberry Pi Foundation** — for the RPi 5 platform
- **The open-source community** — for CMake, OpenCV, Qt, and Python
