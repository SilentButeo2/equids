#include "CCircleDetect.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

int* CCircleDetect::buffer = NULL;
int* CCircleDetect::queue = NULL;

//Variable initialization
CCircleDetect::CCircleDetect(int wi, int he, float diamRatio) {
	lastTrackOK = false;
	debug = false;
	draw = false;
	drawAll = false;
	maxFailed = 0;
	minSize = 10;
	thresholdStep = 256;
	maxThreshold = 3 * 256;
	centerDistanceToleranceRatio = 1.1; //TODO
	centerDistanceToleranceAbs = 5;
	circularTolerance = 0.3;
	ratioTolerance = 0.4;
	threshold = maxThreshold / 2;
	numFailed = maxFailed;
	track = true;
	//circularityTolerance = 0.02;
	circularityTolerance = 0.1;

	//initialization - fixed params
	width = wi;
	height = he;
	len = width * height;
	siz = len * 3;
	ownBuffer = false;
	if (buffer == NULL) {
		ownBuffer = true;
		buffer = (int*) malloc(len * sizeof(int));
		queue = (int*) malloc(len * sizeof(int));
		SSegment dummy;
		bufferCleanup(dummy);
	}
	//diameterRatio = 5.0/14.0; //inner vs. outer circle diameter
	diameterRatio = diamRatio;
	float areaRatioInner_Outer = diameterRatio * diameterRatio;
	outerAreaRatio = M_PI * (1.0 - areaRatioInner_Outer) / 4;
	innerAreaRatio = M_PI / 4.0;
	areasRatio = (1.0 - areaRatioInner_Outer) / areaRatioInner_Outer;
	tima = timb = timc = timd = sizer = sizerAll = 0;
}

CCircleDetect::~CCircleDetect() {
//	if (debug) printf("Timi %i %i %i %i\n",tima,timb,sizer,sizerAll);
	if (ownBuffer) {
		free(buffer);
		free(queue);
	}
}

void CCircleDetect::setDiameterRatio(float diameter) {
	this->diameterRatio = diameter;
}

bool CCircleDetect::changeThreshold() {
	int div = 1;
	int dum = numFailed;
	while (dum > 1) {
		dum = dum / 2;
		div *= 2;
	}
	int step = 256 / div;
	threshold = 3 * (step * (numFailed - div) + step / 2);
	//if (debug) fprintf(stdout,"Threshold: %i %i %i\n",div,numFailed,threshold/3);
	return step > 16;
}

bool CCircleDetect::examineSegment(CRawImage *image, SSegment *segmen, int ii,
		float areaRatio) {
	timer.reset();
	timer.start();
	int vx, vy;
	queueOldStart = queueStart;
	int position = 0;
	int pos;
	bool result = false;
	int type = buffer[ii];
	int maxx, maxy, minx, miny;

	buffer[ii] = ++numSegments;
	segmen->x = ii % width;
	segmen->y = ii / width;
	minx = maxx = segmen->x;
	miny = maxy = segmen->y;
	segmen->valid = false;
	segmen->round = false;
	//push segment coords to the queue
	queue[queueEnd++] = ii;
	//and until queue is empty
	while (queueEnd > queueStart) {
		//pull the coord from the queue
		position = queue[queueStart++];
		//search neighbours
		pos = position + 1;
		if (buffer[pos] == 0) {
			ptr = &image->data[pos * 3];
			buffer[pos] = ((ptr[0] + ptr[1] + ptr[2]) > threshold) - 2;
		}
		if (buffer[pos] == type) {
			queue[queueEnd++] = pos;
			maxx = max(maxx,pos%width);
			buffer[pos] = numSegments;
		}
		pos = position - 1;
		if (buffer[pos] == 0) {
			ptr = &image->data[pos * 3];
			buffer[pos] = ((ptr[0] + ptr[1] + ptr[2]) > threshold) - 2;
		}
		if (buffer[pos] == type) {
			queue[queueEnd++] = pos;
			minx = min(minx,pos%width);
			buffer[pos] = numSegments;
		}
		pos = position - width;
		if (buffer[pos] == 0) {
			ptr = &image->data[pos * 3];
			buffer[pos] = ((ptr[0] + ptr[1] + ptr[2]) > threshold) - 2;
		}
		if (buffer[pos] == type) {
			queue[queueEnd++] = pos;
			miny = min(miny,pos/width);
			buffer[pos] = numSegments;
		}
		pos = position + width;
		if (buffer[pos] == 0) {
			ptr = &image->data[pos * 3];
			buffer[pos] = ((ptr[0] + ptr[1] + ptr[2]) > threshold) - 2;
		}
		if (buffer[pos] == type) {
			queue[queueEnd++] = pos;
			maxy = max(maxy,pos/width);
			buffer[pos] = numSegments;
		}
	}

	//once the queue is empty, i.e. segment is complete, we compute its size 
	segmen->size = queueEnd - queueOldStart;
	if (segmen->size > minSize) {
		//and if its large enough, we compute its other properties 
		segmen->maxx = maxx;
		segmen->maxy = maxy;
		segmen->minx = minx;
		segmen->miny = miny;
		segmen->type = -type;
		vx = (segmen->maxx - segmen->minx + 1);
		vy = (segmen->maxy - segmen->miny + 1);
		segmen->x = (segmen->maxx + segmen->minx) / 2;
		segmen->y = (segmen->maxy + segmen->miny) / 2;
		segmen->roundness = vx * vy * areaRatio / segmen->size;
		//we check if the segment is likely to be a ring 
		if (segmen->roundness - circularTolerance < 1.0
				&& segmen->roundness + circularTolerance > 1.0) {
			//if its round, we compute yet another properties 
			segmen->round = true;
			segmen->mean = 0;
			for (int p = queueOldStart; p < queueEnd; p++) {
				pos = queue[p];
				segmen->mean += image->data[pos * 3] + image->data[pos * 3 + 1]
						+ image->data[pos * 3 + 2];
			}
			segmen->mean = segmen->mean / segmen->size;
			result = true;
		}
	}
	timb += timer.getTime();
	return result;
}

void CCircleDetect::bufferCleanup(SSegment init) {
	int pos = (height - 1) * width;
	if (init.valid == false || track == false || lastTrackOK == false) {
		memset(buffer, 0, sizeof(int) * len);
		for (int i = 0; i < width; i++) {
			buffer[i] = -1000;
			buffer[pos + i] = -1000;
		}
		for (int i = 0; i < height; i++) {
			buffer[width * i] = -1000;
			buffer[width * i + width - 1] = -1000;
		}
	} else {
		int pos, ix, ax, iy, ay;
		ix = max(init.minx - 2,1);
		ax = min(init.maxx + 2,width-2);
		iy = max(init.miny - 2,1);
		ay = min(init.maxy + 2,height-2);
		for (int y = iy; y < ay; y++) {
			pos = y * width;
			for (int x = ix; x < ax; x++)
				buffer[pos + x] = 0;
		}
	}
}

SSegment CCircleDetect::findSegment(CRawImage* image, SSegment init) {//printf("findingSegment\n");
	SSegment result;
	result.x = 0;
	result.y = 0;
	result.round = false;
	result.valid = false;
	numSegments = 0;

	timer.reset();
	timer.start();
	tima += timer.getTime();
	int pos = 0;
	int ii = 0;
	int start = 0;
	bool cont = true;

	//bufferCleanup(init);
	if (init.valid && track) {
		ii = ((int) init.y) * image->getwidth() + init.x;
		start = ii;
	}
	while (cont) {
		if (buffer[ii] == 0) {
			ptr = &image->data[ii * 3];
			//buffer[ii]=((ptr[0]+ptr[1]+ptr[2]) > threshold)-2;
			if ((ptr[0] + ptr[1] + ptr[2]) < threshold)
				buffer[ii] = -2;
		}
		if (buffer[ii] == -2 && numSegments < MAX_SEGMENTS) {
			//new segment found
			queueEnd = 0;
			queueStart = 0;
			//if the segment looks like a ring, we check its inside area
			if (examineSegment(image, &segmentArray[numSegments], ii,
					outerAreaRatio)) {
				pos = segmentArray[numSegments - 1].y * image->getwidth()
						+ segmentArray[numSegments - 1].x;
				if (buffer[pos] == 0) {
					ptr = &image->data[pos * 3];
					buffer[pos] = ((ptr[0] + ptr[1] + ptr[2]) > threshold) - 2;
				}
				if (buffer[pos] == -1 && numSegments < MAX_SEGMENTS) {
					if (examineSegment(image, &segmentArray[numSegments], pos,
							innerAreaRatio)) {
						//the inside area is a circle. now what is the area ratio of the black and white ? also, are the circles concentric ?

						if (debug)
							printf("Area ratio %i/%i is %.3f %.3f %.3f\n",
									numSegments - 2, numSegments - 1,
									(float) segmentArray[numSegments - 2].size
											/ segmentArray[numSegments - 1].size,
									areasRatio,
									segmentArray[numSegments - 2].size
											/ areasRatio
											/ segmentArray[numSegments - 1].size);
						if (((float) segmentArray[numSegments - 2].size
								/ areasRatio
								/ (float) segmentArray[numSegments - 1].size
								- ratioTolerance < 1.0
								&& (float) segmentArray[numSegments - 2].size
										/ areasRatio
										/ (float) segmentArray[numSegments - 1].size
										+ ratioTolerance > 1.0)
								&& (abs(
										segmentArray[numSegments - 1].x
												- segmentArray[numSegments - 2].x)
										<= centerDistanceToleranceAbs
												+ centerDistanceToleranceRatio
														* ((float) (segmentArray[numSegments
																- 2].maxx
																- segmentArray[numSegments
																		- 2].minx)))
								&& (abs(
										segmentArray[numSegments - 1].y
												- segmentArray[numSegments - 2].y)
										<= centerDistanceToleranceAbs
												+ centerDistanceToleranceRatio
														* ((float) (segmentArray[numSegments
																- 2].maxy
																- segmentArray[numSegments
																		- 2].miny)))

								) {
							float tx, ty, cm0, cm1, cm2, sx, sy, fm0, fm1, fm2,
									f0, f1;
							sx = sy = cm0 = cm1 = cm2 = 0;
							//	segmentArray[numSegments-1].x = segmentArray[numSegments-2].x;
							//	segmentArray[numSegments-1].y = segmentArray[numSegments-2].y;

							for (int p = queueOldStart; p < queueEnd; p++) {
								pos = queue[p];
								sx += pos % image->getwidth();
								sy += pos / image->getwidth();
							}
							segmentArray[numSegments - 2].x = sx
									/ (queueEnd - queueOldStart);
							segmentArray[numSegments - 2].y = sy
									/ (queueEnd - queueOldStart);
							for (int p = 0; p < queueOldStart; p++) {
								pos = queue[p];
								sx += pos % image->getwidth();
								sy += pos / image->getwidth();
							}
							sx = sx / queueEnd;
							sy = sy / queueEnd;
							for (int p = 0; p < queueEnd; p++) {
								pos = queue[p];
								tx = pos % image->getwidth() - sx;
								ty = pos / image->getwidth() - sy;
								cm0 += tx * tx;
								cm1 += tx * ty;
								cm2 += ty * ty;
							}
							fm0 = cm0 / queueEnd;
							fm1 = cm1 / queueEnd;
							fm2 = cm2 / queueEnd;
							f0 =
									((fm0 + fm2)
											+ sqrt(
													(fm0 + fm2) * (fm0 + fm2)
															- 4
																	* (fm0 * fm2
																			- fm1
																					* fm1)))
											/ 2;
							f1 =
									((fm0 + fm2)
											- sqrt(
													(fm0 + fm2) * (fm0 + fm2)
															- 4
																	* (fm0 * fm2
																			- fm1
																					* fm1)))
											/ 2;
							segmentArray[numSegments - 1].m0 = sqrt(f0);
							segmentArray[numSegments - 1].m1 = sqrt(f1);
							segmentArray[numSegments - 1].v0 = -fm1
									/ sqrt(fm1 * fm1 + (fm0 - f0) * (fm0 - f0));
							segmentArray[numSegments - 1].v1 = (fm0 - f0)
									/ sqrt(fm1 * fm1 + (fm0 - f0) * (fm0 - f0));
							segmentArray[numSegments - 1].bwRatio =
									(float) segmentArray[numSegments - 2].size
											/ segmentArray[numSegments - 1].size;

							if (track)
								ii = start - 1;
							sizer += segmentArray[numSegments - 2].size
									+ segmentArray[numSegments - 1].size; //for debugging
							sizerAll += len; 					//for debugging
							float circularity = M_PI * 4
									* (segmentArray[numSegments - 1].m0)
									* (segmentArray[numSegments - 1].m1)
									/ queueEnd;
							if (debug)
								fprintf(stdout, "Circularity: %f  \n",
										circularity);
							if (circularity - 1.0 < circularityTolerance
									&& circularity - 1.0
											> -circularityTolerance) {
								segmentArray[numSegments - 2].valid =
										segmentArray[numSegments - 1].valid =
												true;
								threshold = (segmentArray[numSegments - 2].mean
										+ segmentArray[numSegments - 1].mean)
										/ 2;
								if (debug)
									fprintf(stdout,
											"Circularity: %i %03f %03f %03f \n",
											queueEnd,
											M_PI * 4
													* (segmentArray[numSegments
															- 1].m0)
													* (segmentArray[numSegments
															- 1].m1) / queueEnd,
											M_PI * 4
													* (segmentArray[numSegments
															- 1].m0)
													* (segmentArray[numSegments
															- 1].m1),
											segmentArray[numSegments - 1].x
													/ 1000.0);

								//pixel leakage correction
								float r = diameterRatio * diameterRatio;
								float m0o = sqrt(f0);
								float m1o = sqrt(f1);
								float ratio =
										(float) segmentArray[numSegments - 1].size
												/ (segmentArray[numSegments - 2].size
														+ segmentArray[numSegments
																- 1].size);
								float m0i = sqrt(ratio) * m0o;
								float m1i = sqrt(ratio) * m1o;
								float a = (1 - r);
								float b = -(m0i + m1i) - (m0o + m1o) * r;
								float c = (m0i * m1i) - (m0o * m1o) * r;
								float t = (-b - sqrt(b * b - 4 * a * c))
										/ (2 * a);
								m0i -= t;
								m1i -= t;
								m0o += t;
								m1o += t;
								//fprintf(stdout,"UUU: %f %f %f %f %f\n",t,ratio,(m1i-t)/(m1o+t)*0.14,(m0i-t)/(m0o+t)*0.14,(m0o*m1o-m0i*m1i)/(m0i*m1i));
								segmentArray[numSegments - 1].m0 = sqrt(f0) + t;
								segmentArray[numSegments - 1].m1 = sqrt(f1) + t;
								segmentArray[numSegments - 1].maxx =
										segmentArray[numSegments - 2].maxx;
								segmentArray[numSegments - 1].maxy =
										segmentArray[numSegments - 2].maxy;
								segmentArray[numSegments - 1].minx =
										segmentArray[numSegments - 2].minx;
								segmentArray[numSegments - 1].miny =
										segmentArray[numSegments - 2].miny;
								segmentArray[numSegments - 1].x = sx;
								segmentArray[numSegments - 1].y = sy;
								segmentArray[numSegments - 1].size =
										segmentArray[numSegments - 2].size
												+ segmentArray[numSegments - 1].size;
								segmentArray[numSegments - 1].horizontal = sx
										- segmentArray[numSegments - 2].x;
								segmentArray[numSegments - 1].angle = atan2(
										sy - segmentArray[numSegments - 2].y,
										sx - segmentArray[numSegments - 2].x);
								segmentArray[numSegments - 2].x = sx;
								segmentArray[numSegments - 2].y = sy;
							}
						}
					}
				}
			}
		}
		ii++;
		if (ii >= len)
			ii = 0;
		cont = (ii != start);
	}
	for (int i = 0; i < numSegments; i++) {
		if (segmentArray[i].size > minSize
				&& (segmentArray[i].valid || debug)) {
			if (debug)
				fprintf(stdout,
						"Segment %i Type: %i Pos: %.2f %.2f Area: %i Vx: %i Vy: %i Mean: %i Thr: %i Eigen: %03f %03f %03f Roundness: %03f\n",
						i, segmentArray[i].type, segmentArray[i].x,
						segmentArray[i].y, segmentArray[i].size,
						segmentArray[i].maxx - segmentArray[i].minx,
						segmentArray[i].maxy - segmentArray[i].miny,
						segmentArray[i].mean, threshold, segmentArray[i].m0,
						segmentArray[i].m1,
						M_PI * 4 * segmentArray[i].m0 * segmentArray[i].m1,
						segmentArray[i].roundness);
			if (segmentArray[i].valid)
				result = segmentArray[i];
		}
	}
	if (numSegments == 2 && segmentArray[0].valid && segmentArray[1].valid)
		lastTrackOK = true;
	else
		lastTrackOK = false;
	//Drawing results 
	if (result.valid) {
		lastThreshold = threshold;
		drawAll = false;
		numFailed = 0;
	} else if (numFailed < maxFailed) {
		if (numFailed++ % 2 == 0)
			changeThreshold();
		else
			threshold = lastThreshold;
		if (debug)
			drawAll = true;
	} else {
		numFailed++;
		if (changeThreshold() == false)
			numFailed = 0;
		if (debug)
			drawAll = true;
	}
	int j = 0;
	for (int p = queueOldStart; p < queueEnd; p++) {
		pos = queue[p];
		image->data[3 * pos + 0] = 0;
		image->data[3 * pos + 1] = 0;
		image->data[3 * pos + 2] = 0;
	}

	if (draw) {
		for (int i = 0; i < len; i++) {
			j = buffer[i];
			if (j > 0) {
				if (drawAll || segmentArray[j - 1].valid) {
					image->data[i * 3 + j % 3] = 0;
					image->data[i * 3 + (j + 1) % 3] = 255;
					image->data[i * 3 + (j + 2) % 3] = 255;
				}
			}
		}
	}
	bufferCleanup(result);
	return result;
}
