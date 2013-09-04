/**
 * 456789------------------------------------------------------------------------------------------------------------120
 *
 * @brief ...
 * @file LaserScanController.cpp
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
 * @date      Sep 2, 2013
 * @project   Replicator 
 * @company   Almende B.V.
 * @company   Distributed Organisms B.V.
 * @case      Sensor fusion
 */

#include <LaserScanController.h>

#include <syslog.h> // LOG_DEBUG

LaserScanController::LaserScanController(): scan(NULL), imageSem(new sem_t()), image_server(NULL), images(), patch(),
mosaic_image(NULL), streaming(false), motors(NULL), create_mosaic(true) {
	images.resize(4);
}

LaserScanController::~LaserScanController() {
	// flush, because deallocation can go wrong somewhere and we'd have a memory dump
	std::cout << std::endl << std::flush;
	std::cout << "Robot object is automatically deleted by the factory." << std::endl;

	std::cout << "Delete CLaserScan instance" << std::endl;
	delete scan;
	std::cout << "Delete CMotors instance" << std::endl;
	delete motors;
}

void LaserScanController::initRobotPeriphery() {
	robot->SetLEDAll(0, LED_OFF);
	robot->SetLEDAll(1, LED_RED);
	robot->SetLEDAll(2, LED_GREEN);

	std::cout << "Setup laser functionality" << std::endl;
	scan = new CLaserScan(robot, robot_type, 640, 480, 640);
	scan->Init();

	motors = new CMotors(robot, robot_type);
	motors->init();

	if (robot_id == 217) {
		motors->reversed(true);
	}
}

void LaserScanController::motorCommand(MotorCommand &motorCommand) {
	if (motors == NULL) {
		std::cerr << "Motor is null, did you call initRobotPeriphery through sending MSG_INIT!?" << std::endl;
		return;
	}
	motors->setRadianSpeeds(motorCommand.forward, motorCommand.radius);
	usleep(100000);
}

void LaserScanController::sendDetectedObject(MappedObjectPosition &position) {

	// overwrite position.type
	ObjectType object;
	int distance;

	scan->GetRecognizedObject(object, distance);
	switch(object) {
	case O_WALL:
		position.type = WALL;
		break;
	case O_SMALL_STEP:
		position.type = SMALL_STEP;
		break;
	case O_LARGE_STEP:
		position.type = LARGE_STEP;
		break;
	default:
		position.type = UNIDENTIFIED;
		break;
	}

	// overwrite message type
	CMessage msg;
	msg.type = MSG_MAP_DATA;

	// overwrite sender id
	position.mappedBy = robot_id;

	// set relative position
	// assuming that phi is from -pi to +pi, and 0 at [x,y]=[+1,0].
	position.xPosition += std::sin(position.phiPosition) * distance;
	position.yPosition += std::cos(position.phiPosition) * distance;

	// set payload
	msg.len = sizeof(MappedObjectPosition);
	memcpy(msg.data, &position, msg.len);

	// send message
	server->sendMessage(msg);

	// delete payload of message
	if (msg.data != NULL) {
		delete [] msg.data;
	}
}

/**
 * Just prints distance to an object or anything.
 */
void LaserScanController::tick() {
	int distance = 0;
	scan->GetDistance(distance);
	std::cout << "Distance: " << distance << " cm" << std::endl;

	if (streaming) {

		if (create_mosaic) {

			if (log_level >= LOG_DEBUG) std::cout << "Compress images so they fit one mosaic image" << std::endl;
			// fill for patches
			for (int i = 0; i < 4; i++) {
				images[i]->compress(patch[i]);
			}

			mosaic_image->setPatch(0, 0, patch[0]);
			mosaic_image->setPatch(0, 1, patch[1]);
			mosaic_image->setPatch(1, 0, patch[2]);
			mosaic_image->setPatch(1, 1, patch[3]);

			if (log_level >= LOG_DEBUG) std::cout << "Written all subimages to one image" << std::endl;

		} else {
			// pick one of the images
			mosaic_image = images[2];
		}

		if (sem_post(imageSem) == -1) {
			std::cerr << "Fail to sem_post image semaphore" << std::endl;
		} else {
			if (log_level >= LOG_DEBUG) std::cout << "Signalled CImageServer through semaphore" << std::endl;
		}
	}

	// every 0.1 seconds
	usleep(100000);
}

void LaserScanController::pause() {
	if (motors != NULL) {
		if (log_level >= LOG_DEBUG) std::cout << "Stop motors" << std::endl;
		motors->set_to_zero();
//		motors->halt();
		sleep(1);
	}
	CController::pause();
}

void LaserScanController::startVideoStream(std::string port) {
	if (log_level >= LOG_DEBUG) printf("%s(): configure streaming of images...\n", __func__);

	mosaic_image = new CRawImage(640,480,3);

	assert (scan != NULL);
	images[0] = scan->getRedDiffImg();
	images[1] = scan->getRGBDiffImg();
	images[2] = scan->getImg1();
	images[3] = scan->getImg2();

	for (int i = 0; i < 4; i++)
		assert (images[i] != NULL);

	if (create_mosaic) {
		if (log_level >= LOG_DEBUG) printf("%s(): create mosaic image...\n", __func__);
		//			if (log_level >= LOG_DEBUG) printf("%s(): initialize patches...\n", __func__);
		for (int i = 0; i < 4; i++) {
			patch[i].init(640/2, 480/2);
		}

	}

	sem_init(imageSem, 0, 0); // do not send first image, only at sem_post, see below

	image_server = new CImageServer(imageSem, mosaic_image);
	image_server->initServer(port.c_str());

	if (log_level >= LOG_DEBUG) printf("%s(): create semaphore for streaming images at the right moment...\n", __func__);

	streaming = true;
}

void LaserScanController::stopVideoStream() {
	if (mosaic_image != NULL)
		delete mosaic_image;

	if (create_mosaic) {
		for (int i = 0; i < 4; i++) {
			patch[i].free();
		}
	}

	if (log_level >= LOG_DEBUG) printf("%s(): stop image server...\n", __func__);

	image_server->stopServer();

	streaming = false;
}

void LaserScanController::testCamera() {
	int cameraDeviceHandler;
	int imgWidth = 640;
	int imgHeight = 480;
	int bpp = 3;
	CCamera camera;
	camera.Init("/dev/video0", cameraDeviceHandler, imgWidth, imgHeight);
	CRawImage *image = new CRawImage(imgWidth, imgHeight, bpp);
	camera.renewImage(image, true);

	image->plotCenter();
	//	image->plotLine(30,30);
	image->saveBmp("test_camera.bmp");
	sleep(1);
}