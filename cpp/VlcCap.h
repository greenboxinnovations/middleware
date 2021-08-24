#ifndef VLCCAP_H
#define VLCCAP_H


#include <opencv2/opencv.hpp>
#include <vlc/vlc.h>
#include <mutex>
#include <atomic>


using namespace std;
using namespace cv;


class VlcCap {
    public:
        VlcCap();
        ~VlcCap();

        void open(const char* url);
        void release();
        bool isOpened();

        bool read(Mat& outFrame);


    private:
        unsigned format(char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines);
        void*    lock(void** p_pixels);
        void     unlock(void* id, void* const* p_pixels);

        static unsigned format_helper(void** data, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines);
        static void*    lock_helper(void* data, void** p_pixels);
        static void     unlock_helper(void* data, void* id, void* const* p_pixels);


    private:
        mutex           m_mutex;
        string          m_url;
        Mat             m_frame;
        bool            m_isOpen;
        atomic<bool>    m_hasFrame;

        libvlc_instance_t*      m_vlcInstance;
        libvlc_media_player_t*  m_mp;
};


#endif //VLCCAP_H