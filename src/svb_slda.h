#ifndef SVB_SLDA_H
#define SVB_SLDA_H

class SvbSlda {
private:
	int n_document;
	double lhood(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_type_each_doc, int n_document, int n_word_type);
	void FreeDoubleArray(void **array, int n_row);
public:
	int n_topic;
	int n_iter;
	int inner_iter;
	double alpha;
	double beta;
	int batch_size;
	int step_size;
	int n_word_type;
	double **phi;
	double **theta;

	SvbSlda(int n_topic=10, int n_iter=10, int inner_iter=10, double alpha=0.1,
			double beta=0.01, int batch_size=32, double step_size=0.05) :
				n_topic(n_topic), n_iter(n_iter), inner_iter(inner_iter),
				alpha(alpha), beta(beta), batch_size(batch_size), step_size(step_size){
	}

	~SvbSlda();
	void fit(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_each_doc, int* n_word_type_each_doc,unsigned long long n_all_word, int n_document, int n_word_type, int *sample_y, int n_sample_y);
};
#endif

