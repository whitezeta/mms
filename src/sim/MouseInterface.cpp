#include "MouseInterface.h"

#include "units/Meters.h"
#include "units/MetersPerSecond.h"
#include "units/Milliseconds.h"
#include "units/Seconds.h"

#include "Assert.h"
#include "Color.h"
#include "CPMath.h"
#include "Logging.h"
#include "Param.h"
#include "State.h"
#include "SimUtilities.h"

// Needs to be included last for Windows compatibility
#include "CPMinMax.h"

namespace sim {

MouseInterface::MouseInterface(const Maze* maze, Mouse* mouse, MazeGraphic* mazeGraphic) :
        m_maze(maze), m_mouse(mouse), m_mazeGraphic(mazeGraphic) {
}

void MouseInterface::delay(int milliseconds) {
    sim::SimUtilities::sleep(Milliseconds(milliseconds));
}

void MouseInterface::setTileColor(int x, int y, char color) {

    if (!withinMaze(x, y)) {

#ifndef _WIN32 // TODO
        Logging::simLogger()->warn(
            "There is no tile at position (%v, %v) and thus you cannot set its "
            "color.",
            x, y);
#endif
        return;
    }

    if (!SimUtilities::mapContains(CHAR_TO_COLOR, color)) {
#ifndef _WIN32 // TODO
        Logging::simLogger()->warn(
            "You cannot set the color of tile (%v, %v) to '%v' since '%v' is "
            "not mapped to a color.",
            x, y, color, color);
#endif
        return;
    }

    m_mazeGraphic->setTileColor(x, y, CHAR_TO_COLOR.at(color));
    m_tilesWithColor.insert(std::make_pair(x, y));
}

void MouseInterface::clearTileColor(int x, int y) {

    if (!withinMaze(x, y)) {
#ifndef _WIN32 // TODO
        Logging::simLogger()->warn(
            "There is no tile at position (%v, %v) and thus you cannot clear its "
            "color.",
            x, y);
#endif
        return;
    }

    m_mazeGraphic->setTileColor(x, y, STRING_TO_COLOR.at(P()->tileBaseColor()));
    m_tilesWithColor.erase(std::make_pair(x, y));
}

void MouseInterface::clearAllTileColor() {

    for (std::pair<int, int> position : m_tilesWithColor) {
        m_mazeGraphic->setTileColor(position.first, position.second, STRING_TO_COLOR.at(P()->tileBaseColor()));
    }
    m_tilesWithColor.clear();
}

void MouseInterface::declareWall(int x, int y, char direction, bool wallExists) {

    if (!withinMaze(x, y)) {
        SimUtilities::print(std::string("Error: There is no tile at position (") + std::to_string(x) + std::string(", ")
            + std::to_string(y) + std::string("), and thus you cannot declare any of its walls."));
        return;
    }

    if (!SimUtilities::mapContains(CHAR_TO_DIRECTION, direction)) {
        SimUtilities::print(std::string("The character '") + direction + std::string("' is not mapped to a valid direction."));
        return;
    }

    m_mazeGraphic->declareWall(x, y, CHAR_TO_DIRECTION.at(direction), wallExists); 
    if (P()->declareBothWallHalves() && hasOpposingWall(x, y, CHAR_TO_DIRECTION.at(direction))) {
        std::pair<std::pair<int, int>, Direction> opposing = getOpposingWall(x, y, CHAR_TO_DIRECTION.at(direction));
        m_mazeGraphic->declareWall(opposing.first.first, opposing.first.second, opposing.second, wallExists); 
    }
}

void MouseInterface::undeclareWall(int x, int y, char direction) {

    if (!withinMaze(x, y)) {
        SimUtilities::print(std::string("Error: There is no tile at position (") + std::to_string(x) + std::string(", ")
            + std::to_string(y) + std::string("), and thus you cannot undeclare any of its walls."));
        return;
    }

    if (!SimUtilities::mapContains(CHAR_TO_DIRECTION, direction)) {
        SimUtilities::print(std::string("The character '") + direction + std::string("' is not mapped to a valid direction."));
        return;
    }

    m_mazeGraphic->undeclareWall(x, y, CHAR_TO_DIRECTION.at(direction));
    if (P()->declareBothWallHalves() && hasOpposingWall(x, y, CHAR_TO_DIRECTION.at(direction))) {
        std::pair<std::pair<int, int>, Direction> opposing = getOpposingWall(x, y, CHAR_TO_DIRECTION.at(direction));
        m_mazeGraphic->undeclareWall(opposing.first.first, opposing.first.second, opposing.second);
    }
}

void MouseInterface::setTileFogginess(int x, int y, bool foggy) {

    if (!withinMaze(x, y)) {
        SimUtilities::print(std::string("Error: There is no tile at position (") + std::to_string(x) + std::string(", ")
            + std::to_string(y) + std::string("), and thus you cannot set its fogginess."));
        return;
    }

    if (!P()->algorithmControlsTileFog()) { 
        // TODO: MACK - error statement
        return;
    }

    m_mazeGraphic->setTileFogginess(x, y, foggy);
}

void MouseInterface::declareTileDistance(int x, int y, int distance) {

    if (!withinMaze(x, y)) {
        SimUtilities::print(std::string("Error: There is no tile at position (") + std::to_string(x) + std::string(", ")
            + std::to_string(y) + std::string("), and thus you cannot set its distance."));
        return;
    }

    m_mazeGraphic->setTileText(x, y, {std::to_string(distance)});
}

void MouseInterface::undeclareTileDistance(int x, int y) {

    if (!withinMaze(x, y)) {
        SimUtilities::print(std::string("Error: There is no tile at position (") + std::to_string(x) + std::string(", ")
            + std::to_string(y) + std::string("), and thus you cannot clear its distance."));
        return;
    }

    m_mazeGraphic->setTileText(x, y, {});
}

void MouseInterface::resetPosition() {
    m_mouse->teleport(m_mouse->getInitialTranslation(), Radians(0.0));
}

bool MouseInterface::inputButtonPressed(int inputButton) {

    if (inputButton < 0 || 9 < inputButton) {
        SimUtilities::print(std::string("Error: There is no input button with the number ") + std::to_string(inputButton)
            + std::string(", and thus you cannot check to see if it has been pressed."));
        return false;
    }

    return S()->inputButtonWasPressed(inputButton);
}

void MouseInterface::acknowledgeInputButtonPressed(int inputButton) {

    if (inputButton < 0 || 9 < inputButton) {
        SimUtilities::print(std::string("Error: There is no input button with the number ") + std::to_string(inputButton)
            + std::string(", and thus you cannot acknowledge that it has been pressed."));
        return;
    }

    S()->setInputButtonWasPressed(inputButton, false);
}

void MouseInterface::setWheelSpeeds(double leftWheelRadiansPerSecond, double rightWheelRadiansPerSecond) {

    ENSURE_CONTINUOUS_INTERFACE

    m_mouse->setWheelSpeeds(RadiansPerSecond(leftWheelRadiansPerSecond), RadiansPerSecond(rightWheelRadiansPerSecond));
}

double MouseInterface::read(std::string name) {

    ENSURE_CONTINUOUS_INTERFACE

    if (!m_mouse->hasSensor(name)) {
        SimUtilities::print(std::string("Error: There is no sensor called \"") + std::string(name)
            + std::string("\" and thus you cannot read its value."));
        return 0.0;
    }

    // Start the timer
    double start(sim::SimUtilities::getHighResTime());

    // Retrieve the value
    double value = m_mouse->read(name);

    // Stop the timer
    double end(sim::SimUtilities::getHighResTime());
    double duration = end - start;

    // Display to the user, if requested
    if (sim::P()->printLateSensorReads() && duration > m_mouse->getReadDuration(name).getSeconds()) {
        sim::SimUtilities::print(std::string("A sensor read was late by ")
            + std::to_string(duration - m_mouse->getReadDuration(name).getSeconds())
            + std::string(" seconds, which is ")
            + std::to_string((duration - m_mouse->getReadDuration(name).getSeconds())/(m_mouse->getReadDuration(name).getSeconds()) * 100)
            + std::string(" percent late."));
    }

    // Sleep for the read time
    sim::SimUtilities::sleep(sim::Seconds(std::max(0.0, 1.0/sim::P()->frameRate() - duration)));

    // Return the value
    return value;
}

double MouseInterface::readGyro() {

    ENSURE_CONTINUOUS_INTERFACE

    return m_mouse->readGyro().getDegreesPerSecond();
}

bool MouseInterface::wallFront() {

    ENSURE_DISCRETE_INTERFACE

    return isWall(getDiscretizedTranslation(), getDiscretizedRotation());
}

bool MouseInterface::wallRight() {

    ENSURE_DISCRETE_INTERFACE

    std::pair<int, int> position = getDiscretizedTranslation();

    switch (getDiscretizedRotation()) {
        case Direction::NORTH:
            return isWall(position, Direction::EAST);
        case Direction::EAST:
            return isWall(position, Direction::SOUTH);
        case Direction::SOUTH:
            return isWall(position, Direction::WEST);
        case Direction::WEST:
            return isWall(position, Direction::NORTH);
    }
}

bool MouseInterface::wallLeft() {

    ENSURE_DISCRETE_INTERFACE

    std::pair<int, int> position = getDiscretizedTranslation();

    switch (getDiscretizedRotation()) {
        case Direction::NORTH:
            return isWall(position, Direction::WEST);
        case Direction::EAST:
            return isWall(position, Direction::NORTH);
        case Direction::SOUTH:
            return isWall(position, Direction::EAST);
        case Direction::WEST:
            return isWall(position, Direction::SOUTH);
    }
}

void MouseInterface::moveForward() {

    ENSURE_DISCRETE_INTERFACE

    if (wallFront()) {
        if (!S()->crashed()) {
            S()->setCrashed();
        }
        return;
    }

    // TODO: Check speed here, skip logic and just teleport if too fast (then sleep for the remaining)
    // TODO: Do the same for other movement methods
    // TODO: Try to increase max speed
    //if (S()->simSpeed() < X) {

    Meters tileLength = Meters(P()->wallLength() + P()->wallWidth());
    Meters currentX = tileLength * (getDiscretizedTranslation().first) + m_mouse->getInitialTranslation().getX();
    Meters currentY = tileLength * (getDiscretizedTranslation().second) + m_mouse->getInitialTranslation().getY();

    // We modify these values in the switch statement
    Cartesian destinationTranslation = Cartesian(currentX, currentY);
    Degrees destinationRotation(0.0);

    switch (getDiscretizedRotation()) {
        case Direction::NORTH: {
            destinationTranslation += Cartesian(Meters(0), tileLength);
            destinationRotation = Degrees(0);
            while (m_mouse->getCurrentTranslation().getY() < destinationTranslation.getY()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()), RadiansPerSecond(S()->simSpeed()));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::EAST: {
            destinationTranslation += Cartesian(tileLength, Meters(0));
            destinationRotation = Degrees(270);
            while (m_mouse->getCurrentTranslation().getX() < destinationTranslation.getX()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()), RadiansPerSecond(S()->simSpeed()));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::SOUTH: {
            destinationTranslation += Cartesian(Meters(0), tileLength * -1);
            destinationRotation = Degrees(180);
            while (destinationTranslation.getY() < m_mouse->getCurrentTranslation().getY()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()), RadiansPerSecond(S()->simSpeed()));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::WEST: {
            destinationTranslation += Cartesian(tileLength * -1, Meters(0));
            destinationRotation = Degrees(90);
            while (destinationTranslation.getX() < m_mouse->getCurrentTranslation().getX()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()), RadiansPerSecond(S()->simSpeed()));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
    }

    m_mouse->setWheelSpeeds(RadiansPerSecond(0), RadiansPerSecond(0));
    m_mouse->teleport(destinationTranslation, destinationRotation);
}

void MouseInterface::turnRight() {

    ENSURE_DISCRETE_INTERFACE

    Cartesian destinationTranslation = m_mouse->getCurrentTranslation();
    Degrees destinationRotation = m_mouse->getCurrentRotation() - Degrees(90);

    switch (getDiscretizedRotation()) {
        case Direction::NORTH: {
            while (destinationRotation < m_mouse->getCurrentRotation() || m_mouse->getCurrentRotation() < Degrees(180)) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(S()->simSpeed()/2.0), RadiansPerSecond(S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::EAST:
        case Direction::SOUTH: {
            while (destinationRotation < m_mouse->getCurrentRotation()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(S()->simSpeed()/2.0), RadiansPerSecond(S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::WEST: {
            while (m_mouse->getCurrentRotation() < Degrees(180)) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(S()->simSpeed()/2.0), RadiansPerSecond(S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
    }

    m_mouse->setWheelSpeeds(RadiansPerSecond(0), RadiansPerSecond(0));
    m_mouse->teleport(destinationTranslation, destinationRotation);
}

void MouseInterface::turnLeft() {

    ENSURE_DISCRETE_INTERFACE

    Cartesian destinationTranslation = m_mouse->getCurrentTranslation();
    Degrees destinationRotation = m_mouse->getCurrentRotation() + Degrees(90);

    switch (getDiscretizedRotation()) {
        case Direction::NORTH: {
            while (m_mouse->getCurrentRotation() < destinationRotation ||  Degrees(180) < m_mouse->getCurrentRotation()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()/2.0), RadiansPerSecond(-S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::EAST: {
            while (Degrees(180) < m_mouse->getCurrentRotation()) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()/2.0), RadiansPerSecond(-S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
        case Direction::SOUTH:
        case Direction::WEST: {
            while (m_mouse->getCurrentRotation() < destinationRotation) {
                checkPaused();
                m_mouse->setWheelSpeeds(RadiansPerSecond(-S()->simSpeed()/2.0), RadiansPerSecond(-S()->simSpeed()/2.0));
                sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
            }
            break;
        }
    }

    m_mouse->setWheelSpeeds(RadiansPerSecond(0), RadiansPerSecond(0));
    m_mouse->teleport(destinationTranslation, destinationRotation);
}

void MouseInterface::turnAround() {

    ENSURE_DISCRETE_INTERFACE

    turnRight();
    turnRight();
}

void MouseInterface::ensureDiscreteInterface(const std::string& callingFunction) const {
    if (S()->interfaceType() != InterfaceType::DISCRETE) {
        SimUtilities::print(std::string("Error: You must declare the interface type to be sim::InterfaceType::DISCRETE to use MouseInterface::")
            + callingFunction + std::string("()."));
        SimUtilities::quit();
    }
}

void MouseInterface::ensureContinuousInterface(const std::string& callingFunction) const {
    if (S()->interfaceType() != InterfaceType::CONTINUOUS) {
        SimUtilities::print(std::string("Error: You must declare the interface type to be sim::InterfaceType::CONTINUOUS to use MouseInterface::")
            + callingFunction + std::string("()."));
        SimUtilities::quit();
    }
}

void MouseInterface::checkPaused() const {
    if (S()->paused()) {
        m_mouse->setWheelSpeeds(RadiansPerSecond(0), RadiansPerSecond(0));
        while (S()->paused()) {
            sim::SimUtilities::sleep(Milliseconds(P()->minSleepDuration()));
        }
    }
}

bool MouseInterface::isWall(std::pair<int, int> position, Direction direction) {

    ASSERT(0 <= position.first && position.first < m_maze->getWidth()
        && 0 <= position.second && position.second < m_maze->getHeight());

    bool wallExists = m_maze->getTile(position.first, position.second)->isWall(direction);

    if (P()->discreteInterfaceDeclareWallOnRead()) {
        declareWall(position.first, position.second, DIRECTION_TO_CHAR.at(direction), wallExists);
    }

    return wallExists;
}

bool MouseInterface::withinMaze(int x, int y) const {
    return 0 <= x && x < m_maze->getWidth() && 0 <= y && y < m_maze->getHeight();
}

bool MouseInterface::hasOpposingWall(int x, int y, Direction direction) const {
    switch (direction) {
        case Direction::NORTH:
            return y < m_maze->getHeight() - 1;
        case Direction::EAST:
            return x < m_maze->getWidth() - 1;
        case Direction::SOUTH:
            return y > 0;
        case Direction::WEST:
            return x > 0;
    }
}

std::pair<std::pair<int, int>, Direction> MouseInterface::getOpposingWall(int x, int y, Direction direction) const {
    ASSERT(hasOpposingWall(x, y, direction));
    switch (direction) {
        case Direction::NORTH:
            return std::make_pair(std::make_pair(x, y + 1), Direction::SOUTH);
        case Direction::EAST:
            return std::make_pair(std::make_pair(x + 1, y), Direction::WEST);
        case Direction::SOUTH:
            return std::make_pair(std::make_pair(x, y - 1), Direction::NORTH);
        case Direction::WEST:
            return std::make_pair(std::make_pair(x - 1, y), Direction::EAST);
    }
}

std::pair<int, int> MouseInterface::getDiscretizedTranslation() const {
    return m_mouse->getDiscretizedTranslation();
}

Direction MouseInterface::getDiscretizedRotation() const {
    return m_mouse->getDiscretizedRotation();
}

} // namespace sim
