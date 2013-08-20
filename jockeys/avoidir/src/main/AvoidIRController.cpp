/**
 * 456789------------------------------------------------------------------------------------------------------------120
 *
 * @brief ...
 * @file AvoidIRController.cpp
 * 
 * This file is created at Almende B.V. and Distributed Organisms B.V. It is open-source software and belongs to a
 * larger suite of software that is meant for research on self-organization principles and multi-agent systems where
 * learning algorithms are an important aspect.
 *
 * This software is published under the GNU Lesser General Public license (LGPL).
 *
 * It is not possible to add usage restrictions to an open-source license. Nevertheless, we personally strongly object
 * against this software being used for military purposes, factory farming, animal experimentation, and "Universal
 * Declaration of Human Rights" violations.
 *
 * Copyright (c) 2013 Anne C. van Rossum <anne@almende.org>
 *
 * @author    Anne C. van Rossum
 * @date      Aug 16, 2013
 * @project   Replicator 
 * @company   Almende B.V.
 * @company   Distributed Organisms B.V.
 * @case      Sensor fusion
 */

#include <AvoidIRController.h>


AvoidIRController::AvoidIRController(): port(""), server(NULL), robot(NULL), robot_type(RobotBase::UNKNOWN),
motors(NULL), leds(NULL) {

}

AvoidIRController::~AvoidIRController() {

}

//! First get the port of the jockey
void AvoidIRController::parsePort(int argc, char **argv) {
	std::string port = "";
	if (argc <= 1) {
		std::cout << DEBUG << "First parameter must be the port the jockey can be reached on" << std::endl;
		exit(EXIT_FAILURE);
	}
	port = std::string(argv[1]);
}

//! Create server and start it
void AvoidIRController::initServer() {
	std::cout << "Create (receiving) message server on port " << port << std::endl;
	server = new CMessageServer();
	server->initServer(port.c_str());
}

void AvoidIRController::initRobot() {
	robot_type = RobotBase::Initialize(NAME);
	robot = RobotBase::Instance();
	for (int i = 0; i < 4; ++i)
		robot->SetPrintEnabled(i, false);
	std::cout << "Initialized robot of type " << RobotTypeStr[robot_type] << std::endl;
}

void AvoidIRController::initRobotPeriphery() {
	// we need to initialize the motors before calibrate leds (which turns the robot around)
	motors = new CMotors(robot, robot_type);
	motors->init();

	std::cout << "Setup leds functionality" << std::endl;
	leds = new CLeds(robot, robot_type);
	leds->init();
}

void AvoidIRController::acknowledge() {
	server->sendMessage(MSG_ACKNOWLEDGE, NULL, 0);
}

void AvoidIRController::pause() {
	robot->pauseSPI(true);
	usleep(10000);
}

void AvoidIRController::start() {
	robot->pauseSPI(false);
	usleep(10000);
}

/**
 * The main function of the controller.
 */
void AvoidIRController::tick() {
	// by default straight forward
	int speed = 40;
	int radius = 1000;

	// sample the LEDs for a while
	for (int s = 0; s < leds->get_window_size(); ++s)  {
		leds->update();
		//usleep(100000); // 1000 * 100 is every 0.1seconds
		//		sleep(1);
	}
	if (leds->collision()) {
		// two seconds in reverse
		std::cout << "Go back and rotate for 60 degrees" << std::endl;
		motors->setRadianSpeeds(-speed, 1000);
		sleep(2);
		// and rotate
		motors->rotate(60);
	} else {
		motors->setRadianSpeeds(speed, radius);
	}
	//usleep(100000); // 1000 * 100 is every 0.1seconds
	//	std::cout << "Send wheel commands [" << speed << ',' << radius << ']' << std::endl;
	//	motors->setSpeeds(speed, radius);
}

/**
 * Set the speed to zero, but also halt the motors (and turn them off).
 */
void AvoidIRController::graceful_end() {
	motors->setSpeeds(0, 0);
	sleep(1);
	motors->halt();
	sleep(1);

	std::cout << NAME << " quits" << std::endl;
	//exit(EXIT_SUCCESS);
}

/**
 * Show that the controller ended properly by a LED show.
 */
void AvoidIRController::signal_end() {
	leds->color(LC_ORANGE);
	sleep(1);
	leds->color(LC_GREEN);
}

void AvoidIRController::calibrate() {
	std::cout << "Calibrate!" << std::endl;
	leds->calibrate();
	std::cout << "Calibration done" << std::endl;
	graceful_end();
}

void AvoidIRController::get_calibration() {
	std::cout << "Get calibration values" << std::endl;
	leds->get_calibration();
	std::cout << "Sliding window size used of " << leds->get_window_size() << std::endl;
}

