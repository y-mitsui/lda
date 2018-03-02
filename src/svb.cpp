#include "digamma.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

class SvbLda : LDA {
public:
	int n_topic;
	int n_iter;
	int inner_iter;
	double alpha;
	double beta;
	int batch_size;
	int step_size;

	SvbLda(n_topic=10, n_iter=100, inner_iter=10, alpha=0.1,
			beta=0.01, batch_size=16, step_size=0.1) :
				n_topic(n_topic), n_iter(n_iter), inner_iter(inner_iter),
				alpha(alpha), beta(beta), batch_size(batch_size), step_size(step_size){
	}

	void fit(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_each_doc, int* n_word_type_each_doc,unsigned long long n_all_word, int n_document, int n_word_type){
		int *document_idxs = new int[batch_size];
		double **theta = new double*[n_document];
		for (int d = 0; d < n_document; d++)
			theta[d] = new double[n_topic];
		double **phi = new double*[n_topic];
		double **prob_w = new double*[n_topic];
		double **nkv = new double*[n_topic];
		for (int k = 0; k < n_document; k++) {
			phi[k] = new double[n_word_type];
			prob_w[k] = new double[n_word_type];
			nkv[k] = new double[n_word_type];
		}
		double *latent_z = new double[n_topic];
		double *ndk = new double[n_topic];
		double *prob_d = new double[n_topic];
		double document_rate = (double)n_document / batch_size;
		for(int iter; iter < n_iter; iter++) {
			for(int k=0; k < n_topic; k++) {
				memset(nkv[k], 0, sizeof(double) * n_word_type);
			}
			for (int i=0; i < batch_size; i++) {
				document_idxs[i] = xor128() % n_document;
			}
			for(int k=0; k < n_topic; k++) {
				double sum_phi = 0.;
				for(int i=0; i < n_word_type; i++) {
					sum_phi += phi[i];
				}
				sum_phi = digammal(sum_phi);
				for(int i=0; i < n_word_type; i++) {
					prob_w[k][i] = digammal(phi[k][i]) - sum_phi;
				}
			}

			for (int d = 0; d < batch_size; d++) {
				for(int k=0; k < n_topic; k++)
					theta[d][k] = (double)n_word_type_each_doc[d] / n_topic + alpha;

				for (int iter2=0 ; iter2 < inner_iter; iter2++) {
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
							latent_z[k] = exp(prob_w[k][word_no] + prob_d);
							sum_z += latent_z[k];
						}
						for(int k=0; k < n_topic; k++) {
							latent_z[k] /= sum_z;
							ndk[k] += latent_z[k] * word_counts_ptr[d][w];
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
						latent_z[k] = exp(prob_w[k][word_no] + prob_d);
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
		}

		delete[] document_idxs;
		for (int d = 0; d < n_document; d++)
			delete[] theta[d];
		delete[] theta;
		for (int d = 0; d < n_topic; d++) {
			delete[] phi[d];
			delete[] nkv[d];
			delete[] prob_w[d];
		}
		delete[] prob_w;
		delete[] nkv;
		delete[] latent_z;
		delete[] phi;
		delete[] prob_w
		delete[] ndk;
		delete[] prob_d;
	}


};

