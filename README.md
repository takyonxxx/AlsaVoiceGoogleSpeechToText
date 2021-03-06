# AlsaVoice Google Speech To Text

This project shows you how to send a speech recognition request to Speech-to-Text using the Google REST interface.</br>
</br>
#sudo apt-get install libasound2-dev</br>
#sudo apt-get install alsa alsa-tools</br>
#sudo apt-get install qtmultimedia5-dev</br>
#sudo apt install libqt5multimedia5-plugins</br>
#sudo apt-get install libflac-dev</br>
</br>

Speech-to-Text enables easy integration of Google speech recognition technologies into developer applications.</br>
You can send audio data to the Speech-to-Text API, which then returns a text transcription of that audio file.</br> 
For more information about the service, see Speech-to-Text basics.</br>
</br>
Make an audio transcription request</br>
You can use Speech-to-Text to transcribe an audio file to text.</br>
Use this Qt c++17 project to send a recognize REST request to the Speech-to-Text API.</br>
</br>
Project records your voice to flac file and Create a JSON request file with the following QJsonDocument:</br>
My language is Turkish, you may set it to English. ("en-US")</br>
</br>
 QFile file{};</br>
 file.setFileName("your recorded flac file name with path");</br>
 QByteArray fileData = file.readAll();</br>
 file.close();</br>
 QJsonDocument data {</br>
        QJsonObject { {</br>
                "audio",</br>
                QJsonObject { {"content", QJsonValue::fromVariant(fileData.toBase64())} }</br>
                      },  {</br>
                "config",</br>
                QJsonObject {</br>
                    {"encoding", "FLAC"},</br>
                    {"languageCode", "tr-TR"},</br>
                    {"model", "command_and_search"},</br>
                    {"enableWordTimeOffsets", false},</br>
                    {"sampleRateHertz", (int)audioRecorder.getSampleRate()}</br>
                }}</br>
                    }</br>
    };
</br>  
This JSON snippet indicates that the audio file has a FLAC encoding format, a sample rate of your samplerate in Hz,</br>
and that the audio flac file data is fileData.</br>
Important : </br>
In my experience recording should be 1 channel.</br>
</br>
My settings are:</br>
    audioRecorder.setOutputLocation(filePath);</br>
    audioRecorder.setDeviceName((char*)"plughw:0,0");</br>
    audioRecorder.setChannels(1);</br>
    audioRecorder.setSampleRate(44100);</br>
    audioRecorder.initCaptureDevice();</br>
</br> 
I use default sound card. Change it to your input sound card number.</br>
(plughw:0,0, plughw:1,0, etc...)</br>    
And change the duration time according to your needs in miliseconds.</br>
</br>
auto google_speech = new AlsaTranslator(nullptr);</br>
QObject::connect(google_speech, &AlsaTranslator::speechChanged, [](auto speech)</br>
{</br>
    qDebug() << speech;</br>
});</br>
google_speech->setRecordDuration(3000);</br>
google_speech->start();</br>
</br>
My purpose of doing this project is to control my robot.</br> 
I tried it in Raspberry Pi3 and works efficiently.</br>

