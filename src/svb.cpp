#include "digamma.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

unsigned long xor128(){
    static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
    unsigned long t;
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
}

class SvbLda {
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

	int getSample(const char *path, int ***word_indexes_r, unsigned short ***word_counts_r, int **n_word_type_each_doc_r, int **n_word_each_doc_r, unsigned long long *n_all_word_r, int *n_document_r, int *n_word_type_r){
		int n_document;
		FILE *fp_word_indexes = fopen(path, "rb");
		if(!fp_word_indexes) return -1;
		fread(&n_document, sizeof(int), 1, fp_word_indexes);
		int **word_indexes = new int*[n_document];
		unsigned short **word_counts = new unsigned short*[n_document];
		int *n_word_type_each_doc = new int[n_document];
		int *n_word_each_doc = new int[n_document];
		unsigned long long n_all_word = 0;
		int n_word_type=0;
		int i, j;
		for (i=0; i < n_document; i++) {
			fread(&n_word_type_each_doc[i], sizeof(int), 1, fp_word_indexes);
			word_indexes[i] = new int[n_word_type_each_doc[i]];
			word_counts[i] = new unsigned short[n_word_type_each_doc[i]];
			for (j=0; j < n_word_type_each_doc[i]; j++) {
				fread(&word_indexes[i][j], sizeof(int), 1, fp_word_indexes);
				fread(&word_counts[i][j], sizeof(unsigned short), 1, fp_word_indexes);
				n_word_each_doc[i] += word_counts[i][j];
				n_all_word += word_counts[i][j];
				if (n_word_type < word_indexes[i][j])
					n_word_type = word_indexes[i][j];
			}
		}
		fclose(fp_word_indexes);

		n_word_type++;
		*word_indexes_r = word_indexes;
		*word_counts_r = word_counts;
		*n_word_type_each_doc_r = n_word_type_each_doc;
		*n_word_each_doc_r = n_word_each_doc;
		*n_all_word_r = n_all_word;
		*n_document_r = n_document;
		*n_word_type_r = n_word_type;
		return 0;
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

	void fitFile(const char *path) {
		int **word_indexes;
		unsigned short **word_counts;
		int *n_word_type_each_doc, *n_word_each_doc;
		int n_document, n_word_type;
		unsigned long long n_all_word;

		getSample(path, &word_indexes, &word_counts, &n_word_type_each_doc, &n_word_each_doc, &n_all_word, &n_document, &n_word_type);
		fit(word_indexes, word_counts, n_word_each_doc, n_word_type_each_doc, n_all_word, n_document, n_word_type);
		for(int i=0; i < n_document; i++) {
			delete[] word_indexes[i];
			delete[] word_counts[i];
		}
		delete[] word_indexes;
		delete[] word_counts;
		delete[] n_word_type_each_doc;
		delete[] n_word_each_doc;
	}
};

