#ifndef __ALSADevices_H
#define __ALSADevices_H

#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <alsa/asoundlib.h>
#include </usr/include/alsa/pcm.h>
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

#define OPEN_ERROR        1
#define MALLOC_ERROR      2
#define ANY_ERROR         3
#define ACCESS_ERROR      4
#define FORMAT_ERROR      5
#define RATE_ERROR        6
#define CHANNELS_ERROR    7
#define PARAMS_ERROR      8
#define PREPARE_ERROR     9
#define FOPEN_ERROR       10
#define FCLOSE_ERROR      11
#define SNDREAD_ERROR     12
#define START_ERROR       13

// this is the bitrate
#define MAX_SAMPLES     512000 //wav
#define SAMPLING_RATE   16000

#include <sys/stat.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define READSIZE 1020

static unsigned totalSamples; /* can use a 32-bit number due to WAVE size limitations */
static FLAC__byte buffer[READSIZE/*samples*/ * 2/*bytes_per_sample*/ * 2/*channels*/]; /* we read the WAVE data into here */
static FLAC__int32 pcm[READSIZE/*samples*/ * 2/*channels*/];

static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void)encoder, (void)client_data;

    //fprintf(stderr, "Wrote %llu bytes, %llu/%u samples, %u/%u frames\n", bytes_written, samples_written, totalSamples, frames_written, total_frames_estimate);
}

class ALSARecorder:public QThread
{
    Q_OBJECT

protected:
    snd_pcm_t* capture_handle{};
    snd_pcm_uframes_t frames_per_period{};            // Latency - lower numbers will decrease latency and increase CPU usage.
    snd_pcm_format_t format{SND_PCM_FORMAT_S16_BE};
    FLAC__StreamEncoder *encoder{} ;// Bit depth - Quality.
    bool m_stop{false};

    void set_hw_params();
public:

    explicit ALSARecorder(QObject *parent = nullptr);
    ~ALSARecorder();

    enum State
    {
        StoppedState,
        RecordingState,
        PausedState
    };

    bool initCaptureDevice();
    void setDeviceName(const QString &value);
    void setOutputLocation(const QString &value);
    void setSampleRate(unsigned int value);
    unsigned int getSampleRate() const;
    void setChannels(unsigned int value);
    bool record(int mseconds);
    void close();

private:
    struct wav_header // Wav file header structure
    {
        uint8_t ChunkID[4];
        uint32_t ChunkSize;
        uint8_t Format[4];
        uint8_t Subchunk1ID[4];
        uint32_t Subchunk1Size;
        uint16_t AudioFormat;
        uint16_t NumChannels;
        uint32_t SampleRate;
        uint32_t ByteRate;
        uint16_t BlockAlign;
        uint16_t BitsPerSample;
        uint8_t Subchunk2ID[4];
        uint32_t Subchunk2Size;
    };

    QString deviceName;
    QString fileName;
    unsigned int sampleRate;
    unsigned int channels;

    QString baseApi = "https://speech.googleapis.com/v1/speech:recognize";
    QString apiKey = "AIzaSyCY8Xg5wfn6Ld67287SGDBQPZvGCEN6Fsg";

    bool initFlacDecoder(char* flacfile);

    char* allocate_buffer();
    unsigned int get_frames_per_period();
    unsigned int get_bytes_per_frame();

    int init_wav_header(void);
    int init_wav_file( char *);
    int close_wav_file(void);
    void set_record_file(char *aFileName);
    unsigned int capture_into_buffer(char* buffer);
    int encodePcm(const char *wavfile, const char *flacfile);

    static int FileExists(char *path)
    {
        struct stat fileStat;
        if ( stat(path, &fileStat) )
        {
            return 0;
        }
        if ( !S_ISREG(fileStat.st_mode) )
        {
            return 0;
        }
        return 1;
    }


    struct wav_header wav_h;
    char * wav_name;
    char * fname{};
    FILE * fwav{};

signals:
    void stateChanged(State state);

};

#endif
