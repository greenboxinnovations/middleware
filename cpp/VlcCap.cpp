#include "VlcCap.h"
#include <thread>
#include <cstring>


VlcCap::VlcCap()
  : m_isOpen(false)
  , m_hasFrame(false)
  , m_vlcInstance(NULL)
  , m_mp(NULL)
{
}


VlcCap::~VlcCap() {
  try {
    release();
  } catch(...) {
    // TODO log
  }
}


void VlcCap::open(const char* url) {
  release();
  m_url = url;

  const char* args[] = {"-I", "dummy", "--ignore-config"};
  m_vlcInstance = libvlc_new(3, args);  

  libvlc_media_t* media = libvlc_media_new_location(m_vlcInstance, m_url.c_str());  
  m_mp = libvlc_media_player_new_from_media(media);  
  libvlc_media_release(media);

  libvlc_video_set_callbacks(m_mp, lock_helper, unlock_helper, NULL, this);  
  libvlc_video_set_format_callbacks(m_mp, format_helper, NULL);

  int resp = libvlc_media_player_play(m_mp);
  if(resp == 0) {
    m_isOpen = true;
  } else {
    release();
  }
}


void VlcCap::release() {
  if(m_vlcInstance) {
    libvlc_media_player_stop(m_mp);
    libvlc_release(m_vlcInstance);
    libvlc_media_player_release(m_mp);
    m_vlcInstance = NULL;
    m_mp = NULL;
  }
  m_hasFrame = false;
  m_isOpen   = false;
}


bool VlcCap::isOpened() {
  if(!m_isOpen)
      return false;

  libvlc_state_t state = libvlc_media_player_get_state(m_mp);
  return (state != libvlc_Paused && state != libvlc_Stopped && state != libvlc_Ended && state != libvlc_Error);
}

bool VlcCap::read(Mat& outFrame) {
  while(!m_hasFrame) {
    this_thread::sleep_for(chrono::milliseconds(10));
    if(!isOpened())
      return false; // connection closed
  }

  {
    lock_guard<mutex>  guard(m_mutex);
    outFrame = m_frame.clone();
    m_hasFrame = false;
  }
  return true;
}


unsigned VlcCap::format(char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines) {
  // TODO: Allow overriding of native size?
  lock_guard<mutex>  guard(m_mutex);
  //cout << "VlcCap::format - " << chroma << " - " << *width<<"/"<<*height << endl;
  memcpy(chroma, "RV24", 4);
  pitches[0] = (*width) * 24/8;
  lines[0]   = *height;
  m_frame.create(*height, *width, CV_8UC3);

  return 1;
}


void* VlcCap::lock(void** p_pixels) {
  //cout << "VlcCap::lock" << endl;
  m_mutex.lock();
  *p_pixels = (unsigned char*)m_frame.data;
  return NULL;  
}  

void VlcCap::unlock(void* id, void* const* p_pixels) {
  m_hasFrame = true;
  m_mutex.unlock();
}  


unsigned VlcCap::format_helper(void** data, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines) {
  return ((VlcCap*)(*data))->format(chroma, width, height, pitches, lines);
}


void* VlcCap::lock_helper(void* data, void** p_pixels) {
  return ((VlcCap*)data)->lock(p_pixels);
}

void VlcCap::unlock_helper(void* data, void* id, void* const* p_pixels) {
  ((VlcCap*)data)->unlock(id, p_pixels);
}