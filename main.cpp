#include <iostream>
#include <vector>
#include <chrono>
extern "C" {
    #include <unistd.h>
    #include <time.h>
    #include <sys/time.h>
    #include <gst/gst.h>
    #include <gst/app/gstappsink.h>
    #include <gst/app/gstappsrc.h>
}
#define  PROP "videotestsrc ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! x264enc ! appsink name=yuvsink"

GstElement * pipeline=NULL;
GstElement * appsink=NULL;
GstAppSinkCallbacks *appsink_callbacks=NULL;

struct CameraData{
    std::vector<unsigned char>  yuvBuffer;
    std::time_t timestamp;
    CameraData(){

    }
    CameraData(const std::vector<unsigned char> &yuvBuffer) : yuvBuffer(yuvBuffer) {

    }
    virtual ~CameraData() {

    }

    CameraData & operator=(const CameraData &rhs){
        yuvBuffer == rhs.yuvBuffer;
        timestamp == rhs.timestamp;
        return *this;
    }

};

std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());//获取当前时间点
    std::time_t timestamp =  tp.time_since_epoch().count(); //计算距离1970-1-1,00:00的时间长度
    return timestamp;
}


static GstFlowReturn appsink_new_sample_callback(GstAppSink *slt,gpointer user_data)
{
    CameraData  data;
    GstSample* sample = gst_app_sink_pull_sample(slt);
    if(sample == NULL) {
        return GST_FLOW_ERROR;
    }
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMemory* memory = gst_buffer_get_all_memory(buffer);
    GstMapInfo map_info;
    if(!gst_memory_map(memory, &map_info, GST_MAP_READ)) {
        gst_memory_unref(memory);
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }
    data.yuvBuffer.resize(map_info.size);
    g_printerr ("on app_sink_new_sample size %.2fk\n",map_info.size/1024.00);
    gst_buffer_extract(buffer, 0, data.yuvBuffer.data(),map_info.size);
    //render using map_info.data
    data.timestamp=getTimeStamp();
    gst_memory_unmap(memory, &map_info);
    gst_memory_unref(memory);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

int main() {
    gst_init(NULL,NULL);
    pipeline=gst_parse_launch(PROP,NULL);
    appsink = gst_bin_get_by_name(GST_BIN(pipeline),"yuvsink");
    gst_app_sink_set_emit_signals(reinterpret_cast<GstAppSink *>(appsink), true);
    gst_app_sink_set_max_buffers (reinterpret_cast<GstAppSink *>(appsink), 10); // limit number of buffers queued
    gst_app_sink_set_drop(reinterpret_cast<GstAppSink *>(appsink), true ); // drop old buffers in queue when full
    appsink_callbacks = (GstAppSinkCallbacks *)malloc(sizeof(GstAppSinkCallbacks));
    appsink_callbacks->eos = NULL;
    appsink_callbacks->new_preroll = NULL;
    appsink_callbacks->new_sample = appsink_new_sample_callback;
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), appsink_callbacks,NULL,NULL);
    gst_element_set_state(pipeline,GST_STATE_PLAYING);
    while (true){
        sleep(1);
    }
    return 0;
}