import numpy as np, cv2, os, glob, time

watch_dir = "/home/mamba/vision/linux_ros/linux/build"
out_dir = "/home/mamba/vision/output"
os.makedirs(out_dir, exist_ok=True)
seen = set()

print("Watching for new frames. Press Ctrl+C to stop.")
while True:
    for f in sorted(glob.glob(os.path.join(watch_dir, "*_rgb_*.yuv"))):
        if f not in seen:
            seen.add(f)
            raw = open(f, "rb").read()
            bgr = np.frombuffer(raw, dtype=np.uint8).reshape((480, 640, 3))
            name = os.path.basename(f).replace(".yuv", ".jpg")
            cv2.imwrite(os.path.join(out_dir, name), bgr, [cv2.IMWRITE_JPEG_QUALITY, 95])
            print(f"Converted: {name}")
    time.sleep(0.5)
