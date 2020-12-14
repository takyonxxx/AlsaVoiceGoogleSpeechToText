#include "alsatranslator.h"

AlsaTranslator::AlsaTranslator(QObject *parent)
    : QObject(parent)
{
    if (!this->location.exists())
        this->location.mkpath(".");

    char devname[10] = {0};
    findCaptureDevice(devname);

    if(QString(devname).isEmpty())
        return;

    this->filePath = location.filePath(fileName);
    this->audioRecorder.setOutputLocation(filePath);
    this->audioRecorder.setDeviceName(devname);
    this->audioRecorder.setChannels(1);
    this->audioRecorder.setSampleRate(44100);
    this->audioRecorder.initCaptureDevice();

    this->url.setUrl(baseApi);
    this->url.setQuery("key=" + apiKey);

    this->request.setUrl(this->url);
    this->request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //this->request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    file.setFileName(this->filePath);

    connect(&audioRecorder, &ALSARecorder::stateChanged, [this](auto state)
    {
        if (state == ALSARecorder::StoppedState)
            this->translate();
    });

    connect(&qam, &QNetworkAccessManager::finished, [this](QNetworkReply *response)
    {
        auto data = QJsonDocument::fromJson(response->readAll());
        response->deleteLater();

        //QString strFromJson = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString().c_str();

        auto error = data["error"]["message"];

        if (error.isUndefined()) {
            auto command = data["results"][0]["alternatives"][0]["transcript"].toString();
            auto confidence = data["results"][0]["alternatives"][0]["confidence"].toDouble();
            if(!command.isEmpty())
                emit speechChanged(command + " %" + QString::number(int(confidence * 100)));
            else
                emit speechChanged("Not understood!");

            setRunning(false);
            setCommand(command);

        } else {
            setRunning(false);
            setError(error.toString());
        }

        record();
    });

    qDebug() << "Flac location:" << this->filePath;
}

void AlsaTranslator::translate() {        

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug()  << "cannot open file:" << file.errorString() << file.fileName();
        setRunning(false);
        setError(file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();
    file.remove();

    QJsonDocument data {
        QJsonObject { {
                "audio",
                QJsonObject { {"content", QJsonValue::fromVariant(fileData.toBase64())} }
                      },  {
                "config",
                QJsonObject {
                    {"encoding", "FLAC"},
                    {"languageCode", "tr-TR"},
                    {"model", "command_and_search"},
                    {"enableWordTimeOffsets", false},
                    {"sampleRateHertz", (int)audioRecorder.getSampleRate()}
                }}
                    }
    };

    qam.post(this->request, data.toJson(QJsonDocument::Compact));
}

void AlsaTranslator::setRecordDuration(int value)
{
    recordDuration = value;
}

void AlsaTranslator::record()
{
    setError("");
    setCommand("");
    setRunning(true);
    audioRecorder.record(recordDuration);
}

void AlsaTranslator::findCaptureDevice(char *devname)
{
    int idx, dev, err;
    snd_ctl_t *handle;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    char str[128];
    bool found = false;

    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);
    printf("\n");

    idx = -1;
    while (!found)
    {
        if ((err = snd_card_next(&idx)) < 0) {
            printf("Card next error: %s\n", snd_strerror(err));
            break;
        }
        if (idx < 0)
            break;
        sprintf(str, "hw:CARD=%i", idx);
        if ((err = snd_ctl_open(&handle, str, 0)) < 0) {
            printf("Open error: %s\n", snd_strerror(err));
            continue;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            printf("HW info error: %s\n", snd_strerror(err));
            continue;
        }

        dev = -1;
        while (1) {
            snd_pcm_sync_id_t sync;
            if ((err = snd_ctl_pcm_next_device(handle, &dev)) < 0) {
                printf("  PCM next device error: %s\n", snd_strerror(err));
                break;
            }
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                printf("Sound card - %i - '%s' has no capture device.\n",
                       snd_ctl_card_info_get_card(info), snd_ctl_card_info_get_name(info));
                continue;
            }
            printf("Sound card - %i - '%s' has capture device.\n", snd_ctl_card_info_get_card(info), snd_ctl_card_info_get_name(info));
            sprintf(devname, "plughw:%d,0", snd_ctl_card_info_get_card(info));
            found = true;
            break;
        }
        snd_ctl_close(handle);
    }

    snd_config_update_free_global();
}

void AlsaTranslator::start()
{
    qDebug() << "Alsa Translator is started...";
    record();
}
