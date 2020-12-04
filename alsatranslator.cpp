#include "alsatranslator.h"

AlsaTranslator::AlsaTranslator(QObject *parent)
    : QObject(parent)
{
    if (!this->location.exists())
        this->location.mkpath(".");

    this->filePath = location.filePath(fileName);
    this->audioRecorder.setOutputLocation(filePath);
    this->audioRecorder.setDeviceName((char*)"plughw:0,0");
    this->audioRecorder.setChannels(1);
    this->audioRecorder.setSampleRate(44100);
    this->audioRecorder.initCaptureDevice();

    this->url.setUrl(this->baseApi);
    this->url.setQuery("key=" + this->apiKey);

    this->request.setUrl(this->url);
    this->request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //this->request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    file.setFileName(this->filePath);

    connect(&audioRecorder, &ALSARecorder::stateChanged, [this](auto state)
    {
        //qDebug() << "state:" << state;

        if (state == ALSARecorder::StoppedState)
            this->translate();
    });

    connect(&qam, &QNetworkAccessManager::finished, [this](QNetworkReply *response)
    {
        auto data = QJsonDocument::fromJson(response->readAll());
        response->deleteLater();

        QString strFromJson = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString().c_str();

        auto error = data["error"]["message"];

        if (error.isUndefined()) {
            auto command = data["results"][0]["alternatives"][0]["transcript"].toString();
            if(!command.isEmpty())
                emit speechChanged(command);
            else
                emit speechChanged(strFromJson);

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
    //file.remove();

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
                    {"sampleRateHertz", (int)audioRecorder.getSampleRate()}
                }}
                    }
    };

    qam.post(this->request, data.toJson(QJsonDocument::Compact));
}

void AlsaTranslator::stop()
{
    m_stop = true;
}

void AlsaTranslator::record()
{
    qDebug() << "Listening...";

    setError("");
    setCommand("");
    setRunning(true);
    audioRecorder.record(recordDuration);
}

void AlsaTranslator::start()
{
    qDebug() << "Alsa Translator is started...";
    record();
}
