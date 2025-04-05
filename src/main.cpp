#include "AudioSplitter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AudioSplitter w;
    w.show();
    return a.exec();
}
