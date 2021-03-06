#include "FFmpegWrapper.hpp"
#include "FFmpegVideoReader.hpp"
#include "FFmpegAudioReader.hpp"

namespace JAZZROS {

long                                        FFmpegWrapper::g_maxIndexVideoFiles = 0;
std::map<const long, FFmpegVideoReader*>    FFmpegWrapper::g_openedVideoFiles;
long                                        FFmpegWrapper::g_maxIndexAudioFiles = 0;
std::map<const long, FFMPEGAUDIOREADER*>    FFmpegWrapper::g_openedAudioFiles;


/// ====================================================================================
/// Reading video
/// ====================================================================================

const short
FFmpegWrapper::checkIndexVideoValid(const long indexFile)
{
    if (g_openedVideoFiles.find(indexFile) != g_openedVideoFiles.end())
    {
        return 0;
    }
    else
    {
        av_log(NULL, AV_LOG_ERROR, "Try access to unexist %ld index of files", indexFile);
    }
    return -1;
}

const long
FFmpegWrapper::openVideo(const char * pFileName,
                         FFmpegParameters * parameters,
                         AVPixelFormat & outPixFmt,
                         float & aspectRatio,
                         float & frame_rate,
                         bool & alphaChannel)
{
    long ret_falue = -1;
    FFmpegVideoReader* media = new FFmpegVideoReader();
    try
    {
        int ret = media->openFile(pFileName, parameters, outPixFmt, aspectRatio, frame_rate, alphaChannel);
        if (ret == 0)
        {
            outPixFmt = media->getPixFmt();
            const long maxIndexVideoFiles = g_maxIndexVideoFiles;

            g_openedVideoFiles[maxIndexVideoFiles] = media;

            ++g_maxIndexVideoFiles;
            ret_falue = maxIndexVideoFiles;
        }
        else
        {
            // Message notified before
        }
    }
    catch (...)
    {
        ret_falue = -1;
    }

    if (ret_falue < 0)
        delete media;

    return ret_falue;
}

const short
FFmpegWrapper::closeVideo(const long indexFile)
{
    short rez_value = -1;
    try
    {
        if (checkIndexVideoValid(indexFile) == 0)
        {
            FFmpegVideoReader* media = g_openedVideoFiles[indexFile];
            media->close();
            delete media;
            g_openedVideoFiles.erase(indexFile);
            rez_value = 0;
        }
    }
    catch (...)
    {
        rez_value = -1;
    }
    return rez_value;
}


const short
FFmpegWrapper::getImageSize(const long indexFile, unsigned short * ptr)
{
    short ret_value = -1;
    try
    {
        if (ptr)
        {
            if (checkIndexVideoValid(indexFile) == 0)
            {
                const int w = g_openedVideoFiles[indexFile]->get_width();
                const int h = g_openedVideoFiles[indexFile]->get_height();
                //
                ptr[0] = (unsigned short)w;
                ptr[1] = (unsigned short)h;

                ret_value = 0;
            }
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}

const short
FFmpegWrapper::getVideoTimeLimits(const long indexFile, unsigned long * ptr)
{
    short ret_value = -1;
    try
    {
        if (ptr)
        {
            if (checkIndexVideoValid(indexFile) == 0)
            {
                const int64_t duration = g_openedVideoFiles[indexFile]->get_duration();
                //
                ptr[0] = (unsigned long)0;
                ptr[1] = (unsigned long)duration;

                ret_value = 0;
            }
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}

const short
FFmpegWrapper::getImage(const long indexFile, unsigned long msTime, unsigned char * bufRGB24)
{
    short ret_value = -1;
    try
    {
        if (checkIndexVideoValid(indexFile) == 0 && bufRGB24 != NULL)
        {
            unsigned long timeLimits[2];
            if (FFmpegWrapper::getVideoTimeLimits(indexFile, &timeLimits[0]) == 0)
            {
                if (msTime >= timeLimits[0] && msTime <= timeLimits[1])
                {
                    const int64_t   timestamp(msTime);
                    const int       seekRez = g_openedVideoFiles[indexFile]->seek(timestamp, bufRGB24);
                    if (seekRez == 0)
                    {
                        ret_value = 0;
                    }
                }
                else
                {
                    av_log(NULL, AV_LOG_WARNING, "Video seeking: asked time is out of range");
                }
            }
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}

const short
FFmpegWrapper::getImageFastNonAccurate(const long indexFile, unsigned long & msTime, unsigned char * bufRGB24)
{
    short ret_value = -1;
    try
    {
        if (checkIndexVideoValid(indexFile) == 0 && bufRGB24 != NULL)
        {
            unsigned long timeLimits[2];
            if (FFmpegWrapper::getVideoTimeLimits(indexFile, &timeLimits[0]) == 0)
            {
                if (msTime >= timeLimits[0] && msTime <= timeLimits[1])
                {
                    int64_t         timestamp(msTime);
                    const int       seekRez = g_openedVideoFiles[indexFile]->fast_nonaccurate_seek(timestamp, bufRGB24);
                    if (seekRez == 0)
                    {
                        msTime = timestamp;
                        ret_value = 0;
                    }
                }
                else
                {
                    av_log(NULL, AV_LOG_WARNING, "Video seeking: asked time is out of range");
                }
            }
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}

const short
FFmpegWrapper::getNextImage(const long indexFile, unsigned char * bufRGB24, double & timeStampInSec, const size_t & drop_frame_nb, const bool decodeTillMinReqTime, const double minReqTimeMS)
{
    short ret_value = -1;
    try
    {
        if (checkIndexVideoValid(indexFile) == 0 && bufRGB24 != NULL)
        {
            ret_value = g_openedVideoFiles[indexFile]->grabNextFrame(bufRGB24, timeStampInSec, drop_frame_nb, decodeTillMinReqTime, minReqTimeMS);
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}
/// ====================================================================================
/// Reading audio
/// ====================================================================================
const short
FFmpegWrapper::checkIndexAudioValid(const long indexFile)
{
    if (g_openedAudioFiles.find(indexFile) != g_openedAudioFiles.end())
    {
        return 0;
    }
    else
    {
        av_log(NULL, AV_LOG_ERROR, "Try access to unexist %ld index of files", indexFile);
    }
    return -1;
}

const short
FFmpegWrapper::getAudioTimeLimits(const long indexFile, unsigned long * ptr)
{
    short ret_value = -1;
    try
    {
        if (ptr)
        {
            if (checkIndexAudioValid(indexFile) == 0)
            {
                const int64_t duration = g_openedAudioFiles[indexFile]->get_duration();
                //
                ptr[0] = (unsigned long)0;
                ptr[1] = (unsigned long)duration;

                ret_value = 0;
            }
        }
    }
    catch (...)
    {
        ret_value = -1;
    }
    return ret_value;
}

const long
FFmpegWrapper::openAudio(const char * pFileName, FFmpegParameters * parameters)
{
    long ret_falue = -3;
    FFMPEGAUDIOREADER* media = new FFMPEGAUDIOREADER;
    try
    {
        int ret = media->openFile(pFileName, parameters);
        if (ret == 0)
        {
            //
            // Initialize \media
            //
            media->seek(0);

            unsigned long   startTimeMS;
            AVSampleFormat  output_sampleFormat;
            unsigned short  output_FrameRate;
            unsigned long   samplesNb;
            FFMPEGAUDIOREADER::getSamples(media,
                                            startTimeMS,
                                            0,
                                            output_sampleFormat,
                                            output_FrameRate,
                                            samplesNb,
                                            NULL,
                                            -1.0);


            const long maxIndexAudioFiles = g_maxIndexAudioFiles;

            g_openedAudioFiles[maxIndexAudioFiles] = media;

            ++g_maxIndexAudioFiles;
            ret_falue = maxIndexAudioFiles;
        }
        else
        {
            // Message notified before
        }
    }
    catch (...)
    {
        ret_falue = -2;
    }

    if (ret_falue < 0)
        delete media;

    return ret_falue;
}

const short
FFmpegWrapper::seekAudio(const long indexFile, const unsigned long time)
{
    short rez_value = -1;
    try
    {
        if (checkIndexAudioValid(indexFile) == 0)
        {
            FFMPEGAUDIOREADER* media = g_openedAudioFiles[indexFile];

            if (media->seek(time) >= 0)
            {
                //
                // Initialize grabber
                //
                unsigned long   startTimeMS;
                AVSampleFormat  output_sampleFormat;
                unsigned short  output_FrameRate;
                unsigned long   samplesNb;
                FFMPEGAUDIOREADER::getSamples(media,
                                                startTimeMS,
                                                0,
                                                output_sampleFormat,
                                                output_FrameRate,
                                                samplesNb,
                                                NULL,
                                                -1.0);

                rez_value = 0;
            }
        }
    }
    catch (...)
    {
        rez_value = -1;
    }
    return rez_value;
}

const short
FFmpegWrapper::closeAudio(const long indexFile)
{
    short rez_value = -1;
    try
    {
        if (checkIndexAudioValid(indexFile) == 0)
        {
            FFMPEGAUDIOREADER* media = g_openedAudioFiles[indexFile];

            media->close();
            delete media;
            g_openedAudioFiles.erase(indexFile);
            rez_value = 0;
        }
    }
    catch (...)
    {
        rez_value = -1;
    }
    return rez_value;
}

const short
FFmpegWrapper::getAudioInfo(const long indexFile, unsigned long * data)
{
    short rez_value = -1;
    try
    {
        if (checkIndexAudioValid(indexFile) == 0)
        {
            FFMPEGAUDIOREADER* media = g_openedAudioFiles[indexFile];

            data[0] = (unsigned long)(media->getSampleSizeInBytes());
            data[1] = (unsigned long)(media->getChannels());
            data[2] = (unsigned long)(media->getFrameRate());
            data[3] = (int)(media->getSampleFormat());

            rez_value = 0;
        }
    }
    catch (...)
    {
        rez_value = -1;
    }
    return rez_value;
}

const int
FFmpegWrapper::getAudioSamples(const long indexFile,
                                unsigned long msTime,
                                unsigned short channelsNb,
                                const AVSampleFormat & output_sampleFormat,
                                unsigned short sample_rate,
                                unsigned long samplesNb,
                                unsigned char * bufSamples,
                                const double & max_avail_time_micros)
{
    int rez_value = -1;
    try
    {
        if (checkIndexAudioValid(indexFile) == 0)
        {
            FFMPEGAUDIOREADER* media = g_openedAudioFiles[indexFile];

            rez_value = FFMPEGAUDIOREADER::getSamples(media,
                                                    msTime,
                                                    channelsNb,
                                                    output_sampleFormat,
                                                    sample_rate,
                                                    samplesNb,
                                                    bufSamples,
                                                    max_avail_time_micros);
        }
    }
    catch (...)
    {
        rez_value = -1;
    }
    return rez_value;
}
} // namespace JAZZROS
