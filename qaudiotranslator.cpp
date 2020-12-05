#include "qaudiotranslator.h"

QAudioTranslator::QAudioTranslator(QObject *parent)
    : QObject(parent)
{
    if (!this->location.exists())
        this->location.mkpath(".");

    this->filePath = location.filePath(fileName);

    this->audioSettings.setCodec("audio/x-flac");
    this->audioSettings.setSampleRate(44100);
    this->audioSettings.setQuality(QMultimedia::VeryHighQuality);
    this->audioSettings.setChannelCount(1);
    this->audioRecorder.setEncodingSettings(audioSettings);
    this->audioRecorder.setOutputLocation(filePath);

    this->url.setUrl(baseApi);
    this->url.setQuery("key=" + apiKey);

    this->request.setUrl(this->url);
    this->request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //this->request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    file.setFileName(this->filePath);

    connect(&audioRecorder, &QAudioRecorder::durationChanged, [this](auto duration)
    {
        if (duration >= this->recordDuration &&
                this->audioRecorder.state() != QAudioRecorder::StoppedState &&
                this->audioRecorder.status() == QAudioRecorder::Status::RecordingStatus)
        {
            this->audioRecorder.stop();
        }
    });

    connect(&audioRecorder, &QAudioRecorder::statusChanged, [](auto status)
    {
        //qDebug() << "status:" << status;
    });

    connect(&audioRecorder, &QAudioRecorder::stateChanged, [this](auto state)
    {
        //qDebug() << "state:" << state;

        if (state == QAudioRecorder::StoppedState)
            this->translate();
    });

    connect(&audioRecorder, QOverload<QAudioRecorder::Error>::of(&QAudioRecorder::error), [this]
    {
        qDebug() << "error:" << audioRecorder.errorString();

        setRunning(false);
        setError(audioRecorder.errorString());
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

        if(!m_stop)
            record();
    });

    qDebug() << "Flac location:" << this->filePath;
}

void QAudioTranslator::translate() {

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
                    {"languageCode", "en-US"},
                    {"model", "command_and_search"},
                    {"sampleRateHertz", audioSettings.sampleRate()}
                }}
        }
    };

    qam.post(this->request, data.toJson(QJsonDocument::Compact));
}

void QAudioTranslator::stop()
{
    m_stop = true;
}

void QAudioTranslator::record()
{
    qDebug() << "Listening...";

    setError("");
    setCommand("");
    setRunning(true);
    audioRecorder.record();
}

void QAudioTranslator::start()
{
    qDebug() << "QAudio Translator is started...";
    record();
}
