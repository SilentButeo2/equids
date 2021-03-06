/**
 * 456789------------------------------------------------------------------------------------------------------------120
 *
 * @brief Test the laser
 * @file lasertest.cpp
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
 * @date      Jul 30, 2012
 * @project   Replicator
 * @company   Almende B.V.
 * @company   Distributed Organisms B.V.
 * @case      Testing
 */

#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>

/***********************************************************************************************************************
 * Middleware includes
 **********************************************************************************************************************/

#include <IRobot.h>

#include <comm/IRComm.h>

/***********************************************************************************************************************
 * Jockey framework includes
 **********************************************************************************************************************/

#include <CLaser.h>

/***********************************************************************************************************************
 * Implementation
 **********************************************************************************************************************/

//! The name of the controller can be used for controller selection
std::string NAME = "LaserTest";

/**
 * If the user presses Ctrl+C, this can be used to do memory deallocation or a last communication with the MSPs.
 */
void interrupt_signal_handler(int signal) {
	if (signal == SIGINT) {
		//RobotBase::MSPReset();
		exit(0);
	}
}

/**
 * Basically only turns on and off the laser for a couple of times.
 */
int main(int argc, char **argv) {
	int nof_switches = 10;

	struct sigaction a;
	a.sa_handler = &interrupt_signal_handler;
	sigaction(SIGINT, &a, NULL);

//	IRobotFactory factory;
//	RobotBase* robot = factory.GetRobot();
//	RobotBase::RobotType robot_type = factory.GetType();


	// old irobot
	RobotBase::RobotType robot_type = RobotBase::Initialize(NAME);
	RobotBase* robot = RobotBase::Instance();
	for (int i = 0; i < 4; ++i)
		robot->SetPrintEnabled(i, false);


	switch(robot_type) {
	case RobotBase::UNKNOWN: std::cout << "Detected unknown robot" << std::endl; break;
	case RobotBase::KABOT: std::cout << "Detected Karlsruhe robot" << std::endl; break;
	case RobotBase::ACTIVEWHEEL: std::cout << "Detected Active Wheel robot" << std::endl; break;
	case RobotBase::SCOUTBOT: std::cout << "Detected Scout robot" << std::endl; break;
	default:
		std::cout << "No known type (even not unknown). Did initialization go well?" << std::endl;
	}

	std::string msg = "test";
	IRComm::SendMessage(0, msg.c_str(), msg.size());

	std::cout << "Setup laser functionality" << std::endl;
	CLaser laser(robot, robot_type);
	for (int i = 0; i < nof_switches; ++i) {
		(i % 2) ? laser.Off() : laser.On();
		sleep(2);
	}

	fprintf(stdout,"Stopping laser test.");
	return 0;
}


