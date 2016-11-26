#pragma once

#include <QObject>
#include <QString>
#include <QThread>

#include "Direction.h"
#include "DynamicMouseAlgorithmOptions.h"
#include "Model.h"
#include "MouseInterface.h"
#include "StaticMouseAlgorithmOptions.h"
#include "View.h"
#include "Worker.h"

namespace mms {

class Controller : public QObject {

    Q_OBJECT

public:

    Controller(Model* model, View* view);

    // TODO: MACK
    // ~Controller() {
    //     workerThread.quit();
    //     workerThread.wait();
    // }

    MouseInterface* getMouseInterface();

    StaticMouseAlgorithmOptions getStaticOptions();
    DynamicMouseAlgorithmOptions getDynamicOptions();

// TODO: MACK
// private:

    // TODO: MACK - make this thread more visible
    Worker* m_worker;
    QThread m_workerThread;

    // The model (used for relaying formation to the algo)
    Model* m_model;

    // The interface by which we access the Mouse object
    MouseInterface* m_mouseInterface;

    // The algorithm options
    StaticMouseAlgorithmOptions m_staticOptions;
    DynamicMouseAlgorithmOptions m_dynamicOptions;

    // Whether or not the static options have been finalized
    bool m_staticOptionsFinalized;

    // TODO: MACK = should be private
    Direction getInitialDirection(
        const QString& initialDirection,
        Model* model);


// TODO: MACK
private:

    // TODO: MACK - do validation in some other class

    void validateMouseInterfaceType(
        const QString& mouseAlgorithm,
        const QString& interfaceType);

    void validateMouseInitialDirection(
        const QString& mouseAlgorithm,
        const QString& initialDirection);

    void validateTileTextRowsAndCols(
        const QString& mouseAlgorithm,
        int tileTextNumberOfRows,
        int tileTextNumberOfCols);

    void validateMouseWheelSpeedFraction(
        const QString& mouseAlgorithm,
        double wheelSpeedFraction);

    void initAndValidateMouse(
        const QString& mouseAlgorithm,
        const QString& mouseFile,
        const QString& interfaceType,
        const QString& initialDirection,
        Model* model);

};

} // namespace mms
