#include "alsarecorder.h"

ALSARecorder::ALSARecorder(QObject *parent)
    : QThread(parent)
{
}

ALSARecorder::~ALSARecorder()
{
    close();
}

bool ALSARecorder::initCaptureDevice() {

    snd_pcm_hw_params_t *params;

    int err = 0;

    if ((err = snd_pcm_open(&capture_handle, deviceName.toStdString().c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0)

    {
        std::cerr << "cannot open audio device " << capture_handle << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return OPEN_ERROR;
    }

    if ((err = snd_pcm_hw_params_malloc(&params)) < 0)
    {
        std::cerr << "cannot allocate hardware parameter structure " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return MALLOC_ERROR;
    }

    if ((err = snd_pcm_hw_params_any(capture_handle, params)) < 0)
    {
        std::cerr << "cannot initialize hardware parameter structure " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return ANY_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        std::cerr << "cannot set access type " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return ACCESS_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        std::cerr << "cannot set sample format " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return FORMAT_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, params, &sampleRate, 0)) < 0)
    {
        std::cerr << "cannot set sample rate " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return RATE_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, params, channels))< 0)
    {
        std::cerr << "cannot set channel count " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return CHANNELS_ERROR;
    }

    if ((err =  snd_pcm_hw_params_set_period_size_near(capture_handle, params, &frames_per_period, 0)) < 0)
    {
        std::cerr << "cannot set period_size " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return RATE_ERROR;
    }

    if ((err = snd_pcm_hw_params(capture_handle, params)) < 0)
    {
        std::cerr << "cannot set parameters " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return PARAMS_ERROR;
    }

    if ((err = snd_pcm_prepare(capture_handle)) < 0)
    {
        std::cerr << "cannot prepare audio interface for use " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return PREPARE_ERROR;
    }

    if ((err = snd_pcm_start(capture_handle)) < 0)
    {
        std::cerr << "cannot start soundcard " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return START_ERROR;
    }


    unsigned int val;
    int dir;

    snd_pcm_hw_params_get_period_size(params, &frames_per_period, &dir);
    int size = frames_per_period * 4; /* 2 bytes/sample, 2 channels */

    printf("\n");
    //printf("ALSA library version: %s\n", SND_LIB_VERSION_STR);
    printf("Device type: capture\n");
    printf("Device name = '%s'\n", snd_pcm_name(capture_handle));
    snd_pcm_hw_params_get_channels(params, &val);
    printf("Channels = %d\n", val);
    snd_pcm_hw_params_get_rate(params, &val, &dir);
    printf("Rate = %d bps\n", val);
    printf("Size_of_one_frame = %d frames\n", get_frames_per_period());
    val = snd_pcm_hw_params_get_sbits(params);
    printf("Significant bits = %d\n", val);
    printf("Size = %d\n", size);
    printf("\n");
    return true;
}

void ALSARecorder::close() {

    m_stop = true;

    if(capture_handle)
    {
        snd_pcm_drain(capture_handle);
        snd_pcm_close(capture_handle);
    }
}

unsigned int ALSARecorder::getSampleRate() const
{
    return sampleRate;
}

void ALSARecorder::setChannels(unsigned int value)
{
    channels = value;
}

void ALSARecorder::setSampleRate(unsigned int value)
{
    sampleRate = value;
    frames_per_period = (int)(sampleRate/60.0);
}

void ALSARecorder::setOutputLocation(const QString &value)
{
    fileName = value;
}

void ALSARecorder::setDeviceName(const QString &value)
{
    deviceName = value;
}

char* ALSARecorder::allocate_buffer() {
    unsigned int size_of_one_frame = (snd_pcm_format_width(format)/8) * channels;
    return (char*) calloc(frames_per_period, size_of_one_frame);
}

unsigned int ALSARecorder::get_frames_per_period() {
    return frames_per_period;
}

unsigned int ALSARecorder::get_bytes_per_frame() {
    unsigned int size_of_one_frame = (snd_pcm_format_width(format)/8) * channels;
    return size_of_one_frame;
}


int ALSARecorder::init_wav_header()
{
    wav_h.ChunkID[0]     = 'R';
    wav_h.ChunkID[1]     = 'I';
    wav_h.ChunkID[2]     = 'F';
    wav_h.ChunkID[3]     = 'F';

    wav_h.Format[0]      = 'W';
    wav_h.Format[1]      = 'A';
    wav_h.Format[2]      = 'V';
    wav_h.Format[3]      = 'E';

    wav_h.Subchunk1ID[0] = 'f';
    wav_h.Subchunk1ID[1] = 'm';
    wav_h.Subchunk1ID[2] = 't';
    wav_h.Subchunk1ID[3] = ' ';

    wav_h.Subchunk2ID[0] = 'd';
    wav_h.Subchunk2ID[1] = 'a';
    wav_h.Subchunk2ID[2] = 't';
    wav_h.Subchunk2ID[3] = 'a';

    wav_h.NumChannels = channels;
    wav_h.BitsPerSample = 16;
    wav_h.Subchunk2Size = 300 * MAX_SAMPLES * (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
    //wav_h.Subchunk2Size = 0xFFFFFFFF;
    wav_h.ChunkSize = (uint32_t) wav_h.Subchunk2Size + 36;
    wav_h.Subchunk1Size = 16;
    wav_h.AudioFormat = 1;
    wav_h.SampleRate = sampleRate;
    wav_h.ByteRate = (uint32_t) wav_h.SampleRate
            * (uint32_t) wav_h.NumChannels
            * (uint32_t) wav_h.BitsPerSample / 8;
    wav_h.BlockAlign = (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;

    return EXIT_SUCCESS;
}

int ALSARecorder::init_wav_file(char *fname)
{
    if(FileExists(fname))
    {
        std::remove(fname); // delete file
    }

    fwav = fopen(fname, "wb");

    if (fwav != NULL)
        fwrite(&wav_h, 1, sizeof(wav_h), fwav);
    else
    {
        std::cerr << "cannot open wav file to write data" << "\n";
        return FOPEN_ERROR;
    }

    return EXIT_SUCCESS;
}

int ALSARecorder::close_wav_file()
{
    if (fwav != nullptr)
        fclose(fwav);
    else
    {
        std::cerr << "cannot close wav file" << "\n";
        return FCLOSE_ERROR;
    }

    return EXIT_SUCCESS;
}

void ALSARecorder::set_record_file(char *aFileName)
{
    fname = aFileName;
    if(fname)
    {
        init_wav_header();
        init_wav_file(fname);
    }
}

unsigned int ALSARecorder::capture_into_buffer(char* buffer) {

    snd_pcm_sframes_t frames_read = snd_pcm_readi(capture_handle, buffer, get_frames_per_period());


    if(frames_read == 0) {
        fprintf(stderr, "End of file.\n");
        return 0;
    }
    else if (frames_read == -EPIPE) {
        /* EPIPE means overrun */
        snd_pcm_prepare(capture_handle);
        return 0;
    }
    return frames_read;
}

int ALSARecorder::encodePcm(const char *wavfile, const char *flacfile)
{
    FLAC__bool ok = true;
    FLAC__StreamEncoder *encoder = 0;
    FLAC__StreamEncoderInitStatus initStatus;
    FILE *fin;

    unsigned bps = 0;

    if((fin = fopen(wavfile, "rb")) == NULL)
    {
        fprintf(stderr, "ERROR: opening %s for output\n", wavfile);
        return 1;
    }

    if (fread(buffer, 1, 44, fin) != 44 || memcmp(buffer, "RIFF", 4))
    {
        fprintf(stderr, "ERROR: invalid/unsupported WAVE file.\n");
        fclose(fin);
        return 1;
    }

    bps = 16;
    totalSamples = (((((((unsigned)buffer[43] << 8) | buffer[42]) << 8) | buffer[41]) << 8) | buffer[40]) / 4;

    /* allocate the encoder */
    if((encoder = FLAC__stream_encoder_new()) == NULL)
    {
        fprintf(stderr, "ERROR: allocating encoder\n");
        fclose(fin);
        return 1;
    }

    ok &= FLAC__stream_encoder_set_verify(encoder, true);
    ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
    ok &= FLAC__stream_encoder_set_channels(encoder, channels);
    ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
    ok &= FLAC__stream_encoder_set_sample_rate(encoder, sampleRate);
    ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, totalSamples);

    /* initialize encoder */
    if(ok)
    {
        initStatus = FLAC__stream_encoder_init_file(encoder, flacfile, progress_callback, nullptr);
        if(initStatus != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
        {
            fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[initStatus]);
            ok = false;
        }
    }

    /* read blocks of samples from WAVE file and feed to encoder */
    if(ok)
    {
        fprintf(stdout, "Encoding: ");
        size_t left = (size_t)totalSamples;
        while(ok && left)
        {
            size_t need = (left>READSIZE ? (size_t)READSIZE : (size_t)left);
            if (fread(buffer, channels * (bps / 8), need, fin) != need)
            {
                ok = false;
            }
            else
            {
                /* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
                size_t i;
                for(i = 0; i < need*channels; i++)
                {
                    /* inefficient but simple and works on big- or little-endian machines */
                    pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[2 * i + 1] << 8) | (FLAC__int16)buffer[2 * i]);
                }
                /* feed samples to encoder */
                ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
            }
            left -= need;
        }
    }

    if (!FLAC__stream_encoder_finish(encoder))
    {
        return fprintf(stderr, "ERROR: finishing encoder: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
    }

    FLAC__stream_encoder_delete(encoder);

    fclose(fin);
    return 0;
}


bool ALSARecorder::record(int mseconds)
{  
    if(!capture_handle)
        return false;

    auto wavFile = fileName;
    wavFile = wavFile.replace("flac", "wav");

    set_record_file((char*)wavFile.toStdString().c_str());

    char* buffer = allocate_buffer();
    auto endwait = QDateTime::currentMSecsSinceEpoch() + mseconds;

    do
    {
        if(m_stop)
            break;

        auto read = capture_into_buffer(buffer);

        if (fwav != nullptr)
        {
            fwrite(buffer, 1, read * 2 * channels, fwav);
        }

    } while (QDateTime::currentMSecsSinceEpoch() < endwait);

    if (fwav != nullptr)
    {
        fwrite(&wav_h, 1, sizeof(wav_h), fwav);
        close_wav_file();
    }

    encodePcm((char*)wavFile.toStdString().c_str(), (char*)fileName.toStdString().c_str());

    if (fwav != nullptr)
    {
        if( remove(wavFile.toStdString().c_str()) != 0 )
            perror( "Error deleting wav file" );
    }

    emit stateChanged(StoppedState);
    return true;
}
