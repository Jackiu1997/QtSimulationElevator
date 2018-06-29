#include "simulationwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimulationWindow w;
    w.show();

    return a.exec();
}
