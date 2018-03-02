#include "digamma.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "svb_slda.h"

using namespace std;

unsigned long xor128(){
    static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
    unsigned long t;
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
}

void deltaRegW(double *reg_weights, int n_documents, int n_topics, int *n_word_each_docs, int* n_word_type_each_docs, double ***y_z, int *sample_y, double *delta) {
	double *temp_exp = new double[n_topics];
	double *temp1 = new double[n_topics];
	double *temp2 = new double[n_topics];

	for(int d=0; d < n_documents; d++) {
		for(int k=0; k < n_topics; k++) {
			temp_exp[k] = exp(reg_weights[k] / n_word_each_docs[d]);
		}
		double z_reg_w_tot = 1.;
		for(int w=0; w < n_word_type_each_docs[d]; w++) {
			double temp = 0.;
			for(int k=0; k < n_topics; k++) {
				temp += y_z[d][w][k] * temp_exp[k];
			}
			z_reg_w_tot *= temp;
		}
		for(int k=0; k < n_topics; k++) {
			double tot = 0.;
			for(int i=0;  i < n_word_type_each_docs[d]; i++) {
				tot += y_z[d][i][k];
			}
			temp1[k] = (double)sample_y[d] * tot / n_word_each_docs[d];
		}

		for(int w=0;  w < n_word_type_each_docs[d]; w++) {
			double temp3 = 0.;
			for(int k=0; k < n_topics; k++) {
				temp3 += y_z[d][w][k] * temp_exp[k];
			}
			double z_reg_w = z_reg_w_tot / temp3;
			for(int k=0; k < n_topics; k++) {
				double temp = y_z[d][w][k] / n_word_each_docs[d];
				temp2[k] += temp * temp_exp[k] * z_reg_w;
			}
		}
		for(int k=0; k < n_topics; k++)
			delta[k] = temp1[k] - temp2[k];
	}
	delete [] temp_exp;
	delete [] temp1;
	delete [] temp2;
}


void SvbSlda::FreeDoubleArray(void **array, int n_row) {
	for (int i = 0; i < n_row; i++)
			delete[] array[i];
	delete[] array;
}

SvbSlda::~SvbSlda() {
	FreeDoubleArray((void**)theta, n_document);
	FreeDoubleArray((void**)phi, n_topic);
}

double SvbSlda::lhood(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_type_each_doc, int n_document, int n_word_type) {
	double **theta_hat = new double*[n_document];
	for (int d = 0; d < n_document; d++) {
		theta_hat[d] = new double[n_topic];
		double r_sum = 0.;
		for (int k = 0; k < n_topic; k++) {
			r_sum += theta[d][k];
		}
		for (int k = 0; k < n_topic; k++) {
			theta_hat[d][k] = theta[d][k] / r_sum;
		}
	}
	double **phi_hat = new double*[n_topic];
	for (int k = 0; k < n_topic; k++) {
		phi_hat[k] = new double[n_word_type];
		double r_sum = 0.;
		for (int w = 0; w < n_word_type; w++) {
			r_sum += phi[k][w];
		}
		for (int w = 0; w < n_word_type; w++) {
			phi_hat[k][w] = phi[k][w] / r_sum;
		}
	}

	double ll = 0.;
	for(int d=0; d < n_document; d++) {
		for(int w=0; w < n_word_type_each_doc[d] ; w++) {
			int word_no = word_indexes_ptr[d][w];
			double est_prob = 0.;
			for(int k=0; k < n_topic; k++) {
				est_prob += theta_hat[d][k] * phi_hat[k][word_no];
			}
			ll += log(est_prob) * word_counts_ptr[d][w];
		}
	}
	FreeDoubleArray((void**)theta_hat, n_document);
	FreeDoubleArray((void**)phi_hat, n_topic);
	return ll;
}

void SvbSlda::fit(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_each_doc, int* n_word_type_each_doc,unsigned long long n_all_word, int n_document, int n_word_type, int *sample_y, int n_sample_y){
	this->n_document = n_document;
	this->n_word_type = n_word_type;
	int *document_idxs = new int[batch_size];
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

	double *reg_weights = new double[n_topic];
	for (int k = 0; k < n_topic; k++) {
		reg_weights[k] = (double)rand() / RAND_MAX;
	}

	double ***y_z = new double**[n_sample_y];
	for (int d = 0; d < n_sample_y; d++) {
		y_z[d] = new double*[n_word_type_each_doc[d]];
		for (int w = 0; w < n_word_type_each_doc[d]; w++) {
			y_z[d][w] = new double[n_topic];
		}
	}
	double *latent_z = new double[n_topic];
	double *ndk = new double[n_topic];
	double *prob_d = new double[n_topic];
	double document_rate = (double)n_document / batch_size;
	double *delta = new double[n_topic];
	double *slack = new double[n_sample_y];
	double *temp5 = new double[n_topic];

	for(int iter=0; iter < n_iter; iter++) {
		if (iter % 100 == 0) {
			cout << "============ " << iter << " / " << n_iter << " ============" << endl;
			double ll = lhood(word_indexes_ptr, word_counts_ptr, n_word_type_each_doc, n_document, n_word_type);
			cout << "lhood:" << ll << endl;
		}
		for(int k=0; k < n_topic; k++) {
			memset(nkv[k], 0, sizeof(double) * n_word_type);
		}
		double weights_process = false;
		for (int i=0; i < batch_size; i++) {
			document_idxs[i] = xor128() % n_document;
			if (document_idxs[i] < n_sample_y) {
				weights_process = true;
			}
		}
		for(int k=0; k < n_topic; k++) {
			double sum_phi = 0.;
			for(int i=0; i < n_word_type; i++) {
				sum_phi += phi[k][i];
			}
			sum_phi = digammal(sum_phi);
			for(int i=0; i < n_word_type; i++) {
				prob_w[k][i] = digammal(phi[k][i]) - sum_phi;
			}
		}

		for (int d_idx = 0; d_idx < batch_size; d_idx++) {
			int d = document_idxs[d_idx];
			for (int k=0; k < n_topic; k++)
				theta[d][k] = (double)n_word_type_each_doc[d] / n_topic + alpha;

			for (int iter2=0 ; iter2 < inner_iter; iter2++) {
				memset(ndk, 0, sizeof(double) * n_topic);
				double sum_theta_d = 0.0;
				for(int k=0; k < n_topic; k++)
					sum_theta_d += theta[d][k];
				sum_theta_d = digammal(sum_theta_d);
				for(int k=0; k < n_topic; k++)
					prob_d[k] = digammal(theta[d][k]) - sum_theta_d;

				double z_reg_w_tot = 1.;
				if (d < n_sample_y) {

					for(int w=0; w < n_word_type_each_doc[d]; w++) {
						double temp = 0.;
						for(int k=0; k < n_topic; k++) {
							temp += y_z[d][w][k] * reg_weights[k] / n_word_type_each_doc[d];
						}
						z_reg_w_tot *= temp;
					}
				}

				for(int w=0; w < n_word_type_each_doc[d]; w++) {
					int word_no = word_indexes_ptr[d][w];

					if (d < n_sample_y) {
						double temp = 0.;
						for(int k=0; k < n_topic; k++) {
							temp += y_z[d][w][k] * exp(reg_weights[k] / n_word_type_each_doc[d]);
						}
						double z_reg_w = z_reg_w_tot / temp;
						for(int k=0; k < n_topic; k++) {
							double temp3 = sample_y[d] * reg_weights[k] / n_word_type_each_doc[d];
							double temp4 = (1. / slack[d]) * exp(reg_weights[k] / n_word_type_each_doc[d]) * z_reg_w;
							temp5[k] = temp3 - temp4;
						}
					}else {
						memset(temp5, 0, sizeof(double) * n_topic);
					}

					double sum_z = 0.;
					for(int k=0; k < n_topic; k++) {
						latent_z[k] = exp(prob_w[k][word_no] + prob_d[k] + temp5[k]);
						sum_z += latent_z[k];
					}
					for(int k=0; k < n_topic; k++) {
						latent_z[k] /= sum_z;
						ndk[k] += latent_z[k] * word_counts_ptr[d][w];
					}
					if (d < n_sample_y) {
						memcpy(y_z[d][w], latent_z, sizeof(double) * n_topic);
						double temp2 = 1.;
						for(int w=0; w < n_word_type_each_doc[d]; w++) {
							double tmp = 0.;
							for(int k=0; k < n_topic; k++) {
								tmp += y_z[d][w][k] * (1 + exp(reg_weights[k] / n_word_type_each_doc[d]));
							}
							temp2 *= tmp;
						}
						slack[d] = temp2;
					}
				}
				for(int k=0; k < n_topic; k++) {
					theta[d][k] = ndk[k] + alpha;
				}
			}

			memset(ndk, 0, sizeof(double) * n_topic);
			double sum_theta_d = 0.0;
			for(int k=0; k < n_topic; k++)
				sum_theta_d += theta[d][k];
			sum_theta_d = digammal(sum_theta_d);
			for(int k=0; k < n_topic; k++)
				prob_d[k] = digammal(theta[d][k]) - sum_theta_d;

			for(int w=0; w < n_word_type_each_doc[d]; w++) {
				int word_no = word_indexes_ptr[d][w];
				double sum_z = 0.;
				for(int k=0; k < n_topic; k++) {
					latent_z[k] = exp(prob_w[k][word_no] + prob_d[k]);
					sum_z += latent_z[k];
				}
				for(int k=0; k < n_topic; k++) {
					latent_z[k] /= sum_z;
					ndk[k] += latent_z[k] * word_counts_ptr[d][w];
					nkv[k][word_no] += latent_z[k] * word_counts_ptr[d][w];
				}
			}
			for(int k=0; k < n_topic; k++) {
				theta[d][k] = ndk[k] + alpha;
			}

			for(int k=0; k < n_topic; k++) {
				for(int w=0; w < n_word_type; w++) {
					double diff = document_rate * nkv[k][w] + beta - phi[k][w];
					phi[k][w] += step_size * diff;
				}
			}
		}

		for(int i=0; i < 100 ; i++) {
			deltaRegW(reg_weights, n_sample_y, n_topic, n_word_each_doc, n_word_type_each_doc, y_z, sample_y, delta);
			for(int k=0; k < n_topic; k++) {
				reg_weights[k] += step_size * delta[k];
			}
		}
	}


	for (int d = 0; d < n_document; d++) {
		double theta_sum = 0.;
		for(int k=0; k < n_topic; k++) {
			theta_sum += theta[d][k];
		}
		for(int k=0; k < n_topic; k++) {
			theta[d][k] /= theta_sum;
		}
	}
	for (int k = 0; k < n_topic; k++) {
		double phi_sum = 0.;
		for(int w=0; w < n_word_type; w++) {
			phi_sum += phi[k][w];
		}
		for(int w=0; w < n_word_type; w++) {
			phi[k][w] /= phi_sum;
		}
	}


	delete[] document_idxs;

	for (int d = 0; d < n_topic; d++) {
		delete[] nkv[d];
		delete[] prob_w[d];
	}
	for (int d = 0; d < n_sample_y; d++) {
		for (int w = 0; w < n_word_type_each_doc[d]; w++) {
			delete[] y_z[d][w];
		}
		delete[] y_z[d];
	}
	delete[] y_z;
	delete[] prob_w;
	delete[] nkv;
	delete[] latent_z;
	delete[] ndk;
	delete[] prob_d;
	delete[] delta;
	delete[] temp5;
	delete[] slack;
}


