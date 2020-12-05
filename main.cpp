#include <QCoreApplication>
#include "alsatranslator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    auto google_speech = new AlsaTranslator(nullptr);

    QObject::connect(google_speech, &AlsaTranslator::speechChanged, [](auto speech)
    {
        qDebug() << speech;
    });

    google_speech->setRecordDuration(2000);

    google_speech->start();

    return a.exec();
}
