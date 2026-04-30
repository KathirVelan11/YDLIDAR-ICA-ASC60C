/**
 * @file      main.cpp
 * @brief     angstrong camera demo main program.
 *
 * Copyright (c) 2023 Angstrong Tech.Co.,Ltd
 *
 * @author    Angstrong SDK develop Team
 * @date      2023/05/12
 * @version   1.0

 */

#include <signal.h>
#include "Demo.h"

static Demo demo;

static void Get_CtrlC_handler(int sig)
{
    signal(sig, SIG_IGN);
    LOG(INFO) << "get Ctrl-C, now to exit safely" << std::endl;
    demo.stop();
    LOG(INFO) << "angstrong camera sdk exit." << std::endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    signal(SIGINT, Get_CtrlC_handler);

    demo.start();

    while (true) {
        char ch = (char)getchar();
        if (ch == 's') {
            demo.saveImage();
        } else if (ch == 'f') {
            /* calculate the frame rate */
            demo.logFps(!demo.getLogFps());
        } else if (ch == 'q') {
            break;
        } else if (ch == 'l') {
            /* calculate the frame rate */
            demo.logCfgParameter();
        } else if (ch == 'd') {
#ifdef CFG_OPENCV_ON
            /* disply the image */
            bool enable = demo.getDisplayStatus();
            demo.display(!enable);
            if (!enable) {
                cv::destroyAllWindows();
            }
#else
            LOG(WARN) << "please install opencv and recompilation !" << std::endl;
#endif
        }
    }

    demo.stop();
    return ret;
}
