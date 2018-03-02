#include "lda.h"

class VBLDA : LDA {
	int n_topic;
	int n_iter;
	double alpha;
	double beta;
	int n_document;
	int n_word_type;

public:
	VBLDA(int n_topic=10, int n_iter=100, alpha=0.1, beta=0.01) :
		n_topic(n_topic), n_iter(n_iter), alpha(alpha), beta(beta){}

	void fit(int** word_indexes_ptr, unsigned short** word_counts_ptr, int* n_word_each_doc,
			int* n_word_type_each_doc,unsigned long long n_all_word, int n_document,
			int n_word_type, int *sample_y, int n_sample_y);
};
