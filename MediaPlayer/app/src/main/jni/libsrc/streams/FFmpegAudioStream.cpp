#include "FFmpegAudioStream.hpp"

#include "../FFmpegFileHolder.hpp"
#include "FFmpegStreamer.hpp"

#include <stdexcept>

namespace JAZZROS {

FFmpegAudioStream::FFmpegAudioStream(FFmpegFileHolder * pFileHolder, FFmpegStreamer * pStreamer)
:m_pFileHolder(pFileHolder),m_streamer(pStreamer)
{
}


FFmpegAudioStream::FFmpegAudioStream(const FFmpegAudioStream & audio) :
    AudioStream(audio)
{
}

FFmpegAudioStream::~FFmpegAudioStream()
{
    // detact the audio sink first to avoid destrction order issues.
    // trystan: caused #3333, crash on close audio.
    // setAudioSink (NULL);
}

void FFmpegAudioStream::setAudioSink(AudioSink* audio_sink)
{
    av_log(NULL, AV_LOG_INFO, "FFmpegAudioStream::setAudioSink");

    m_streamer->setAudioSink(audio_sink);
}


void FFmpegAudioStream::consumeAudioBuffer(void * const buffer, const size_t size)
{
    //
    // Inform consume audio buffer size
    //
    static double        playbackSec = -1.0;
    if (playbackSec < 0.0 && size > 0 && m_pFileHolder)
    {
        const AudioFormat   audioFormat = m_pFileHolder->getAudioFormat();
        playbackSec = (double)size / (double)(audioFormat.m_sampleRate * audioFormat.m_bytePerSample * audioFormat.m_channelsNb);
        av_log(NULL, AV_LOG_INFO, "Consumed audio chunk: %f sec", playbackSec);
    }

    m_streamer->audio_fillBuffer(buffer, size);
}

double FFmpegAudioStream::duration() const
{
    return m_pFileHolder->duration_ms();
}


int FFmpegAudioStream::audioFrequency() const
{
    return m_pFileHolder->getAudioFormat().m_sampleRate;
}



int FFmpegAudioStream::audioNbChannels() const
{
    return m_pFileHolder->getAudioFormat().m_channelsNb;
}

int
FFmpegAudioStream::bytePerSample() const
{
    return m_pFileHolder->getAudioFormat().m_bytePerSample;
}

AudioStream::SampleFormat
FFmpegAudioStream::audioSampleFormat() const
{
    AudioStream::SampleFormat  result;
    //
    // see: https://www.ffmpeg.org/doxygen/2.5/group__lavu__sampfmts.html#details
    //
    // "The data described by the sample format is always in native-endian order.
    //  Sample values can be expressed by native C types, hence the lack of a signed 24-bit sample format
    //  even though it is a common raw audio data format."
    //
    // So, even if some file contains audio with 24-bit samples,
    // ffmpeg converts it to available AV_SAMPLE_FMT_S32 automatically
    //
    switch (m_pFileHolder->getAudioFormat().m_avSampleFormat)
    {
    case AV_SAMPLE_FMT_U8:
        result = AudioStream::SAMPLE_FORMAT_U8;
        break;
    case AV_SAMPLE_FMT_S16:
        result = AudioStream::SAMPLE_FORMAT_S16;
        break;
    case AV_SAMPLE_FMT_S32:
        result = AudioStream::SAMPLE_FORMAT_S32;
        break;
    case AV_SAMPLE_FMT_FLT:
        result = AudioStream::SAMPLE_FORMAT_F32;
        break;
    default:
        throw std::runtime_error("unknown audio format");
    };

    return result;
}


} // namespace JAZZROS
