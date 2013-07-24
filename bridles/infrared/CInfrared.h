/**
 * 456789------------------------------------------------------------------------------------------------------------120
 *
 * @brief ...
 * @file CInfrared.h
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
 * @date      Jul 4, 2013
 * @project   Replicator 
 * @company   Almende B.V.
 * @company   Distributed Organisms B.V.
 * @case      Sensor fusion
 */

#ifndef CINFRARED_H_
#define CINFRARED_H_

#include <IRobot.h>
#include <CMotors.h>

#include <vector>

#include <worldfile.h>

#include <CMultiHistogram.h>

class CInfrared {
public:
	CInfrared(RobotBase *robot_base, RobotBase::RobotType robot_type);

	~CInfrared();

	void calibrate(bool turn_around = true);

	int reflective(int i);

	int ambient(int i);

	//! Get distance measurement
	int distance(int i);

	//! Get preferred direction
	void direction(float & angle);
private:
	CMotors *motors;

	RobotBase::RobotType type;

	RobotBase *robot;

	int irled_count;

	bool save_to_file;

	std::vector<int32_t> offset_reflective;

	std::vector<int32_t> offset_ambient;

	CMultiHistogram<int32_t, int32_t> hist_reflective;

	CMultiHistogram<int32_t, int32_t> hist_ambient;

	Worldfile optionfile;
};


#endif /* CINFRARED_H_ */
