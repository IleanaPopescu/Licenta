#include "pch.h"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

#include "opencv2/opencv.hpp"
#include <iostream>
#include <math.h>
#include "Kmeans.h"

using namespace cv;
using namespace std;

int find_point(double ** points, double nr, int ind, int nrPoints)
{
	for (int i = 0; i < nrPoints; i++) {
		if (points[i][ind] == nr)
			return i;
	}
	return 0;
}

int find_duplicate(int * vec, int ind, int nrPoints)
{
	for (int i = 0; i < nrPoints; i++) {
		if (vec[i] == ind)
			return 1;
	}
	return 0;
}

double calculate_distance(int dim, double *p1, double *p2)
{
	double distance = 0;

	for (int i = 0; i < dim; i++) {
		distance += (p2[i] - p1[i]) * (p2[i] - p1[i]);
	}

	distance = sqrt(distance);

	return distance;
}

int * Kmeans(int K, double ** points, int nrPoints, double ** centroids, int dim, int c, int *nrAnom)
{
	int isDif = 1;
	int * new_dist_min = (int *)malloc(sizeof(int) *nrPoints);
	int cluster_count[16];
	int * dist_min = (int *)malloc(sizeof(int) *nrPoints);

	int * vecr = (int *)malloc(sizeof(int)* K);

	if (centroids == NULL) {
		for (int i = 0; i < K; i++) {
			int r = rand() % nrPoints;

			for (int y = 0; y < K; y++) {
				if (vecr[y] == r) {
					r = rand() % nrPoints;
					y = 0;
				}	
			}


			for (int j = 0; j < dim; j++) {
				centroids[i][j] = points[r][j];

			}
		}

	}


	for (int i = 0; i < nrPoints; i++) {
		new_dist_min[i] = 0;
	}
	int contor = 0;
	while (isDif == 1) {
		
		/*for (int i = 0; i < nrPoints; i++) {
			printf("Punctul %d:: ", i);
			for (int j = 0; j < dim; j++) {
				printf(" %f ", points[i][j]);
			}
			printf("\n");
		}

		for (int i = 0; i < K; i++) {
			printf("Centroidul %d:: ", i);
			for (int j = 0; j < dim; j++) {
				printf(" %f ", centroids[i][j]);
			}
			printf("\n");
		}*/

		// calculeaza distanta de la fiecare punct la fiecare centroid
		double ** dist_init = (double **)malloc(sizeof(double *) * nrPoints);

		for (int i = 0; i < nrPoints; i++) {
			dist_init[i] = (double *)malloc(sizeof(double) * K);
		}

		for (int i = 0; i < nrPoints; i++) {
			for (int j = 0; j < K; j++) {
				dist_init[i][j] = calculate_distance(dim, points[i], centroids[j]);
			}
		}

		// calculeaza in ce cluster se afla fiecare punct
		for (int i = 0; i < nrPoints; i++) {
			int index_centroid_min = 0;

			for (int j = 0; j < K; j++) {
				if (dist_init[i][j] < dist_init[i][index_centroid_min]) {
					index_centroid_min = j;
				}
			}
			dist_min[i] = index_centroid_min;
		}

		/*for (int i = 0; i < nrPoints; i++) {
			printf("Punctul (");
			for (int j = 0; j < dim; j++) {
				printf(" %f ", points[i][j]);
			}
			printf(") se afla in clusterul %d \n", dist_min[i]);
		}*/

		//calculeaza noile coordonate ale centroizilor
		double ** new_centroides = (double **)malloc(sizeof(double *) * K);

		for (int i = 0; i < K; i++) {
			new_centroides[i] = (double *)malloc(sizeof(double) * dim);
		}

		for (int i = 0; i < K; i++) {
			for (int j = 0; j < dim; j++) {
				new_centroides[i][j] = 0;
			}
		}

		for (int i = 0; i < 16; i++) {
			cluster_count[i] = 0;
		}

		for (int i = 0; i < nrPoints; i++) {

			// in ce cluster este
			int active_cluster = dist_min[i];

			// contorizeaza numarul de clustere
			cluster_count[active_cluster]++;

			// aduna toate pct dintr-un cluser
			for (int j = 0; j < dim; j++) {
				new_centroides[active_cluster][j] += points[i][j];
			}
		}

		// calculeaza noile coordonate facand media aritmetica 
		for (int i = 0; i < K; i++) {

			if (cluster_count[i] == 0)
				printf("WARNING: Empty cluster %d! \n", i);

			// for each dimension
			for (int j = 0; j < dim; j++) {
				new_centroides[i][j] /= cluster_count[i];
			}
		}

		/*printf("Noile coordonate:: ");

		for (int i = 0; i < K; i++) {
			printf("Punctul (");
			for (int j = 0; j < dim; j++) {
				printf(" %f ", new_centroides[i][j]);
			}
			printf(")\n");
		}
		*/
		isDif = 0;

		for (int i = 0; i < nrPoints; i++) {
			if (new_dist_min[i] != dist_min[i]) {
				isDif = 1;
				break;
			}
		}

		if (isDif == 1) {
			centroids = new_centroides;
			new_dist_min = dist_min;
		}


		Mat img(500, 500, CV_8UC3);
		Mat img1(500, 500, CV_8UC3);
		Mat img2(500, 500, CV_8UC3);
		Mat img3(500, 500, CV_8UC3);
		Mat img4(500, 500, CV_8UC3);

		RNG rng(12345);

		double x = 0, y = 0;

		for (int i = 0; i < nrPoints; i++) {

			int active_cluster = dist_min[i];

			for (int j = 0; j < dim; j++) {
				if (j < dim / 2) {
					x += points[i][j];
				}
				else {
					y += points[i][j];
				}
			}
			x = x * 100;
			y = y * 100;
			if (active_cluster == 0) {
				//line(img, Point2d(x, y), Point2d(x, y), Scalar(0, 255, 0), 2, 8);
				line(img1, Point2d(points[i][0] * 100, points[i][1] * 100), Point2d(points[i][0] * 100, points[i][1] * 100), Scalar(0, 255, 0), 2, 8);
				line(img2, Point2d(points[i][1] * 100, points[i][2] * 100), Point2d(points[i][1] * 100, points[i][2] * 100), Scalar(0, 255, 0), 2, 8);
				line(img3, Point2d(points[i][0] * 100, points[i][4] * 100), Point2d(points[i][0] * 100, points[i][4] * 100), Scalar(0, 255, 0), 2, 8);
				//line(img4, Point2d(points[i][3] * 100, points[i][4] * 100), Point2d(points[i][3] * 100, points[i][4] * 100), Scalar(0, 255, 0), 2, 8);

			}
			else if (active_cluster == 1) {
				//line(img, Point2d(x, y), Point2d(x, y), Scalar(0, 255, 0), 2, 8);
				line(img1, Point2d(points[i][0] * 100, points[i][1] * 100), Point2d(points[i][0] * 100, points[i][1] * 100), Scalar(255, 0, 0), 2, 8);
				line(img2, Point2d(points[i][1] * 100, points[i][2] * 100), Point2d(points[i][1] * 100, points[i][2] * 100), Scalar(255, 0, 0), 2, 8);
				line(img3, Point2d(points[i][0] * 100, points[i][4] * 100), Point2d(points[i][0] * 100, points[i][4] * 100), Scalar(255, 0, 0), 2, 8);
				//line(img4, Point2d(points[i][3] * 100, points[i][4] * 100), Point2d(points[i][3] * 100, points[i][4] * 100), Scalar(255, 0, 0), 2, 8);
			}
			x = 0; y = 0;
		}
		for (int i = 0; i < K; i++) {

			for (int j = 0; j < dim; j++) {
				if (j < dim / 2) {
					x += centroids[i][j];
				}
				else {
					y += centroids[i][j];
				}
			}
			x = x * 50;
			y = y * 50;
			if (i == 0) {
				//line(img, Point2d(x, y), Point2d(x, y), Scalar(0, 200, 0), 2, 20);
				line(img1, Point2d(points[i][0] * 100, points[i][1] * 100), Point2d(points[i][0] * 100, points[i][1] * 100), Scalar(0, 100, 0), 2, 20);
				line(img2, Point2d(points[i][1] * 100, points[i][2] * 100), Point2d(points[i][1] * 100, points[i][2] * 100), Scalar(0, 100, 0), 2, 20);
				line(img3, Point2d(points[i][0] * 100, points[i][4] * 100), Point2d(points[i][0] * 100, points[i][4] * 100), Scalar(0, 100, 0), 2, 20);
				//line(img4, Point2d(points[i][3] * 100, points[i][4] * 100), Point2d(points[i][3] * 100, points[i][4] * 100), Scalar(0, 200, 0), 2, 20);

			}
			else if (i == 1) {
				//line(img, Point2d(x, y), Point2d(x, y), Scalar(200, 0, 0), 2, 20);
				line(img1, Point2d(points[i][0] * 100, points[i][1] * 100), Point2d(points[i][0] * 100, points[i][1] * 100), Scalar(100, 0, 0), 2, 20);
				line(img2, Point2d(points[i][1] * 100, points[i][2] * 100), Point2d(points[i][1] * 100, points[i][2] * 100), Scalar(100, 0, 0), 2, 20);
				line(img3, Point2d(points[i][0] * 100, points[i][4] * 100), Point2d(points[i][0] * 100, points[i][4] * 100), Scalar(100, 0, 0), 2, 20);
				//line(img4, Point2d(points[i][3] * 100, points[i][4]) * 100, Point2d(points[i][3] * 100, points[i][4] * 100), Scalar(200, 0, 0), 2, 20);
			}
			x = 0; y = 0;
		}
		char name[200];
	//	sprintf_s(name, 20, "adunate -- %d.jpg", contor);
		//imwrite(name, img );
		//imshow(name, img);

		sprintf_s(name, 200, "pageFault-workSetSizeR-%d(%d).jpg", c, contor);
		imwrite(name, img1 );
	//	imshow(name, img1);


		sprintf_s(name, 200, "pagePool-nonPagedPool-%d(%d).jpg", c, contor);
		imwrite(name, img2 );
		//imshow(name, img2);

		sprintf_s(name, 200, "pageFault-cpuUsage-%d(%d).jpg", c, contor);
		imwrite(name, img3 );
		//imshow(name, img3);

		contor++;

		//waitKey(0);
	}


	int * vec_anomalies, index, z = 0, a;
	double * vec;
	int * rez = (int *)malloc(sizeof(int) * nrPoints);
	//int * dupvec = (int *)malloc(sizeof(int) * nrPoints);


	for (int i = 0; i < K; i++) {

		for (int j = 0; j < dim - 3; j++) {

			vec = (double *)malloc(sizeof(double) * cluster_count[i]);
			vec_anomalies = (int *)calloc(cluster_count[i], sizeof(int));

			int q = 0;
			for (int k = 0; k < nrPoints; k++) {

				// in ce cluster este
				int active_cluster = dist_min[k];

				if (active_cluster == i) {
					vec[q] = points[k][j];
					q++;
				}
			}

			find_anomalies(vec, vec_anomalies, cluster_count[i]);
			//printf("vectorul dupa IQR: ");
			for (int l = 0; l < cluster_count[i]; l++) {
				if (vec_anomalies[l] == 1) {
				//	printf(" intru aici\n ");
					index = find_point(points, vec[l], j, nrPoints);
					a = find_duplicate(rez, index, nrPoints);
					if (a == 0) {
						rez[z] = index;
						z++;
					}
				}
			//	printf(" %d ", vec_anomalies[l]);
			}

			//printf("\n");

		}
	}
	*nrAnom = z;
	return rez;
}

