#include "digamma.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "vb.h"

void VBLDA::fit(int** word_indexes_ptr, unsigned short** word_counts_ptr,
			int* n_word_each_doc, int* n_word_type_each_doc,unsigned long long n_all_word,
			int n_document, int n_word_type, int *sample_y, int n_sample_y) {
	this->n_document = n_document;
	this->n_word_type = n_word_type;
	theta = new double*[n_document];
	for (int d = 0; d < n_document; d++) {
		theta[d] = new double[n_topic];
		for (int k = 0; k < n_topic; k++) {
			theta[d][k] = (double)rand() / RAND_MAX;
		}
	}
	phi = new double*[n_topic];
	double **prob_w = new double*[n_topic];
	double **nkv = new double*[n_topic];
	for (int k = 0; k < n_topic; k++) {
		phi[k] = new double[n_word_type];
		prob_w[k] = new double[n_word_type];
		nkv[k] = new double[n_word_type];
		for (int w = 0; w < n_word_type; w++) {
			phi[k][w] = (double)rand() / RAND_MAX;
		}
	}

	for(int iter=0; iter < n_iter; iter++) {
		for(int i=0; i < k ; i++) {
			sum();
		}
	}
}
