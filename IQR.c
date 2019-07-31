#include "pch.h"
#include <iostream>
#include <stdio.h>      
#include <stdlib.h>     /* qsort */
int compare(const void * a, const void * b)
{
	if (*(double*)a > *(double*)b) {
		return 1;
	}
	else if (*(double*)a < *(double*)b) {
		return -1;
	}

	return 0;
}

// function to calculate the median of the array
double Find_median(double array[], int n)
{
	double median = 0;

	// if number of elements are even
	if (n % 2 == 0)
		median = (array[(n - 1) / 2] + array[n / 2]) / 2.0;
	// if number of elements are odd
	else
		median = array[n / 2];

	return median;
}

double IQR(double * vec, int nrPoints)
{
	int mij = nrPoints / 2;

	double * leftSide = (double *)malloc(sizeof(double) * mij);
	double * rightSide = (double *)malloc(sizeof(double) * mij);

	if (nrPoints % 2 == 0) {
		for (int i = 0; i < mij; i++) {
			leftSide[i] = vec[i];
		}
		int k = 0;
		for (int j = mij; j < nrPoints; j++) {
			rightSide[k] = vec[j];
			k++;
		}
	}
	else if (nrPoints % 2 != 0) {
		for (int i = 0; i < mij; i++) {
			leftSide[i] = vec[i];
		}
		int k = 0;
		for (int j = mij + 1; j < nrPoints; j++) {
			rightSide[k] = vec[j];
			k++;
		}
	}

	/*printf("leftSide::");
	for (int i = 0; i < mij; i++) {
		printf(" %f ", leftSide[i]);
	}
	printf("\n");

	printf("rightSide::");
	for (int i = 0; i < mij; i++) {
		printf(" %f ", rightSide[i]);
	}
	printf("\n");
	*/
	// Median of first half 
	double Q1 = Find_median(leftSide, mij);
	//printf("Q1:: %f\n", Q1);

	// Median of first half 
	double Q3 = Find_median(rightSide, mij);
	//printf("Q3:: %f\n", Q3);

	return (Q3 - Q1);
}


void find_anomalies(double * vec, int * anomalies, int nrPoints)
{
	double med_arti = 0;

	for (int i = 0; i < nrPoints; i++) {
		med_arti += vec[i];
	}
	med_arti /= nrPoints;

	//printf("media aritmetica:: %f\n", med_arti);

	qsort(vec, nrPoints, sizeof(double), compare);

	/*printf("vectorul cu pct inainte de IQR:: ");

	for (int i = 0; i < nrPoints; i++) {
		printf(" %f ",vec[i]);
	}
	printf("\n");*/

	double iqr = IQR(vec, nrPoints);

	//printf("IQR:: %f\n", iqr);
	int count = nrPoints;

	while (count) {
		count--;
		if (vec[count] - med_arti > iqr * 10.5) {
			anomalies[count] = 1;
		}
		else {
			break;
		}
	}
}
