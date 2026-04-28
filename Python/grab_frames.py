import cv2
import numpy as np
import win32gui
import win32ui
import win32con
import time
import os

class LiveFrameGrabber:
    def __init__(self, window_title_contains="_rgb_"):
        self.window_title = window_title_contains
        self.hwnd = None
        self.frame_count = 0
        self.target_fps = 60
        self.frame_interval = 1.0 / self.target_fps
        self.client_size = None
        
    def find_window(self):
        def callback(hwnd, windows):
            title = win32gui.GetWindowText(hwnd)
            if self.window_title in title:
                windows.append(hwnd)
            return True
        
        windows = []
        win32gui.EnumWindows(callback, windows)
        if windows:
            self.hwnd = windows[0]
            return True
        return False
    
    def get_client_size(self):
        """Get the client area size (just the image, no borders/title)"""
        if self.hwnd:
            rect = win32gui.GetClientRect(self.hwnd)
            return rect[2], rect[3]  # width, height
        return 640, 480
    
    def grab_frame(self):
        if self.hwnd is None:
            if not self.find_window():
                return None
        
        # Check if window is still valid
        if not win32gui.IsWindow(self.hwnd):
            self.hwnd = None
            return None
        
        # Get client rect (just the image area, no borders)
        left, top, right, bottom = win32gui.GetClientRect(self.hwnd)
        width = right - left
        height = bottom - top
        
        # Skip if window is minimized or zero size
        if width <= 0 or height <= 0:
            return None
        
        hwndDC = win32gui.GetWindowDC(self.hwnd)
        mfcDC = win32ui.CreateDCFromHandle(hwndDC)
        saveDC = mfcDC.CreateCompatibleDC()
        
        bitmap = win32ui.CreateBitmap()
        bitmap.CreateCompatibleBitmap(mfcDC, width, height)
        saveDC.SelectObject(bitmap)
        
        saveDC.BitBlt((0, 0), (width, height), mfcDC, (0, 0), win32con.SRCCOPY)
        
        bmpstr = bitmap.GetBitmapBits(True)
        img = np.frombuffer(bmpstr, dtype=np.uint8).copy()
        
        win32gui.DeleteObject(bitmap.GetHandle())
        saveDC.DeleteDC()
        mfcDC.DeleteDC()
        win32gui.ReleaseDC(self.hwnd, hwndDC)
        
        if img.size == 0:
            return None
        
        img = img.reshape((height, width, 4))
        return img[:, :, :3]
    
    def start(self, save_dir=None):
        if save_dir:
            os.makedirs(save_dir, exist_ok=True)
        
        print("Waiting for ascamera.exe window...")
        while self.hwnd is None:
            if not self.find_window():
                time.sleep(1)
        
        print(f"Capturing from: {win32gui.GetWindowText(self.hwnd)}")
        print(f"Target: {self.target_fps} FPS")
        print(f"Saving to: {save_dir}" if save_dir else "Not saving to disk")
        
        last_frame_time = time.time()
        fps_display_time = time.time()
        fps_counter = 0
        
        while True:
            current_time = time.time()
            
            # Enforce exactly 20 FPS
            elapsed = current_time - last_frame_time
            if elapsed < self.frame_interval:
                time.sleep(self.frame_interval - elapsed)
            
            frame = self.grab_frame()
            
            if frame is not None:
                self.frame_count += 1
                fps_counter += 1
                last_frame_time = time.time()
                
                # Resize to exactly 640x480 if needed
                if frame.shape[0] != 480 or frame.shape[1] != 640:
                    frame = cv2.resize(frame, (640, 480))
                
                # Save frame
                if save_dir:
                    filename = os.path.join(save_dir, f"frame_{self.frame_count:06d}.png")
                    cv2.imwrite(filename, frame)
                
                # Display FPS
                if time.time() - fps_display_time >= 1.0:
                    fps = fps_counter / (time.time() - fps_display_time)
                    print(f"FPS: {fps:.1f} | Total: {self.frame_count} | Size: {frame.shape}", end="\r")
                    fps_display_time = time.time()
                    fps_counter = 0
            else:
                print("Frame grab failed - window minimized?", end="\r")
            
            
        print(f"\nDone. Total frames: {self.frame_count}")


if __name__ == "__main__":
    grabber = LiveFrameGrabber(window_title_contains="_rgb_")
    grabber.start(save_dir="C:/Users/ok/Desktop/IIT KGP/YDLIDAR-ICA-ASC60C/Output/Livefeed")