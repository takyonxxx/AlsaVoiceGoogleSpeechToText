# AlsaVoice Google Speech To Text

This project shows you how to send a speech recognition request to Speech-to-Text using the Google REST interface.</br>

Speech-to-Text enables easy integration of Google speech recognition technologies into developer applications.
You can send audio data to the Speech-to-Text API, which then returns a text transcription of that audio file. 
For more information about the service, see Speech-to-Text basics.

Make an audio transcription request
You can use Speech-to-Text to transcribe an audio file to text.
Use this Qt c++17 project to send a recognize REST request to the Speech-to-Text API.

Project records your voice to flac file and Create a JSON request file with the following QJsonDocument:
My language is Turkish, you may set it to English. ("en-US")

 QFile file{};
 file.setFileName("your recorded flac file name with path");
 
 QByteArray fileData = file.readAll();
 file.close();

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

  
This JSON snippet indicates that the audio file has a FLAC encoding format, a sample rate of your samplerate in Hz,
and that the audio flac file data is fileData.
Important : 
In my experience recording should be 1 channel.

My settings are:
    audioRecorder.setOutputLocation(filePath);
    audioRecorder.setDeviceName((char*)"plughw:0,0");
    audioRecorder.setChannels(1);
    audioRecorder.setSampleRate(44100);
    audioRecorder.initCaptureDevice();
    
I use default sound card. Change it to your input sound card number.
(plughw:0,0, plughw:1,0, etc...)    

And change the duration time according to your needs in miliseconds.

auto google_speech = new AlsaTranslator(nullptr);
QObject::connect(google_speech, &AlsaTranslator::speechChanged, [](auto speech)
{
    qDebug() << speech;
});
google_speech->setRecordDuration(3000);
google_speech->start();

My purpose of doing this project is to control my robot. 
I tried it in Raspberry Pi3 and works efficiently.

